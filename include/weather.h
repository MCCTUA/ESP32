#pragma once

#include <Arduino.h>

struct WeatherData
{
    bool valid = false;
    uint32_t version = 0; // เพิ่มทุกครั้งที่ดึงข้อมูลสำเร็จ (ใช้ตรวจว่าต้องวาดจอใหม่)
    String description; // คำอธิบายสภาพอากาศ (ภาษาอังกฤษ เพราะฟอนต์ OLED ไม่รองรับไทย)
    float temp;         // °C
    float feelsLike;    // °C
    int humidity;       // %
    int aqi;            // 1 (ดีมาก) - 5 (แย่มาก)
    float pm25;         // µg/m3
    float pm10;         // µg/m3
};

extern WeatherData weather;

// เรียกใน loop() — ดึงข้อมูลจาก OpenWeather ทุก WEATHER_INTERVAL_MS เมื่อ WiFi เชื่อมต่ออยู่
void weatherUpdate();
