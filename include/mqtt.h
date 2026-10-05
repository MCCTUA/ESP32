#pragma once

#include <Arduino.h>

// state: 0 = ปิด, 1 = เปิด, -1 = สลับ
using RelayControlHandler = void (*)(uint8_t index, int8_t state);

void mqttInit(RelayControlHandler handler);

// เรียกใน loop() — เชื่อมต่อ/เชื่อมต่อใหม่กับ broker (non-blocking) และรับคำสั่ง
void mqttUpdate();

bool mqttConnected();

// true หนึ่งครั้งหลังเชื่อมต่อ broker ได้ (ให้ส่ง telemetry ล่าสุดทันที)
bool mqttJustConnected();

// ส่งสภาพอากาศ + สถานะรีเลย์ไปที่ esp32relay/<BOARD_ID>/telemetry (retained)
void mqttPublishTelemetry(const bool *relays, uint8_t relayCount);
