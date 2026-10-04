#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config.h"
#include "display.h"
#include "mqtt.h"
#include "telegram.h"
#include "weather.h"

#define RELAY_ON LOW // รีเลย์เป็นแบบ Active LOW
#define RELAY_OFF HIGH
#define SW_PRESSED LOW // สวิตช์เป็นแบบ Active LOW + external pull-up

const uint8_t RELAY_PINS[3] = {17, 18, 4}; // relay1, relay2, relay3
const uint8_t SW_PINS[3] = {34, 35, 32};   // SW1, SW2, SW3
const uint8_t CHANNEL_COUNT = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
const unsigned long DEBOUNCE_MS = 30; // เวลาที่สัญญาณต้องนิ่งก่อนยอมรับ
const unsigned long WIFI_RESET_HOLD_MS = 5000; // กด SW1 ค้างเท่านี้ = ล้างการตั้งค่า WiFi

WiFiManager wm;
bool wifiWasConnected = false;
uint32_t reportedWeatherVersion = 0; // version ของข้อมูลอากาศที่ส่งสรุปไปล่าสุด
unsigned long lastReportMs = 0;
bool reportedOnce = false;
bool aqiAlerting = false;  // AQI อยู่ในช่วงเกินเกณฑ์ (แจ้งไปแล้ว)
bool pm25Alerting = false; // PM2.5 อยู่ในช่วงเกินเกณฑ์ (แจ้งไปแล้ว)

bool relayOn[CHANNEL_COUNT] = {false, false, false}; // สถานะรีเลย์แต่ละตัว
bool telemetryDirty = true;                          // มีข้อมูลใหม่ที่ยังไม่ได้ส่ง MQTT
uint32_t publishedWeatherVersion = 0;

// ข้อมูล debounce ของสวิตช์แต่ละตัว
bool lastReading[CHANNEL_COUNT];       // ค่าที่อ่านได้ล่าสุด (ยังไม่ผ่าน debounce)
bool stableState[CHANNEL_COUNT];       // ค่าที่นิ่งแล้ว
unsigned long lastChangeMs[CHANNEL_COUNT]; // เวลาที่ค่าที่อ่านได้เปลี่ยนล่าสุด
unsigned long sw1PressedMs = 0;            // เวลาที่ SW1 ถูกกด (ที่ผ่าน debounce แล้ว)

void setRelay(uint8_t index, bool on)
{
    relayOn[index] = on;
    digitalWrite(RELAY_PINS[index], on ? RELAY_ON : RELAY_OFF);
    telemetryDirty = true;
    telegramNotify(String("Relay ") + (index + 1) + ": " + (on ? "ON" : "OFF"));
}

// คำสั่งจาก MQTT topic control
void onMqttControl(uint8_t index, int8_t state)
{
    if (index >= CHANNEL_COUNT)
        return;
    bool on = state < 0 ? !relayOn[index] : state > 0;
    if (on != relayOn[index])
        setRelay(index, on);
}

// ส่งสรุปสภาพอากาศเป็นรอบ และแจ้งเตือนเมื่อ AQI / PM2.5 เกินเกณฑ์ใน config.h (แจ้งตอนข้ามเกณฑ์ และตอนกลับเป็นปกติ)
void weatherReport(unsigned long now)
{
    if (!weather.valid || weather.version == reportedWeatherVersion)
        return;
    reportedWeatherVersion = weather.version;

    String summary = String("Nonthaburi: ") + weather.description + "\nTemp " + String(weather.temp, 1) +
                     " C (feels " + String(weather.feelsLike, 1) + "), RH " + weather.humidity + "%\nAQI " +
                     weather.aqi + ", PM2.5 " + String(weather.pm25, 1) + ", PM10 " + String(weather.pm10, 1) +
                     " ug/m3";
    bool alerted = false;

    if (ALERT_AQI_LEVEL > 0)
    {
        bool over = weather.aqi >= ALERT_AQI_LEVEL;
        if (over != aqiAlerting)
        {
            aqiAlerting = over;
            telegramNotify(String(over ? "ALERT: AQI " : "OK: AQI back to ") + weather.aqi +
                           " (threshold " + ALERT_AQI_LEVEL + ")\n" + summary);
            alerted = true;
        }
    }

    if (ALERT_PM25_UGM3 > 0)
    {
        bool over = pm25Alerting ? weather.pm25 > ALERT_PM25_UGM3 - ALERT_PM25_HYSTERESIS
                                 : weather.pm25 >= ALERT_PM25_UGM3;
        if (over != pm25Alerting)
        {
            pm25Alerting = over;
            if (!alerted) // ถ้า AQI เพิ่งแจ้งพร้อมสรุปไปแล้ว ไม่ส่งซ้ำ
                telegramNotify(String(over ? "ALERT: PM2.5 " : "OK: PM2.5 back to ") + String(weather.pm25, 1) +
                               " ug/m3 (threshold " + String(ALERT_PM25_UGM3, 1) + ")\n" + summary);
            alerted = true;
        }
    }

    bool periodicDue = REPORT_INTERVAL_MS > 0 && (!reportedOnce || (now - lastReportMs) >= REPORT_INTERVAL_MS);
    if (periodicDue && !alerted)
        telegramNotify(summary);
    if (periodicDue || alerted)
    {
        lastReportMs = now;
        reportedOnce = true;
    }
}

