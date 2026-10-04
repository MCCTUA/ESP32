#pragma once

#include <Arduino.h>

void displayInit();

// เรียกใน loop() — วาดจอใหม่เฉพาะเมื่อข้อมูลหรือสถานะ WiFi เปลี่ยน
void displayUpdate(bool wifiConnected);

// แสดงข้อความสั้น ๆ เต็มจอ (ใช้ก่อนรีสตาร์ท ฯลฯ)
void displayMessage(const char *text);
