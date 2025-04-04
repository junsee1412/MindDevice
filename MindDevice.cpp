#include "MindDevice.h"
#if defined(ESP8266) || defined(ESP32)

MindDevice::MindDevice()
{
}

MindDevice::~MindDevice()
{
}

void MindDevice::begin()
{
    if (loadConfig())
    {
        Serial.println("loaded");
    }
    else
    {
        Serial.println("load fails");
    }
    wfmind.setSaveConnectTimeout(1000);
    wfmind.setConfigPortalTimeout(200);
    _wifiConnected = wfmind.autoConnect("MIND Dev", "mindprojects");

    if (!_wifiConnected)
    {
        ESP.restart();
    }
    wfmind.setAllowExit(false);
    wfmind.startWebPortal();

    mqttclient.setClient(wfclient);
    reloadMQTT();
}

void MindDevice::process()
{
    wfmind.process();
    if (!mqttclient.connected())
    {
        reconnect();
    }
    else
    {
        mqttclient.loop();

        if (_now - _lastAttribute >= _attributeFrequency)
        {
            _lastAttribute = _now;
            char msg[32] = {0};
            sprintf(msg, "{\"RSSI\":%d}", wfmind.getRSSIasQuality(WiFi.RSSI()));
            mqttclient.publish(DEVICE_ATTRIBUTES_TOPIC, msg, 32);
        }
        if (_now - _lastTelemetry >= _telemetryFrequency)
        {
            _lastTelemetry = _now;
        }
    }
}

void MindDevice::reloadMQTT()
{
    mqttclient.disconnect();
    mqttclient.setServer(
        // config["mqtt"]["host"] | "",
        // config["mqtt"]["port"] | 1883);
        "10.10.1.102",
        32686);
}

void MindDevice::reloadNTP()
{}

void MindDevice::reconnect()
{
    if (!mqttclient.connected() && _now - _lastReconnect > _timeReconnect)
    {
        Serial.println("reconn");
        if (mqttclient.connect(
                // config["mqtt"]["id"] | "",
                // config["mqtt"]["user"] | "",
                // config["mqtt"]["pass"] | ""))
                "xejOfRHvHrckkuHJCYoV",
                "lkDuOpytoycxZOwWCHsu",
                "veUHqZHslwsTKFaKpCCb"))
        {
            mqttclient.subscribe(DEVICE_ATTRIBUTES_TOPIC);
            mqttclient.subscribe(DEVICE_ATTRIBUTES_TOPIC_RESPONSE);

            if (_onMQTTconnected)
            {
                // callback digitalWrite(LEDMQTT_PIN, LOW);
                _onMQTTconnected(false);
            }
        }
        else
        {
            _lastReconnect = _now;
            if (_onMQTTconnected)
            {
                // callback digitalWrite(LEDMQTT_PIN, HIGH);
                _onMQTTconnected(true);
            }
        }
    }
}

void MindDevice::mqtt_on_message(char *topic, byte *payload, unsigned int length)
{
    char json[length + 1];
    strncpy(json, (char *)payload, length);
    json[length] = '\0';
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error)
        return;
    on_attribute(doc);
}

void MindDevice::on_attribute(JsonDocument json)
{
    const char *fw_url = json[F("fw_url")] | "";
    if (strcmp(fw_url, "") != 0)
        otaUpdate.update(wfclient, fw_url);
}

bool MindDevice::loadConfig()
{
    if (!LittleFS.begin())
    {
        return false;
    }
    // Open file for reading
    File file = LittleFS.open(FILE_CONFIG_PATH, READ_FILE);

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(config, file);
    if (error)
    {
        return false;
    }
    return true;
}

bool MindDevice::saveConfig()
{
    bool saved = true;
    if (!LittleFS.begin())
    {
        return false;
    }
    // Delete existing file, otherwise the configuration is appended to the file
    if (LittleFS.remove(FILE_CONFIG_PATH))
    {
        Serial.println(F("File deleted"));
    }
    else
    {
        Serial.println(F("Delete failed"));
    }

    // Open file for writing
    File file = LittleFS.open(FILE_CONFIG_PATH, WRITE_FILE);
    if (!file)
    {
        Serial.println(F("Failed to create file"));
        return false;
    }

    if (serializeJson(config, file) == 0)
    {
        saved = false;
        Serial.println(F("Failed to write to file"));
    }

    // Close the file
    file.close();
    LittleFS.end();

    return saved;
}
// void MindDevice::setAttributeFrequency(unsigned char freq)
// {
//     _attributeFrequency = freq;
// }

// void MindDevice::setTelemetryFrequency(unsigned char freq)
// {
//     _telemetryFrequency = freq;
// }

#endif
