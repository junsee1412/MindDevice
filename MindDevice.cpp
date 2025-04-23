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
    bool loaded = false;
    if (loadConfig())
    {
        Serial.println("loaded");
        loaded = true;
    }
    else
    {
        Serial.println("load fails");
        loaded = false;
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

    if (loaded)
    {
        reloadMQTT();
        reloadRTU();
        reloadTime();
    }
}

void MindDevice::process()
{
#ifdef ESP32
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    time(&_now);
#endif

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
            char msg[128] = {0};
            sprintf(
                msg,
                "{\"RSSI\":%d,\"SSID\":\"%s\",\"IP\":\"%s\",\"MAC\":\"%s\"}",
                wfmind.getRSSIasQuality(WiFi.RSSI()),
                WiFi.SSID().c_str(),
                WiFi.localIP().toString().c_str(),
                WiFi.macAddress().c_str());
            mqttclient.publish(DEVICE_ATTRIBUTES_TOPIC, msg, 32);
            sendAttribute();
        }
        if (_now - _lastTelemetry >= _telemetryFrequency)
        {
            _lastTelemetry = _now;
            // sendTelemetry();
        }
    }
}

void MindDevice::sendAttribute()
{
    if (!rtu.slave())
    {
        JsonArray devices = config["device"].as<JsonArray>();
        for (JsonObject device : devices)
        {
            JsonDocument resDevice;
            Serial.println();
            Serial.println(device["name"] | "");
            JsonArray keys = config["map"][device["profile"] | 0].as<JsonArray>();
            for (JsonObject key : keys)
            {
                // while (rtu.slave())
                // {
                //     rtu.task();
                //     delay(10);
                // }
                // resDevice[key["key"] | "bool"] = rand();
                int func = key["func"] | 0;
                int device_id = device["id"] | 0;
                uint16_t offset = key["offset"] | 0;
                uint16_t numregs = type_to_numregs(key["type"] | 0);
                Serial.printf("%d %d %u %u\n", device_id, func, offset, numregs);
                switch (func)
                {
                case 1:
                    rtu.readCoil(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    while (rtu.slave())
                    {
                        rtu.task();
                        delay(10);
                    }

                    resDevice[key["key"] | "v1"] = _boolregs;
                    break;
                case 2:
                    rtu.readIsts(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    while (rtu.slave())
                    {
                        rtu.task();
                        delay(10);
                    }

                    resDevice[key["key"] | "v2"] = _boolregs;
                    break;
                case 3:
                    rtu.readHreg(device_id, offset, _numregs, numregs, _rtuCallback);

                    while (rtu.slave())
                    {
                        rtu.task();
                        delay(10);
                    }
                    resDevice[key["key"] | "v3"] = merge_variables(numregs);
                    break;

                case 4:
                    rtu.readIreg(device_id, offset, _numregs, numregs, _rtuCallback);

                    while (rtu.slave())
                    {
                        rtu.task();
                        delay(10);
                    }
                    resDevice[key["key"] | "v4"] = merge_variables(numregs);
                    break;

                default:
                    break;
                }
            }
            serializeJson(resDevice, Serial);
        }
    }
}

uint16_t MindDevice::type_to_numregs(uint8_t type)
{
    if (type <= 4)
    {
        return 1;
    }
    else if (type <= 6)
    {
        return 2;
    }
    else
    {
        return 4;
    }
}

uint64_t MindDevice::merge_variables(uint8_t numr)
{
    uint64_t c_var = 0;
    for (uint8_t i = 0; i < numr; i++)
    {
        Serial.println(".");
        c_var = c_var | _numregs[i] << (16 * i);
        _numregs[i] = 0;
    }
    return c_var;
}

void MindDevice::reloadNetwork()
{
    if (config["network"]["dhcp"] | false)
    {
        IPAddress ip;
        IPAddress gw;
        IPAddress sn;
        IPAddress dns;
        if (!ip.fromString(config["network"]["ip"] | "") &&
            !gw.fromString(config["network"]["gw"] | "") &&
            !sn.fromString(config["network"]["mask"] | ""))
        {
            return;
        }

        if (dns.fromString(config["network"]["dns"] | ""))
        {
            wfmind.setSTAStaticIPConfig(ip, gw, sn, dns);
        }
        else
        {
            wfmind.setSTAStaticIPConfig(ip, gw, sn);
        }
    }
}

void MindDevice::reloadMQTT()
{
    mqttclient.disconnect();
    mqttclient.setServer(
        config["mqtt"]["host"] | "",
        config["mqtt"]["port"] | 1883);
    _attributeFrequency = config["mqtt"]["attrf"] | 5;
    _telemetryFrequency = config["mqtt"]["telef"] | 300;
    _timeReconnect = config["mqtt"]["reconnect"] | 5;
}

void MindDevice::reloadRTU()
{
#if defined(ESP32)
    Serial2.end();
    Serial2.begin(config["rtu"]["baudrate"], SERIAL_8N1, 16, 17);
    rtu.begin(&Serial2);
#else
    S.end();
    S.begin(config["rtu"]["baudrate"], SWSERIAL_8N1, 12, 14);
    rtu.begin(&S);
#endif
    rtu.master();
}

void MindDevice::reloadTime()
{
#ifdef ESP32
    int time_offset = config["time"]["tz"] | 7;
    configTime(time_offset, 0, config["time"]["host"] | "pool.ntp.org");
#else defined(ESP8266)
    if (_callbackTime)
    {
        _callbackTime(
            config["time"]["host"] | "pool.ntp.org",
            config["time"]["post"] | NTP_DEFAULT_LOCAL_PORT,
            config["time"]["tz"] | 7);
    }
#endif
}

void MindDevice::reconnect()
{
    if (!mqttclient.connected() && _now - _lastReconnect > _timeReconnect)
    {
        Serial.println("reconn");
        if (mqttclient.connect(
                config["mqtt"]["id"] | "",
                config["mqtt"]["user"] | "",
                config["mqtt"]["pass"] | ""))
        {
            mqttclient.subscribe(DEVICE_ATTRIBUTES_TOPIC);
            mqttclient.subscribe(DEVICE_ATTRIBUTES_TOPIC_RESPONSE);

            if (_onMQTTconnected)
            {
                // callback digitalWrite(LEDMQTT_PIN, LOW);
                _onMQTTconnected(true);
            }
        }
        else
        {
            _lastReconnect = _now;
            if (_onMQTTconnected)
            {
                // callback digitalWrite(LEDMQTT_PIN, HIGH);
                _onMQTTconnected(false);
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
#ifdef ESP8266
    if (!LittleFS.begin())
#elif defined(ESP32)
    if (!LittleFS.begin(true))
#endif
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
#ifdef ESP8266
    if (!LittleFS.begin())
#elif defined(ESP32)
    if (!LittleFS.begin(true))
#endif
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
