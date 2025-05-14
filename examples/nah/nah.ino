#include <ArduinoJson.h>
#include <MindDevice.h>
// #include "index.h"
// #include "styles.h"
// #include "script.h"

#ifdef ESP32
#define LED_BUILTIN 13
#endif

#ifdef ESP8266
WiFiUDP _udp;
NTPClient ntp(_udp);
#endif

MindDevice md;

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    // md.setWebServerCallback(&webServer);
    md.wfmind.setWebServerCallback(&webServer);
    md.onPortalOTAStart(&otaStart);
    md.onPortalOTAEnd(&otaEnd);
    md.onPortalOTAError(&otaError);
    md.onMQTTOTAStart(&otaStart);
    md.onMQTTOTAProgress(&otaProgress);
    md.onMQTTOTAEnd(&otaEnd);
    md.onMQTTOTAError(&otaError);
    md.onMQTTConnected(&onMQTT);

#ifdef ESP8266
    md.setReloadNTPCallback(&onReloadNTP);
#endif

    md.setMQTTCallback(&on_message);
    md.setRTUCallback(&cbWrite);

    md.begin();
    // #ifdef ESP8266
    //     onReloadNTP("pool.ntp.org", NTP_DEFAULT_LOCAL_PORT, 7);
    // #endif
    Serial.println(WiFi.localIP());
}

void loop()
{
#ifdef ESP8266
    md._now = ntp.getEpochTime();
#endif
    md.process();
}

void webServer()
{
    md.wfmind.server->on("/api/info", HTTP_GET, handleGetInfo);
    md.wfmind.server->on("/api/key", HTTP_GET, handleGetByKey);
    md.wfmind.server->on("/api/node", HTTP_GET, handleGetByProfile);
    md.wfmind.server->on("/api/system", HTTP_GET, handleGetSystem);
    md.wfmind.server->on("/api/system", HTTP_POST, handleSaveSystem);
    md.wfmind.server->on("/api/wan", HTTP_GET, handleGetWAN);
    md.wfmind.server->on("/api/wan", HTTP_POST, handleSaveWAN);
    md.wfmind.server->on("/api/lan", HTTP_GET, handleGetLAN);
    md.wfmind.server->on("/api/lan", HTTP_POST, handleSaveLAN);
    md.wfmind.server->on("/api/time", HTTP_GET, handleGetTime);
    md.wfmind.server->on("/api/time", HTTP_POST, handleSaveTime);
    md.wfmind.server->on("/api/rtu", HTTP_GET, handleGetRTU);
    md.wfmind.server->on("/api/rtu", HTTP_POST, handleSaveRTU);
    md.wfmind.server->on("/api/mqtt", HTTP_GET, handleGetMqtt);
    md.wfmind.server->on("/api/mqtt", HTTP_POST, handleSaveMqtt);
    md.wfmind.server->on("/api/devices", HTTP_GET, handleGetDevices);
    md.wfmind.server->on("/api/devices", HTTP_POST, handleSaveDevices);
    md.wfmind.server->on("/api/mapping", HTTP_GET, handleGetMapping);
    md.wfmind.server->on("/api/mapping", HTTP_POST, handleSaveMapping);
    md.wfmind.server->on("/api/backup", HTTP_GET, handleBackup);
    md.wfmind.server->on("/api/restore", HTTP_POST, handleRestored, handleRestore);
}

