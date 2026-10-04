#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "display.h"
#include "weather.h"

#define RELAY_ON LOW // รีเลย์เป็นแบบ Active LOW
#define RELAY_OFF HIGH
#define SW_PRESSED LOW // สวิตช์เป็นแบบ Active LOW + external pull-up

const uint8_t RELAY_PINS[3] = {17, 18, 4}; // relay1, relay2, relay3
const uint8_t SW_PINS[3] = {34, 35, 32};   // SW1, SW2, SW3
const uint8_t CHANNEL_COUNT = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
const unsigned long DEBOUNCE_MS = 30; // เวลาที่สัญญาณต้องนิ่งก่อนยอมรับ

WiFiManager wm;
bool wifiWasConnected = false;

bool relayOn[CHANNEL_COUNT] = {false, false, false}; // สถานะรีเลย์แต่ละตัว

// ข้อมูล debounce ของสวิตช์แต่ละตัว
bool lastReading[CHANNEL_COUNT];       // ค่าที่อ่านได้ล่าสุด (ยังไม่ผ่าน debounce)
bool stableState[CHANNEL_COUNT];       // ค่าที่นิ่งแล้ว
unsigned long lastChangeMs[CHANNEL_COUNT]; // เวลาที่ค่าที่อ่านได้เปลี่ยนล่าสุด

void setRelay(uint8_t index, bool on)
{
    relayOn[index] = on;
    digitalWrite(RELAY_PINS[index], on ? RELAY_ON : RELAY_OFF);
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
            Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());
        else
            Serial.println("WiFi disconnected");
    }

    weatherUpdate(); // ดึงสภาพอากาศ/AQI ทุก 2 นาที
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
            }
        }
    }
}