void setup()
{
    Serial.begin(115200);
    displayInit();

    for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
    {
        digitalWrite(RELAY_PINS[i], RELAY_OFF); // ตั้งค่าก่อน pinMode เพื่อไม่ให้รีเลย์ดีดตอนเริ่ม
        pinMode(RELAY_PINS[i], OUTPUT);

        pinMode(SW_PINS[i], INPUT); // มี external pull-up แล้ว (GPIO34/35 ไม่มี pull-up ภายใน)
        bool reading = digitalRead(SW_PINS[i]);
        lastReading[i] = reading;
        stableState[i] = reading;
        lastChangeMs[i] = millis();
    }

    // WiFi: ถ้ายังไม่เคยตั้งค่า จะเปิด AP "ESP32-Relay" ให้เชื่อมต่อแล้วตั้งค่า WiFi ผ่านเว็บ (192.168.4.1)
    // ใช้โหมด non-blocking เพื่อให้สวิตช์/รีเลย์ทำงานได้ระหว่างรอ WiFi
    WiFi.mode(WIFI_STA);
    configTzTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2); // sync เวลาอัตโนมัติเมื่อมีเน็ต และซ้ำเป็นระยะ
    mqttInit(onMqttControl);
    wm.setConfigPortalBlocking(false);
    if (wm.autoConnect("ESP32-Relay"))
    {
        Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        Serial.println("WiFi config portal started: AP ESP32-Relay");
    }
}

void loop()
{
    unsigned long now = millis();

    wm.process(); // จัดการ config portal / การเชื่อมต่อ WiFi

    bool wifiConnected = WiFi.status() == WL_CONNECTED;
    if (wifiConnected != wifiWasConnected)
    {
        wifiWasConnected = wifiConnected;
        if (wifiConnected)
        {
            Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
            telegramNotify(String("ESP32 online, IP ") + WiFi.localIP().toString());
        }
        else
            Serial.println("WiFi disconnected");
    }

    weatherUpdate(); // ดึงสภาพอากาศ/AQI ทุก 2 นาที
    weatherReport(now);
    telegramUpdate(); // ส่งข้อความที่ค้างในคิว

    mqttUpdate();
    if (mqttJustConnected() || weather.version != publishedWeatherVersion)
    {
        publishedWeatherVersion = weather.version;
        telemetryDirty = true;
    }
    if (telemetryDirty && mqttConnected())
    {
        mqttPublishTelemetry(relayOn, CHANNEL_COUNT);
        telemetryDirty = false;
    }
    displayUpdate(wifiConnected);

    for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
    {
        bool reading = digitalRead(SW_PINS[i]);

        // ค่าเปลี่ยน ให้เริ่มนับเวลา debounce ใหม่
        if (reading != lastReading[i])
        {
            lastReading[i] = reading;
            lastChangeMs[i] = now;
        }

        // ค่านิ่งเกินเวลา debounce และต่างจากค่าที่ยอมรับไว้ = เกิดการเปลี่ยนสถานะจริง
        if ((now - lastChangeMs[i]) >= DEBOUNCE_MS && reading != stableState[i])
        {
            stableState[i] = reading;

            // toggle เฉพาะตอนกดลง (ขอบขาลง) ไม่ toggle ตอนปล่อย
            if (stableState[i] == SW_PRESSED)
            {
                setRelay(i, !relayOn[i]);
                if (i == 0)
                    sw1PressedMs = now;
            }
        }
    }

    // กด SW1 ค้างครบเวลา: ล้างค่า WiFi ที่บันทึกไว้ แล้วรีสตาร์ทเพื่อเปิด config portal
    if (stableState[0] == SW_PRESSED && (now - sw1PressedMs) >= WIFI_RESET_HOLD_MS)
    {
        Serial.println("SW1 held 5s: reset WiFi settings, restarting");
        displayMessage("Reset WiFi...");
        wm.resetSettings();
        delay(500);
        ESP.restart();
    }
}