void handleGetInfo()
{
    char buf[256];
    JsonDocument info;
    info["product"] = "ED485";
    info["mac"] = WiFi.macAddress();
    info["ip"] = md.config["wan"]["ip"];
    info["dhcp"] = md.config["wan"]["dhcp"];
    info["mask"] = md.config["wan"]["mask"];
    info["gw"] = md.config["wan"]["gw"];
    info["dns"] = md.config["wan"]["dns"];
    info["version"] = "0.0.1";
    info["time"] = md.config["time"]["ntp"];

#ifdef ESP8266
    info["uptime"] = millis();
#elif defined(ESP32)
    info["uptime"] = esp_timer_get_time();
#endif
    info["freememory"] = ESP.getFreeHeap();
    info["rssi"] = md.wfmind.getRSSIasQuality(WiFi.RSSI());
    serializeJson(info, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleBackup()
{
#ifdef ESP8266
    if (!LittleFS.begin())
#elif defined(ESP32)
    if (!LittleFS.begin(true))
#endif
    {
        Serial.println(F("LittleFS mount failed"));
        return;
    }
    // Open file for reading
    File download = LittleFS.open(FILE_CONFIG_PATH, "r");
    if (download)
    {
        md.wfmind.server->sendHeader(F("Content-Type"), JSONTYPE);
        md.wfmind.server->sendHeader(F("Content-Disposition"), "attachment; filename=" + String(FILE_CONFIG_NAME));
        md.wfmind.server->streamFile(download, FILEDOWNLOADTYPE);
        download.close();
    }
    else
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
    }
}
File fsUploadFile;
void handleRestore()
{
    HTTPUpload &upload = md.wfmind.server->upload();
    if (upload.status == UPLOAD_FILE_START)
    {
#ifdef ESP8266
        if (!LittleFS.begin())
#elif defined(ESP32)
        if (!LittleFS.begin(true))
#endif
        {
            Serial.println(F("LittleFS mount failed"));
            return;
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
        fsUploadFile = LittleFS.open(FILE_CONFIG_PATH, WRITE_FILE);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
            fsUploadFile.close();
        }
        else
        {
            md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
    }
}

void handleRestored()
{
    md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
    md.loadConfig();
}

void handleGetSystem()
{
    char buf[256];
    // JsonDocument sys = md.config["sys"];
    // sys["stassid"] = "hha";
    // sys["stapass"] = "hha";
    serializeJson(md.config["sys"], buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveSystem()
{
    // md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["sys"]["auth"] = doc["auth"] | false;
    md.config["sys"]["user"] = doc["user"] | "admin";
    md.config["sys"]["pass"] = doc["pass"] | "admin";
    md.config["sys"]["hostname"] = doc["hostname"] | "MIND Device";
    md.config["sys"]["port"] = doc["port"] | 80;
    md.config["sys"]["apssid"] = doc["apssid"] | "MIND Device";
    md.config["sys"]["appass"] = doc["appass"] | "mindprojects";
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadSYS();
    }
}

void handleGetWAN()
{
    char buf[256];
    // JsonDocument wan = md.config["wan"];
    serializeJson(md.config["wan"], buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveWAN()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["wan"]["dhcp"] = doc["dhcp"] | false;
    md.config["wan"]["ip"] = doc["ip"] | "";
    md.config["wan"]["gw"] = doc["gw"] | "";
    md.config["wan"]["mask"] = doc["mask"] | "";
    md.config["wan"]["dns"] = doc["dns"] | "";
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadWAN();
    }
}

void handleGetLAN()
{
    char buf[256];
    JsonDocument lan = md.config["lan"];
    serializeJson(lan, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveLAN()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    // md.config["lan"]["dhcp"] = doc["dhcp"] | false;
    md.config["lan"]["ip"] = doc["ip"] | "";
    md.config["lan"]["mask"] = doc["mask"] | "";
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadLAN();
    }
}

void handleGetTime()
{
    char buf[256];
    JsonDocument time = md.config["time"];
    time["now"] = md._now;
    serializeJson(time, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveTime()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["time"]["dhcp"] = doc["dhcp"] | false;
    md.config["time"]["host"] = doc["host"] | "pool.ntp.org";
    md.config["time"]["post"] = doc["post"] | 1337;
    md.config["time"]["tz"] = doc["tz"] | 7;
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadTime();
    }
}

void handleGetRTU()
{
    char buf[256];
    JsonDocument time = md.config["rtu"];
    serializeJson(time, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveRTU()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["rtu"]["baudrate"] = doc["baudrate"] | 9600;
    md.config["rtu"]["databit"] = doc["databit"] | 8;
    md.config["rtu"]["stopbit"] = doc["stopbit"] | 1;
    md.config["rtu"]["parity"] = doc["parity"] | 0;
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadRTU();
    }
}

void handleGetDevices()
{
    // md.wfmind.server->
    char buf[1024];
    JsonDocument device = md.config["device"];
    serializeJson(device, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveDevices()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["device"] = doc.to<JsonArray>();
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
    }
}

void handleGetMapping()
{
    char buf[1024];
    JsonDocument map = md.config["map"];
    serializeJson(map, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveMapping()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["map"] = doc.to<JsonArray>();
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
    }
}

void handleGetMqtt()
{
    char buf[512];
    JsonObject network = md.config["mqtt"];
    serializeJson(network, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveMqtt()
{
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["mqtt"]["id"] = doc["id"] | "";
    md.config["mqtt"]["user"] = doc["user"] | "";
    md.config["mqtt"]["pass"] = doc["pass"] | "";
    md.config["mqtt"]["host"] = doc["host"] | "";
    md.config["mqtt"]["port"] = doc["port"] | 1883;
    md.config["mqtt"]["attrf"] = doc["attrf"] | 5;
    md.config["mqtt"]["telef"] = doc["telef"] | 300;
    md.config["mqtt"]["reconnect"] = doc["reconnect"] | 5;
    if (!md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVE_FAILED));
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, FPSTR(HTTP_MESSAGE_SAVED));
        md.loadConfig();
        md.reloadMQTT();
    }
}

void handleGetByProfile()
{
    int id = (int)md.wfmind.server->arg("id").toInt() | 0;
    uint8_t profile = (uint8_t)md.wfmind.server->arg("profile").toInt() | 0;
    JsonArray keys = md.config["map"][profile].as<JsonArray>();

    JsonDocument res_values;
    bool res_bool = false;
    uint16_t _valregs[4] = {0};
    char buf[256];

    for (JsonObject key : keys)
    {
        int func = key["func"] | 0;
        uint16_t offset = key["offset"] | 0;
        int type = key["type"] | 0;
        uint16_t numregs = md.type_to_numregs(type);
        switch (func)
        {
        case 1:
            md.rtu.readCoil(id, offset, &res_bool, numregs, &cbWrite);
            break;

        case 2:
            md.rtu.readIsts(id, offset, &res_bool, numregs, &cbWrite);
            break;

        case 3:
            md.rtu.readHreg(id, offset, _valregs, numregs, &cbWrite);
            break;

        case 4:
            md.rtu.readIreg(id, offset, _valregs, numregs, &cbWrite);
            break;
        }

        while (md.rtu.slave())
        {
            md.rtu.task();
            delay(10);
        }

        switch (type)
        {
        case 0:
            if (func > 2)
                res_bool = (bool)md.merge_variables(_valregs, numregs);
            res_values[key["key"] | "bool"] = res_bool;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 9:
        case 10:
            res_values[key["key"] | "num"] = md.merge_variables(_valregs, numregs);
            break;
        case 7:
        case 8:
        case 11:
        case 12:
            res_values[key["key"] | "num"] = md.merge_variables(_valregs, numregs, true);
            break;
        }
    }
    serializeJson(res_values, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleGetByKey()
{
    int id = (int)md.wfmind.server->arg("id").toInt() | 0;
    int func = (int)md.wfmind.server->arg("func").toInt() | 0;
    uint16_t numregs = (uint16_t)md.wfmind.server->arg("numregs").toInt() | 0;
    uint16_t offset = (uint16_t)md.wfmind.server->arg("offset").toInt() | 0;

    JsonDocument res_values;
    bool _boolregs[4] = {0};
    uint16_t _valregs[4] = {0};
    char buf[256];

    if (numregs > 4 || id > UINT8_MAX)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }

    switch (func)
    {
    case 1:
        md.rtu.readCoil(id, offset, _boolregs, numregs, &cbWrite);
        break;

    case 2:
        md.rtu.readIsts(id, offset, _boolregs, numregs, &cbWrite);
        break;

    case 3:
        md.rtu.readHreg(id, offset, _valregs, numregs, &cbWrite);
        break;

    case 4:
        md.rtu.readIreg(id, offset, _valregs, numregs, &cbWrite);
        break;
    default:
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }

    while (md.rtu.slave())
    {
        md.rtu.task();
        delay(10);
    }

    res_values["event"] = md.rtu_code;
    JsonArray array = res_values["data"].to<JsonArray>();
    switch (func)
    {
    case 1:
    case 2:
        for (uint16_t i = 0; i < numregs; i++)
            array.add(_boolregs[i]);
        break;
    case 3:
    case 4:
        for (uint16_t i = 0; i < numregs; i++)
            array.add(_valregs[i]);
        break;
    }
    serializeJson(res_values, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void otaStart()
{
    Serial.println("start update");
}

void otaProgress(int cur, int total)
{
    Serial.printf("%u%%\n", 100 * cur / total);
}

void otaEnd()
{
    Serial.println("end update");
}

void otaError(int err)
{
    Serial.printf("error update %d", err);
}

#ifdef ESP8266
void onReloadNTP(const char *poolServerName, unsigned int port, int tz)
{
    ntp.setPoolServerName(poolServerName);
    ntp.setTimeOffset(tz * 3600);
    ntp.begin(port);
}
#endif

void onMQTT(bool connected)
{
#ifdef ESP8266
    int VAL = LOW;
#elif defined(ESP32)
    int VAL = HIGH;
#endif
    if (connected)
    {
        digitalWrite(LED_BUILTIN, HIGH ^ VAL);
        Serial.println("connected");
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW ^ VAL);
        Serial.println("disconnected");
    }
}

void on_message(char *topic, byte *payload, unsigned int length)
{
    md.mqtt_on_message(topic, payload, length);
}

bool cbWrite(Modbus::ResultCode event, uint16_t transactionId, void *data)
{
    //   resultCode = event;
    if (event != Modbus::EX_SUCCESS)
    {
        md.rtu_code = event;
        Serial.println(event, HEX);
    }
    return true;
}
