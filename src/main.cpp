#include <Arduino.h>

#define RELAY_ON LOW   // รีเลย์เป็นแบบ Active LOW
#define RELAY_OFF HIGH

const uint8_t RELAY_PINS[3] = {17, 18, 4}; // relay1, relay2, relay3
const uint8_t RELAY_COUNT = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
const unsigned long interval = 5000; // สลับทุก 5 วินาที

unsigned long previousMillis = 0;
uint8_t currentRelay = 0; // ตัวที่กำลังทำงานอยู่

// เปิดเฉพาะรีเลย์ตัวที่ระบุ ตัวอื่นปิดทั้งหมด
void activateRelay(uint8_t index)
{
    for (uint8_t i = 0; i < RELAY_COUNT; i++)
    {
        digitalWrite(RELAY_PINS[i], i == index ? RELAY_ON : RELAY_OFF);
    }
}

void setup()
{
    for (uint8_t pin : RELAY_PINS)
    {
        digitalWrite(pin, RELAY_OFF); // ตั้งค่าก่อน pinMode เพื่อไม่ให้รีเลย์ดีดตอนเริ่ม
        pinMode(pin, OUTPUT);
    }

    activateRelay(currentRelay); // เริ่มที่ relay1
    previousMillis = millis();
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        currentRelay = (currentRelay + 1) % RELAY_COUNT; // relay1 -> 2 -> 3 -> 1 ...
        activateRelay(currentRelay);
    }
}
