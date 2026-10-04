#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#include "weather.h"

#define OLED_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

static Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool oledReady = false;

// ค่าที่วาดล่าสุด ใช้ตัดสินใจว่าต้องวาดใหม่หรือไม่
static uint32_t drawnVersion = UINT32_MAX;
static int8_t drawnWifi = -1;

static const char *aqiText(int aqi)
{
    switch (aqi)
    {
    case 1: return "Good";
    case 2: return "Fair";
    case 3: return "Moderate";
    case 4: return "Poor";
    case 5: return "Very Poor";
    default: return "-";
    }
}

void displayInit()
{
    Wire.begin(21, 22); // SDA, SCL
    oledReady = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (!oledReady)
    {
        Serial.println("OLED init failed");
        return;
    }
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextSize(1);
    oled.setCursor(0, 0);
    oled.println("Starting...");
    oled.display();
}

void displayUpdate(bool wifiConnected)
{
    if (!oledReady)
        return;
    if (drawnVersion == weather.version && drawnWifi == (int8_t)wifiConnected)
        return;
    drawnVersion = weather.version;
    drawnWifi = wifiConnected;

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setCursor(0, 0);

    if (!weather.valid)
    {
        oled.println("Nonthaburi");
        oled.println();
        oled.println(wifiConnected ? "Loading weather..." : "WiFi: connect to AP");
        if (!wifiConnected)
            oled.println("ESP32-Relay");
        oled.display();
        return;
    }

    oled.print("Nonthaburi");
    oled.setCursor(OLED_WIDTH - 6 * 7, 0);
    oled.print(wifiConnected ? "WiFi" : "no net"); // สถานะ WiFi มุมขวาบน
    oled.setCursor(0, 12);
    oled.println(weather.description);

    oled.setTextSize(2);
    oled.setCursor(0, 24);
    oled.printf("%.1fC", weather.temp);
    oled.setTextSize(1);
    oled.setCursor(80, 24);
    oled.printf("RH %d%%", weather.humidity);
    oled.setCursor(80, 34);
    oled.printf("FL %.0fC", weather.feelsLike);

    oled.setCursor(0, 46);
    oled.printf("AQI %d %s", weather.aqi, aqiText(weather.aqi));
    oled.setCursor(0, 56);
    oled.printf("PM2.5 %.0f PM10 %.0f", weather.pm25, weather.pm10);
    oled.display();
}
