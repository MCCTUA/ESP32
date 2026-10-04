#pragma once

#include <Arduino.h>

void displayInit();

// เรียกใน loop() — วาดจอใหม่เฉพาะเมื่อข้อมูลหรือสถานะ WiFi เปลี่ยน
void displayUpdate(bool wifiConnected);
