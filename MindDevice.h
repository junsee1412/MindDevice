#ifndef MindDevice_h
#define MindDevice_h

#if defined(ESP8266) || defined(ESP32)
#include <WiFiMind.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ModbusTCP.h>
#include <ModbusRTU.h>
#include <LittleFS.h>
#include <RTClib.h>
#include "com_def.h"

#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>
#endif

class MindDevice
{
public:
    MindDevice(/* args */);
    ~MindDevice();
    WiFiMind wfmind;
    PubSubClient mqttclient;
    JsonDocument config;

    void begin();
    void process();
    bool loadConfig();
    bool saveConfig();
    void reloadMQTT();
    void reloadNTP();
    // void setAttributeFrequency(unsigned char freq);
    // void setTelemetryFrequency(unsigned char freq);

private:
    WiFiClient wfclient;

    bool _wifiConnected = false;

    unsigned long _now = 0;
    unsigned long _lastTelemetry = 0;
    unsigned long _lastAttribute = 0;
    unsigned long _lastReconnect = 0;

    unsigned char _attributeFrequency = 0;
    unsigned char _telemetryFrequency = 0;
    unsigned char _timeReconnect = 5;

    std::function<void(bool)> _onMQTTconnected;
#if defined(ESP32)
    using OTAUpdate = HTTPUpdate;
#else
    using OTAUpdate = ESP8266HTTPUpdate;

    WiFiUDP ntpUDP;
    NTPClient** configTime(WiFiUDP ntpUDP, const char* poolServerName);
#endif
    OTAUpdate otaUpdate;

    void reconnect();
    void on_attribute(JsonDocument json);

public:
    void mqtt_on_message(char *topic, byte *payload, unsigned int length);

    // callback
    void setMQTTCallback(std::function<void(char *, uint8_t *, unsigned int)> func) { mqttclient.setCallback(func); }
    void setWebServerCallback(std::function<void()> func) { wfmind.setWebServerCallback(func); }
    void onPortalOTAStart(std::function<void()> func) { wfmind.onOTAStart(func); }
    void onPortalOTAEnd(std::function<void()> func) { wfmind.onOTAEnd(func); }
    void onPortalOTAError(std::function<void(int)> func) { wfmind.onOTAError(func); }

    void onMQTTOTAStart(std::function<void()> func) { otaUpdate.onStart(func); }
    void onMQTTOTAProgress(std::function<void(int, int)> func) { otaUpdate.onProgress(func); }
    void onMQTTOTAEnd(std::function<void()> func) { otaUpdate.onEnd(func); }
    void onMQTTOTAError(std::function<void(int)> func) { otaUpdate.onError(func); }
    void onMQTTConnected(std::function<void(bool)> func) { _onMQTTconnected = func; }
};

#endif // ESP
#endif // MindDevice_h
