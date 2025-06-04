#include <ArduinoJson.h>
#include <MindDevice.h>
#include "esp_mac.h"

#include "styles.css.h"
#include "index.html.h"
#include "lan.html.h"
#include "mapping.html.h"
#include "modbus.html.h"
#include "mqtt.html.h"
#include "other.html.h"
#include "slaves.html.h"
#include "systems.html.h"
#include "wan.html.h"
#include "wifi.html.h"

#define TRIGGER_PIN 0
#ifdef ESP32
#define LED_BUILTIN 13
#endif

#ifdef ESP8266
WiFiUDP _udp;
NTPClient ntp(_udp);
#endif

#include <EasyButton.h>
EasyButton button(TRIGGER_PIN);
void buttonISR()
{
    button.read();
}

MindDevice md;

#ifdef RTOS_DEV
void taskMQTT(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {
        md.processMQTT();
    }
}
#endif

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    button.begin();
    button.onPressedFor(3000, reset_setting);
    //   button.onSequence()
    //   if (button.supportsInterrupt())
    //   {
    button.enableInterrupt(buttonISR);

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

#ifdef RTOS_DEV
    xTaskCreate(taskMQTT, "MQTT Task", 3072, NULL, 0, NULL);
#endif
}

void loop()
{
    button.update();
#ifdef ESP8266
    md._now = ntp.getEpochTime();
#endif
    md.process();
}

void webServer()
{
    md.wfmind.server->on("/", handleIndex);
    md.wfmind.server->on("/lan.html", handleLAN);
    md.wfmind.server->on("/mapping.html", handleMapping);
    md.wfmind.server->on("/modbus.html", handleModbus);
    md.wfmind.server->on("/mqtt.html", handleMQTT);
    md.wfmind.server->on("/other.html", handleOther);
    md.wfmind.server->on("/slaves.html", handleSlaves);
    md.wfmind.server->on("/systems.html", handleSystem);
    md.wfmind.server->on("/wan.html", handleWAN);
    md.wfmind.server->on("/wifi.html", handleWiFi);
    md.wfmind.server->on("/styles.css", handleStyles);
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

void handleIndex()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_index_html, web_out_index_html_len);
}
void handleLAN()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_lan_html, web_out_lan_html_len);
}
void handleMapping()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_mapping_html, web_out_mapping_html_len);
}
void handleModbus()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_modbus_html, web_out_modbus_html_len);
}
void handleMQTT()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_mqtt_html, web_out_mqtt_html_len);
}
void handleOther()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_other_html, web_out_other_html_len);
}
void handleSlaves()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_slaves_html, web_out_slaves_html_len);
}
void handleSystem()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_systems_html, web_out_systems_html_len);
}
void handleWAN()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_wan_html, web_out_wan_html_len);
}
void handleWiFi()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/html", web_out_wifi_html, web_out_wifi_html_len);
}
void handleStyles()
{
    md.wfmind.handleRequest();
    md.wfmind.server->send_P(200, "text/css", web_out_styles_css, web_out_styles_css_len);
}

void handleGetInfo()
{
    md.wfmind.handleRequest();
    char buf[256];
    JsonDocument info;
    info["product"] = "ED485";
    uint8_t base_mac[6];
    esp_base_mac_addr_get(base_mac);
    char base_mac_char[18] = {0};
    sprintf(base_mac_char,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            base_mac[0], base_mac[1], base_mac[2],
            base_mac[3], base_mac[4], base_mac[5]);
    info["mac"] = base_mac_char;
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
    info["rssi"] = WiFi.RSSI();
    serializeJson(info, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleBackup()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    char buf[256];
    // JsonDocument sys = md.config["sys"];
    // sys["stassid"] = "hha";
    // sys["stapass"] = "hha";
    serializeJson(md.config["sys"], buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveSystem()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    char buf[256];
    // JsonDocument wan = md.config["wan"];
    serializeJson(md.config["wan"], buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveWAN()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    char buf[256];
    JsonDocument lan = md.config["lan"];
    serializeJson(lan, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveLAN()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["lan"]["ip"] = doc["ip"] | "";
    md.config["lan"]["mask"] = doc["mask"] | "";
    md.config["lan"]["gw"] = doc["gw"] | "";
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
    md.wfmind.handleRequest();
    char buf[256];
    JsonDocument time = md.config["time"];
    time["now"] = md._now;
    serializeJson(time, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveTime()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error)
    {
        md.wfmind.server->send(500, JSONTYPE, FPSTR(HTTP_MESSAGE_500));
        return;
    }
    md.config["time"]["ntp"] = doc["ntp"] | false;
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
    md.wfmind.handleRequest();
    char buf[256];
    JsonDocument time = md.config["rtu"];
    serializeJson(time, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveRTU()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    // md.wfmind.server->
    char buf[1024];
    JsonDocument device = md.config["device"];
    serializeJson(device, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveDevices()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    // char buf[4096];
    char *buf = (char *)malloc(sizeof(char) * 4096);
    JsonDocument map = md.config["map"];
    // serializeJson(map, buf);
    serializeJson(map, buf, 4096);
    md.wfmind.server->send(200, JSONTYPE, buf);
    free(buf);
}

void handleSaveMapping()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    char buf[512];
    JsonObject network = md.config["mqtt"];
    serializeJson(network, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveMqtt()
{
    md.wfmind.handleRequest();
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
    md.wfmind.handleRequest();
    int id = (int)md.wfmind.server->arg("id").toInt() | 0;
    uint8_t profile = (uint8_t)md.wfmind.server->arg("profile").toInt() | 0;
    JsonArray keys = md.config["map"][profile].as<JsonArray>();

    JsonDocument res_values;
    bool res_bool = false;
    uint16_t _valregs[4] = {0};
    char buf[256];

    v32 val_32;
    v64 val_64;

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
        case 11:
        case 12:
            res_values[key["key"] | "num"] = md.merge_variables(_valregs, numregs);
            break;
        case 9:
            val_32.u = md.merge_variables(_valregs, numregs);
            res_values[key["key"] | "num"] = val_32.f;
            break;
        case 15:
            val_64.u = md.merge_variables(_valregs, numregs);
            res_values[key["key"] | "num"] = val_64.f;
            break;
        case 7:
        case 8:
        case 13:
        case 14:
            res_values[key["key"] | "num"] = md.merge_variables(_valregs, numregs, true);
            break;
        case 10:
            val_32.u = md.merge_variables(_valregs, numregs, true);
            res_values[key["key"] | "num"] = val_32.f;
            break;
        case 16:
            val_64.u = md.merge_variables(_valregs, numregs, true);
            res_values[key["key"] | "num"] = val_64.f;
            break;
        }
    }
    serializeJson(res_values, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleGetByKey()
{
    md.wfmind.handleRequest();
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

void reset_setting()
{
    md.wfmind.resetSettings();
    delay(500);
    ESP.restart();
}
