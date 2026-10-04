#include "mqtt.h"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "secrets.h"
#include "weather.h"

static const char *MQTT_HOST = "broker.hivemq.com";
static const uint16_t MQTT_PORT = 1883;
static const unsigned long RECONNECT_MS = 5000;

static WiFiClient net;
static PubSubClient client(net);
static RelayControlHandler controlHandler = nullptr;
static unsigned long lastAttemptMs = 0;
static bool everAttempted = false;
static bool justConnected = false;
static String boardId;
static String topicTelemetry;
static String topicControl;

// คำสั่งเป็น JSON: {"relay":1,"state":"on"|"off"|"toggle"} (relay 1-3)
static void onMessage(char *topic, uint8_t *payload, unsigned int length)
{
    JsonDocument doc;
    if (deserializeJson(doc, payload, length) || !controlHandler)
    {
        Serial.println("MQTT control: invalid payload");
        return;
    }
    int relay = doc["relay"] | 0;
    String state = doc["state"] | "";
    int8_t value;
    if (state == "on")
        value = 1;
    else if (state == "off")
        value = 0;
    else if (state == "toggle")
        value = -1;
    else
        return;
    if (relay >= 1 && relay <= 255)
        controlHandler(relay - 1, value);
}

void mqttInit(RelayControlHandler handler)
{
    controlHandler = handler;
    boardId = BOARD_ID;
    if (boardId.length() == 0)
        boardId = String("esp32-") + String((uint32_t)ESP.getEfuseMac(), HEX);
    topicTelemetry = String("esp32relay/") + boardId + "/telemetry";
    topicControl = String("esp32relay/") + boardId + "/control";
    Serial.printf("MQTT topics: %s , %s\n", topicTelemetry.c_str(), topicControl.c_str());

    client.setServer(MQTT_HOST, MQTT_PORT);
    client.setBufferSize(512);
    client.setCallback(onMessage);
}

bool mqttConnected() { return client.connected(); }

bool mqttJustConnected()
{
    bool v = justConnected;
    justConnected = false;
    return v;
}

void mqttUpdate()
{
    if (WiFi.status() != WL_CONNECTED)
        return;
    if (client.connected())
    {
        client.loop();
        return;
    }
    unsigned long now = millis();
    if (everAttempted && now - lastAttemptMs < RECONNECT_MS)
        return;
    everAttempted = true;
    lastAttemptMs = now;

    if (client.connect(boardId.c_str())) // client id = board id กันชนกันบน broker; ไม่มี user/password
    {
        client.subscribe(topicControl.c_str());
        justConnected = true;
        Serial.printf("MQTT connected: %s\n", MQTT_HOST);
    }
    else
        Serial.printf("MQTT connect failed, rc=%d\n", client.state());
}

void mqttPublishTelemetry(const bool *relays, uint8_t relayCount)
{
    if (!client.connected())
        return;

    JsonDocument doc;
    doc["board"] = boardId;
    doc["ts"] = (uint32_t)time(nullptr); // epoch วินาที (ใกล้ 0 ถ้ายัง sync NTP ไม่เสร็จ)
    JsonObject w = doc["weather"].to<JsonObject>();
    w["valid"] = weather.valid;
    if (weather.valid)
    {
        w["description"] = weather.description;
        w["temp"] = weather.temp;
        w["feels_like"] = weather.feelsLike;
        w["humidity"] = weather.humidity;
        w["aqi"] = weather.aqi;
        w["pm25"] = weather.pm25;
        w["pm10"] = weather.pm10;
    }
    JsonArray r = doc["relay"].to<JsonArray>();
    for (uint8_t i = 0; i < relayCount; i++)
        r.add(relays[i]);

    String body;
    serializeJson(doc, body);
    client.publish(topicTelemetry.c_str(), body.c_str(), true);
}
