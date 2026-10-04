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
    String body;
    body.reserve(length);
    for (unsigned int i = 0; i < length; i++)
        body += (char)payload[i];
    Serial.printf("MQTT rx [%s]: %s\n", topic, body.c_str());

    JsonDocument doc;
    if (deserializeJson(doc, body) || !doc.is<JsonObject>() || !controlHandler)
    {
        Serial.println("MQTT control: invalid payload (ต้องเป็น JSON เช่น {\"relay\":1,\"state\":\"on\"})");
        return;
    }
    // relay รับได้ทั้งตัวเลขและสตริง ("1")
    int relay = doc["relay"].is<const char *>() ? atoi(doc["relay"].as<const char *>()) : (doc["relay"] | 0);

    int8_t value;
    JsonVariant s = doc["state"];
    if (s.is<bool>())
        value = s.as<bool>() ? 1 : 0;
    else if (s.is<int>())
        value = s.as<int>() ? 1 : 0;
    else
    {
        String st = s | "";
        st.toLowerCase();
        st.trim();
        if (st == "on" || st == "1" || st == "true")
            value = 1;
        else if (st == "off" || st == "0" || st == "false")
            value = 0;
        else if (st == "toggle")
            value = -1;
        else
        {
            Serial.println("MQTT control: unknown state");
            return;
        }
    }
    if (relay >= 1 && relay <= 255)
        controlHandler(relay - 1, value);
    else
        Serial.println("MQTT control: invalid relay number");
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
        bool ok = client.subscribe(topicControl.c_str());
        justConnected = true;
        Serial.printf("MQTT connected: %s, subscribe %s: %s\n", MQTT_HOST, topicControl.c_str(), ok ? "ok" : "FAILED");
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
