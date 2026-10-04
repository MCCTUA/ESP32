#include "telegram.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

const uint8_t QUEUE_SIZE = 6;
const uint16_t TELEGRAM_TIMEOUT_MS = 5000;
const unsigned long RETRY_DELAY_MS = 10000; // ส่งไม่สำเร็จ รอสักครู่แล้วลองใหม่

static String queue[QUEUE_SIZE];
static uint8_t head = 0;  // ตำแหน่งข้อความที่จะส่งถัดไป
static uint8_t count = 0; // จำนวนข้อความในคิว
static unsigned long nextTryMs = 0;

void telegramNotify(const String &text)
{
    if (count == QUEUE_SIZE) // คิวเต็ม ทิ้งข้อความเก่าสุด
    {
        head = (head + 1) % QUEUE_SIZE;
        count--;
    }
    queue[(head + count) % QUEUE_SIZE] = text;
    count++;
}

static bool sendMessage(const String &text)
{
    WiFiClientSecure client;
    client.setInsecure(); // ไม่ตรวจ certificate (เหมือน OpenWeather)

    HTTPClient http;
    http.setTimeout(TELEGRAM_TIMEOUT_MS);
    if (!http.begin(client, String("https://api.telegram.org/bot") + TELEGRAM_BOT_TOKEN + "/sendMessage"))
        return false;
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["chat_id"] = TELEGRAM_CHAT_ID;
    doc["text"] = text;
    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    http.end();
    if (code != HTTP_CODE_OK)
    {
        Serial.printf("Telegram HTTP error: %d\n", code);
        return false;
    }
    return true;
}

void telegramUpdate()
{
    if (count == 0 || WiFi.status() != WL_CONNECTED)
        return;
    if ((long)(millis() - nextTryMs) < 0)
        return;

    if (sendMessage(queue[head]))
    {
        queue[head] = "";
        head = (head + 1) % QUEUE_SIZE;
        count--;
    }
    else
    {
        nextTryMs = millis() + RETRY_DELAY_MS;
    }
}
