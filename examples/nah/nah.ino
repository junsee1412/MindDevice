#include <MindDevice.h>

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
    md.setWebServerCallback(&webServer);
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
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
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
            md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
    }
}

void handleRestored()
{
    md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
    md.loadConfig();
}

void handleGetSystem()
{
    md.wfmind.handleRequest();
    String buf;
    JsonDocument sys = md.config["sys"];
    // sys["stassid"] = "hha";
    // sys["stapass"] = "hha";
    serializeJson(sys, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveSystem()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["sys"]["auth"] = doc["auth"] | false;
        md.config["sys"]["user"] = doc["user"] | "admin";
        md.config["sys"]["pass"] = doc["pass"] | "admin";
        md.config["sys"]["hostname"] = doc["hostname"] | "MIND Device";
        md.config["sys"]["port"] = doc["port"] | 80;
        md.config["sys"]["apssid"] = doc["apssid"] | "MIND Device";
        md.config["sys"]["appass"] = doc["appass"] | "mindprojects";
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadSYS();
    }
}

void handleGetWAN()
{
    md.wfmind.handleRequest();
    String buf;
    JsonDocument wan = md.config["wan"];
    serializeJson(wan, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveWAN()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["wan"]["dhcp"] = doc["dhcp"] | false;
        md.config["wan"]["ip"] = doc["ip"] | "";
        md.config["wan"]["gw"] = doc["gw"] | "";
        md.config["wan"]["mask"] = doc["mask"] | "";
        md.config["wan"]["dns"] = doc["dns"] | "";
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadWAN();
    }
}

void handleGetLAN()
{
    md.wfmind.handleRequest();
    String buf;
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
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        // md.config["lan"]["dhcp"] = doc["dhcp"] | false;
        md.config["lan"]["ip"] = doc["ip"] | "";
        md.config["lan"]["mask"] = doc["mask"] | "";
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadLAN();
    }
}

void handleGetTime()
{
    md.wfmind.handleRequest();
    String buf;
    JsonDocument time = md.config["time"];
    serializeJson(time, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveTime()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["time"]["dhcp"] = doc["dhcp"] | false;
        md.config["time"]["host"] = doc["host"] | "pool.ntp.org";
        md.config["time"]["post"] = doc["post"] | 1337;
        md.config["time"]["tz"] = doc["tz"] | 7;
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadTime();
    }
}

void handleGetRTU()
{
    md.wfmind.handleRequest();
    String buf;
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
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["rtu"]["baudrate"] = doc["baudrate"] | 9600;
        md.config["rtu"]["databit"] = doc["databit"] | 8;
        md.config["rtu"]["stopbit"] = doc["stopbit"] | 1;
        md.config["rtu"]["parity"] = doc["parity"] | NULL;
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadRTU();
    }
}

void handleGetDevices()
{
    // md.wfmind.server->
    md.wfmind.handleRequest();
    String buf;
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
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["device"] = doc.to<JsonArray>();
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
    }
}

void handleGetMapping()
{
    md.wfmind.handleRequest();
    String buf;
    JsonDocument map = md.config["map"];
    serializeJson(map, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveMapping()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["map"] = doc.to<JsonArray>();
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
    }
}

void handleGetMqtt()
{
    md.wfmind.handleRequest();
    String buf;
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
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.config["mqtt"]["id"] = doc["id"] | "";
        md.config["mqtt"]["user"] = doc["user"] | "";
        md.config["mqtt"]["pass"] = doc["pass"] | "";
        md.config["mqtt"]["host"] = doc["host"] | "";
        md.config["mqtt"]["port"] = doc["port"] | 1883;
        md.config["mqtt"]["attrf"] = doc["attrf"] | 5;
        md.config["mqtt"]["telef"] = doc["telef"] | 300;
        md.config["mqtt"]["reconnect"] = doc["reconnect"] | 5;
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.loadConfig();
        md.reloadMQTT();
    }
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
