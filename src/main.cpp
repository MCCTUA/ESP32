#include <Arduino.h>

#define RELAY_ON LOW // รีเลย์เป็นแบบ Active LOW
#define RELAY_OFF HIGH
#define SW_PRESSED LOW // สวิตช์เป็นแบบ Active LOW + external pull-up

const uint8_t RELAY_PINS[3] = {17, 18, 4}; // relay1, relay2, relay3
const uint8_t SW_PINS[3] = {34, 35, 32};   // SW1, SW2, SW3
const uint8_t CHANNEL_COUNT = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);
const unsigned long DEBOUNCE_MS = 30; // เวลาที่สัญญาณต้องนิ่งก่อนยอมรับ

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
}

void loop()
{
    unsigned long now = millis();

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
