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
        reloadSYS();
        reloadLAN();
        reloadWAN();
        loaded = true;
    }
    else
    {
        Serial.println("load fails");
        loaded = false;
    }
    wfmind.setSaveConnectTimeout(1000);
    wfmind.setConfigPortalTimeout(200);
    _wifiConnected = wfmind.autoConnect(config["sys"]["apssid"], config["sys"]["appass"]);

    if (!_wifiConnected)
    {
        ESP.restart();
    }
    wfmind.setAllowExit(false);
    wfmind.startWebPortal();

    mqttclient.setClient(wfclient);

    if (loaded)
    {
        reloadRTU();
        reloadTime();
        reloadMQTT();
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
            sendTelemetry();
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
            bool non_error = true;
            JsonDocument resDevice;
            JsonDocument doc_for_attr;
            JsonArray keys = config["map"][device["profile"] | 0].as<JsonArray>();
            for (JsonObject key : keys)
            {
                int func = key["func"] | 0;
                int device_id = device["id"] | 0;
                uint16_t offset = key["offset"] | 0;
                int type = key["type"] | 0;
                uint16_t numregs = type_to_numregs(type);
                switch (func)
                {
                case 1:
                    rtu.readCoil(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // resDevice[key["key"] | "v1"] = _boolregs;
                    break;

                case 2:
                    rtu.readIsts(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // resDevice[key["key"] | "v2"] = _boolregs;
                    break;

                case 3:
                    rtu.readHreg(device_id, offset, _valregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // resDevice[key["key"] | "v3"] = merge_variables(numregs);
                    break;

                case 4:
                    rtu.readIreg(device_id, offset, _valregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // resDevice[key["key"] | "v4"] = merge_variables(numregs);
                    break;

                default:
                    break;
                }

                while (rtu.slave())
                {
                    rtu.task();
                    delay(10);
                }

                bool res_bool = false;
                switch (type)
                {
                case 0:
                    if (func > 2)
                        res_bool = (bool)merge_variables(numregs);
                    else
                        res_bool = _boolregs;

                    resDevice[key["key"] | "bool"] = res_bool;
                    break;
                case 1:
                    resDevice[key["key"] | "int8"] = (int8_t)merge_variables(numregs);
                    break;
                case 2:
                    resDevice[key["key"] | "uint8"] = (uint8_t)merge_variables(numregs);
                    break;
                case 3:
                    resDevice[key["key"] | "int16"] = (int16_t)merge_variables(numregs);
                    break;
                case 4:
                    resDevice[key["key"] | "uint16"] = (uint16_t)merge_variables(numregs);
                    break;
                case 5:
                    resDevice[key["key"] | "int32"] = (int32_t)merge_variables(numregs);
                    break;
                case 6:
                    resDevice[key["key"] | "uint32"] = (uint32_t)merge_variables(numregs);
                    break;
                case 7:
                    resDevice[key["key"] | "int32"] = (int32_t)merge_variables(numregs, true);
                    break;
                case 8:
                    resDevice[key["key"] | "uint32"] = (uint32_t)merge_variables(numregs, true);
                    break;
                case 9:
                    resDevice[key["key"] | "int64"] = (int64_t)merge_variables(numregs);
                    break;
                case 10:
                    resDevice[key["key"] | "uint64"] = (uint64_t)merge_variables(numregs);
                    break;
                case 11:
                    resDevice[key["key"] | "int64"] = (int64_t)merge_variables(numregs, true);
                    break;
                case 12:
                    resDevice[key["key"] | "uint64"] = (uint64_t)merge_variables(numregs, true);
                    break;
                default:
                    resDevice[key["key"] | "val"] = (uint64_t)merge_variables(numregs);
                    break;
                }
            }
            non_error = non_error & (rtu_code == Modbus::EX_SUCCESS);
            if (non_error)
            {
                doc_for_attr[device["name"] | ""] = resDevice;
                serializeJson(doc_for_attr, bufmqtt);
                Serial.println(bufmqtt);
                mqttclient.publish(GATEWAY_ATTRIBUTES_TOPIC, bufmqtt, 256);
            }
        }
    }
}

void MindDevice::sendTelemetry()
{
    if (!rtu.slave())
    {
        JsonArray devices = config["device"].as<JsonArray>();
        for (JsonObject device : devices)
        {
            bool non_error = true;
            JsonDocument doc_for_tele;
            JsonDocument res_values;
            JsonArray keys = config["map"][device["profile"] | 0].as<JsonArray>();
            for (JsonObject key : keys)
            {
                int func = key["func"] | 0;
                int device_id = device["id"] | 0;
                uint16_t offset = key["offset"] | 0;
                int type = key["type"] | 0;
                uint16_t numregs = type_to_numregs(type);
                switch (func)
                {
                case 1:
                    rtu.readCoil(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // res_values[key["key"] | "v1"] = _boolregs;
                    break;

                case 2:
                    rtu.readIsts(device_id, offset, &_boolregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // res_values[key["key"] | "v2"] = _boolregs;
                    break;

                case 3:
                    rtu.readHreg(device_id, offset, _valregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // res_values[key["key"] | "v3"] = merge_variables(numregs);
                    break;

                case 4:
                    rtu.readIreg(device_id, offset, _valregs, numregs, _rtuCallback);

                    // while (rtu.slave())
                    // {
                    //     rtu.task();
                    //     delay(10);
                    // }
                    // res_values[key["key"] | "v4"] = merge_variables(numregs);
                    break;
                }

                while (rtu.slave())
                {
                    rtu.task();
                    delay(10);
                }

                bool res_bool = false;
                switch (type)
                {
                case 0:
                    if (func > 2)
                        res_bool = (bool)merge_variables(numregs);
                    else
                        res_bool = _boolregs;

                    res_values[key["key"] | "bool"] = res_bool;
                    break;
                case 1:
                    res_values[key["key"] | "int8"] = (int8_t)merge_variables(numregs);
                    break;
                case 2:
                    res_values[key["key"] | "uint8"] = (uint8_t)merge_variables(numregs);
                    break;
                case 3:
                    res_values[key["key"] | "int16"] = (int16_t)merge_variables(numregs);
                    break;
                case 4:
                    res_values[key["key"] | "uint16"] = (uint16_t)merge_variables(numregs);
                    break;
                case 5:
                    res_values[key["key"] | "int32"] = (int32_t)merge_variables(numregs);
                    break;
                case 6:
                    res_values[key["key"] | "uint32"] = (uint32_t)merge_variables(numregs);
                    break;
                case 7:
                    res_values[key["key"] | "int32"] = (int32_t)merge_variables(numregs, true);
                    break;
                case 8:
                    res_values[key["key"] | "uint32"] = (uint32_t)merge_variables(numregs, true);
                    break;
                case 9:
                    res_values[key["key"] | "int64"] = (int64_t)merge_variables(numregs);
                    break;
                case 10:
                    res_values[key["key"] | "uint64"] = (uint64_t)merge_variables(numregs);
                    break;
                case 11:
                    res_values[key["key"] | "int64"] = (int64_t)merge_variables(numregs, true);
                    break;
                case 12:
                    res_values[key["key"] | "uint64"] = (uint64_t)merge_variables(numregs, true);
                    break;
                default:
                    res_values[key["key"] | "val"] = (uint64_t)merge_variables(numregs);
                    break;
                }
            }
            non_error = non_error & (rtu_code == Modbus::EX_SUCCESS);
            if (non_error)
            {
                JsonArray array = doc_for_tele[device["name"] | ""].to<JsonArray>();
                array.add(res_values);
                serializeJson(doc_for_tele, bufmqtt);
                mqttclient.publish(GATEWAY_TELEMETRY_TOPIC, bufmqtt, 256);
            }
        }
    }
}

uint16_t MindDevice::type_to_numregs(uint8_t type)
{
    if (type <= 4)
    {
        return 1;
    }
    else if (type <= 8)
    {
        return 2;
    }
    else
    {
        return 4;
    }
}

uint64_t MindDevice::merge_variables(uint8_t numr, bool reverse)
{
    uint64_t c_var = 0;
    for (uint8_t i = 0; i < numr; i++)
    {
        if (reverse)
            uint8_t mbit = 16 * (numr - i - 1);
        else
            uint8_t mbit = 16 * i;
        c_var = c_var | _valregs[i] << (16 * i);
        _valregs[i] = 0;
    }
    return c_var;
}

// uint64_t MindDevice::merge_reverse_variables(uint8_t numr)
// {
//     uint64_t c_var = 0;
//     for (uint8_t i = 0; i < numr; i++)
//     {
//         uint8_t mbit = 16 * (numr - i - 1);
//         c_var = c_var | _valregs[i] << mbit;
//         _valregs[i] = 0;
//     }
//     return c_var;
// }

void MindDevice::reloadLAN()
{
    IPAddress ip;
    IPAddress gw;
    IPAddress sn;
    if (ip.fromString(config["lan"]["ip"] | "") &&
        gw.fromString(config["lan"]["gw"] | "") &&
        sn.fromString(config["lan"]["mask"] | ""))
    {
        wfmind.setAPStaticIPConfig(ip, gw, sn);
    }
}

void MindDevice::reloadWAN()
{
    if (config["wan"]["dhcp"] | false)
    {
        IPAddress ip;
        IPAddress gw;
        IPAddress sn;
        IPAddress dns;
        if (!ip.fromString(config["wan"]["ip"] | "") &&
            !gw.fromString(config["wan"]["gw"] | "") &&
            !sn.fromString(config["wan"]["mask"] | ""))
        {
            return;
        }

        if (dns.fromString(config["wan"]["dns"] | ""))
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
    static uint32_t SConfig = SERIAL_8N1;
    if (config["rtu"]["databit"] | 7 == 8)
        SConfig += 0x0000004;

    if (config["rtu"]["stopbit"] | 1 == 2)
        SConfig += 0x0000020;

    int parity = config["rtu"]["parity"] | 0;
    switch (parity)
    {
    case 2:
        SConfig += 0x0000002;
        break;
    case 3:
        SConfig += 0x0000003;
        break;
    }

    Serial2.begin(config["rtu"]["baudrate"], SConfig, 17, 16);
    rtu.begin(&Serial2);
#else
    S.end();
    S.begin(config["rtu"]["baudrate"], SWSERIAL_8N1, 14, 12);
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

void MindDevice::reloadSYS()
{
    wfmind.setAllowBasicAuth(config["sys"]["auth"] | false);
    wfmind.setHostname(config["sys"]["hostname"] | "MIND Device");
    wfmind.setHttpPort(config["sys"]["port"] | 80);
    // wfmind.setAP
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

#endif
