#include <Arduino.h>
#line 1 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
#include <MindDevice.h>

#ifdef ESP32
#define LED_BUILTIN 2
#endif
MindDevice md;

#line 8 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void setup();
#line 26 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void loop();
#line 31 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void webServer();
#line 42 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleBackup();
#line 64 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleRestore();
#line 106 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleRestored();
#line 111 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleGetMqtt();
#line 120 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleSaveMqtt();
#line 146 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleGetNetwork();
#line 155 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void handleSaveNetwork();
#line 188 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void otaStart();
#line 193 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void otaProgress(int cur, int total);
#line 198 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void otaEnd();
#line 203 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void otaError(int err);
#line 208 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void onMQTT(bool connected);
#line 222 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void on_message(char *topic, byte *payload, unsigned int length);
#line 8 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    md.setWebServerCallback(&webServer);
    // md.onPortalOTAStart(&otaStart);
    // md.onPortalOTAEnd(&otaEnd);
    // md.onPortalOTAError(&otaError);
    // md.onMQTTOTAStart(&otaStart);
    // md.onMQTTOTAProgress(&otaProgress);
    // md.onMQTTOTAEnd(&otaEnd);
    // md.onMQTTOTAError(&otaError);
    md.onMQTTConnected(&onMQTT);
    md.setMQTTCallback(on_message);
    md.begin();
    Serial.println(WiFi.localIP());
}

void loop()
{
    md.process();
}

void webServer()
{
    Serial.println("web server");
    md.wfmind.server->on("/api/mqtt", HTTP_GET, handleGetMqtt);
    md.wfmind.server->on("/api/mqtt", HTTP_POST, handleSaveMqtt);
    md.wfmind.server->on("/api/network", HTTP_GET, handleGetNetwork);
    md.wfmind.server->on("/api/network", HTTP_POST, handleSaveNetwork);
    md.wfmind.server->on("/api/backup", HTTP_GET, handleBackup);
    md.wfmind.server->on("/api/restore", HTTP_POST, handleRestored, handleRestore);
}

void handleBackup()
{
    if (!LittleFS.begin())
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
        if (!LittleFS.begin())
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
    md.config["mqtt"]["id"] = doc["id"] | "";
    md.config["mqtt"]["user"] = doc["user"] | "";
    md.config["mqtt"]["pass"] = doc["pass"] | "";
    md.config["mqtt"]["host"] = doc["host"] | "";
    md.config["mqtt"]["port"] = doc["port"] | 1883;
    md.config["mqtt"]["attrf"] = doc["attrf"] | 5;
    md.config["mqtt"]["telef"] = doc["telef"] | 300;
    md.config["mqtt"]["reconnect"] = doc["reconnect"] | 5;
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
        md.reloadMQTT();
    }

}

void handleGetNetwork()
{
    md.wfmind.handleRequest();
    String buf;
    JsonObject network = md.config["network"];
    serializeJson(network, buf);
    md.wfmind.server->send(200, JSONTYPE, buf);
}

void handleSaveNetwork()
{
    md.wfmind.handleRequest();
    String buf = md.wfmind.server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, buf);
    md.config["network"]["dhcp"] = doc["dhcp"] | false;
    md.config["network"]["ip"] = doc["ip"] | "";
    md.config["network"]["mask"] = doc["mask"] | "";
    md.config["network"]["gw"] = doc["gw"] | "";
    md.config["network"]["dns"] = doc["dns"] | "";
    // strlcpy(md.config["network"]["ip"],
    //         doc["ip"] | "192.168.1.4",
    //         sizeof(doc["ip"]));
    // strlcpy(md.config["network"]["mask"],
    //         doc["mask"] | "255.255.255.0",
    //         sizeof(doc["mask"]));
    // strlcpy(md.config["network"]["gw"],
    //         doc["gw"] | "192.168.1.1",
    //         sizeof(doc["gw"]));
    // strlcpy(md.config["network"]["dns"],
    //         doc["dns"] | "8.8.8.8",
    //         sizeof(doc["dns"]));
    if (error || !md.saveConfig())
    {
        md.wfmind.server->send(500, JSONTYPE, HTTP_MESSAGE_500);
    }
    else
    {
        md.wfmind.server->send(200, JSONTYPE, HTTP_MESSAGE_SAVED);
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

void onMQTT(bool connected)
{
    if (connected)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("connected");
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("disconnected");
    }
}

void on_message(char *topic, byte *payload, unsigned int length)
{
    md.mqtt_on_message(topic, payload, length);
}

