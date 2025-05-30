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
#include <SoftwareSerial.h>
#include <ESP8266httpUpdate.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#elif defined(ESP32)
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>

#define RTOS_DEV

#endif

union v32
{
    float f;
    uint32_t u;
    int32_t i;
};

union v64
{
    double f;
    uint64_t u;
    int64_t i;
};

class MindDevice
{
public:
    MindDevice(/* args */);
    ~MindDevice();
    WiFiMind wfmind;
    PubSubClient mqttclient;
    ModbusRTU rtu;
    JsonDocument config;
    Modbus::ResultCode rtu_code;
    time_t _now = 0;

    void begin();
    void process();
    bool loadConfig();
    bool saveConfig();
    void reloadMQTT();
    void reloadLAN();
    void reloadWAN();
    void reloadSYS();
    void reloadRTU();
    void reloadTime();
    void processMQTT();
    void sendAttribute();
    void sendTelemetry();

    uint16_t type_to_numregs(uint8_t type);
    uint64_t merge_variables(uint16_t *valregs, uint8_t numr, bool reverse = false);

private:
    WiFiClient wfclient;

    bool _wifiConnected = false;

    unsigned long _lastTelemetry = 0;
    unsigned long _lastAttribute = 0;
    unsigned long _lastReconnect = 0;

    unsigned long _lastDevAttribute = 0;

    unsigned char _attributeFrequency = 0;
    unsigned char _telemetryFrequency = 0;
    unsigned char _timeReconnect = 5;

    uint16_t _valregs[4];
    bool _boolregs;
    v32 _v32;
    v64 _v64;
    // char bufmqtt[512];
    // JsonDocument doc_for_attr;

#if defined(ESP32)
    using OTAUpdate = HTTPUpdate;
#else
    using OTAUpdate = ESP8266HTTPUpdate;
    SoftwareSerial S;
    std::function<void(const char *, unsigned int, int)> _callbackTime;
#endif
    OTAUpdate otaUpdate;

    void reconnect();
    void on_attribute(JsonDocument json);

    std::function<void(bool)> _onMQTTconnected;
    cbTransaction _rtuCallback;

public:
    void mqtt_on_message(char *topic, byte *payload, unsigned int length);

    // callback
    void setMQTTCallback(std::function<void(char *, uint8_t *, unsigned int)> func) { mqttclient.setCallback(func); }
    // void setWebServerCallback(std::function<void()> func) { wfmind.setWebServerCallback(func); }
    void onPortalOTAStart(std::function<void()> func) { wfmind.onOTAStart(func); }
    void onPortalOTAEnd(std::function<void()> func) { wfmind.onOTAEnd(func); }
    void onPortalOTAError(std::function<void(int)> func) { wfmind.onOTAError(func); }

    void onMQTTOTAStart(std::function<void()> func) { otaUpdate.onStart(func); }
    void onMQTTOTAProgress(std::function<void(int, int)> func) { otaUpdate.onProgress(func); }
    void onMQTTOTAEnd(std::function<void()> func) { otaUpdate.onEnd(func); }
    void onMQTTOTAError(std::function<void(int)> func) { otaUpdate.onError(func); }
    void onMQTTConnected(std::function<void(bool)> func) { _onMQTTconnected = func; }

    void setRTUCallback(cbTransaction func) { _rtuCallback = func; }

#ifdef ESP8266
    void setReloadNTPCallback(std::function<void(const char *, unsigned int, int)> func) { _callbackTime = func; }
#endif
};

#endif // ESP
#endif // MindDevice_h
