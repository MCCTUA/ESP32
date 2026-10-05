#pragma once

#include <Arduino.h>

// ใส่ข้อความลงคิว (ส่งจริงใน telegramUpdate) — เรียกได้ทุกที่ ไม่บล็อก
void telegramNotify(const String &text);

// เรียกใน loop() — ส่งข้อความในคิวทีละข้อความเมื่อ WiFi เชื่อมต่ออยู่
void telegramUpdate();
