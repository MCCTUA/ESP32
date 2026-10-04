#include "weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

// จังหวัดนนทบุรี
const char *WEATHER_LAT = "13.8621";
const char *WEATHER_LON = "100.5144";
const unsigned long WEATHER_INTERVAL_MS = 2UL * 60UL * 1000UL; // อ่านทุก 2 นาที
const uint16_t HTTP_TIMEOUT_MS = 5000;

WeatherData weather;

static unsigned long lastFetchMs = 0;
static bool fetchedOnce = false;

// GET แล้ว parse JSON ลง doc; คืน true ถ้าสำเร็จ
static bool fetchJson(const String &url, JsonDocument &doc)
{
    WiFiClientSecure client;
    client.setInsecure(); // ไม่ตรวจ certificate (ข้อมูลสภาพอากาศไม่ sensitive)

    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);
    if (!http.begin(client, url))
        return false;

    int code = http.GET();
    if (code != HTTP_CODE_OK)
    {
        Serial.printf("OpenWeather HTTP error: %d\n", code);
        http.end();
        return false;
    }

    DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();
    if (err)
    {
        Serial.printf("JSON error: %s\n", err.c_str());
        return false;
    }
    return true;
}

static bool fetchWeather()
{
    String query = String("?lat=") + WEATHER_LAT + "&lon=" + WEATHER_LON +
                   "&units=metric&lang=en&appid=" + OPENWEATHER_API_KEY;

    JsonDocument weatherDoc;
    if (!fetchJson("https://api.openweathermap.org/data/2.5/weather" + query, weatherDoc))
        return false;

    JsonDocument airDoc;
    if (!fetchJson("https://api.openweathermap.org/data/2.5/air_pollution" + query, airDoc))
        return false;

    weather.description = weatherDoc["weather"][0]["description"] | "";
    weather.temp = weatherDoc["main"]["temp"] | 0.0f;
    weather.feelsLike = weatherDoc["main"]["feels_like"] | 0.0f;
    weather.humidity = weatherDoc["main"]["humidity"] | 0;

    JsonObject air = airDoc["list"][0];
    weather.aqi = air["main"]["aqi"] | 0;
    weather.pm25 = air["components"]["pm2_5"] | 0.0f;
    weather.pm10 = air["components"]["pm10"] | 0.0f;
    weather.valid = true;
    weather.version++;
    return true;
}

void weatherUpdate()
{
    if (WiFi.status() != WL_CONNECTED)
        return;

    unsigned long now = millis();
    if (fetchedOnce && (now - lastFetchMs) < WEATHER_INTERVAL_MS)
        return;

    // บันทึกเวลาก่อน เพื่อให้ลองใหม่ตามรอบเดิมถ้าล้มเหลว (ไม่ยิงถี่เกินไป)
    fetchedOnce = true;
    lastFetchMs = now;

    if (fetchWeather())
    {
        Serial.printf("Nonthaburi: %s, %.1f C (feels %.1f), RH %d%%\n",
                      weather.description.c_str(), weather.temp, weather.feelsLike, weather.humidity);
        Serial.printf("AQI %d, PM2.5 %.1f, PM10 %.1f ug/m3\n", weather.aqi, weather.pm25, weather.pm10);
    }
    else
    {
        Serial.println("Weather fetch failed");
    }
}
