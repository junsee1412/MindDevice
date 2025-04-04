# 1 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
# 2 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 2




MindDevice md;

void setup()
{
    Serial.begin(9600);
    pinMode(2, 0x01);
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
        Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 46 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                      (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "46" "." "136" "\", \"aSM\", @progbits, 1 #"))) = (
# 46 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                      "LittleFS mount failed"
# 46 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                      ); &__pstr__[0];}))
# 46 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                      ))));
        return;
    }
    // Open file for reading
    File download = LittleFS.open("/mindgateway.json", "r");
    if (download)
    {
        md.wfmind.server->sendHeader(((reinterpret_cast<const __FlashStringHelper *>(
# 53 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "53" "." "137" "\", \"aSM\", @progbits, 1 #"))) = (
# 53 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                                    "Content-Type"
# 53 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                                    ); &__pstr__[0];}))
# 53 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                                    ))), "application/json");
        md.wfmind.server->sendHeader(((reinterpret_cast<const __FlashStringHelper *>(
# 54 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                                    (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "54" "." "138" "\", \"aSM\", @progbits, 1 #"))) = (
# 54 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                                    "Content-Disposition"
# 54 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                                    ); &__pstr__[0];}))
# 54 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                                    ))), "attachment; filename=" + String("mindgateway.json"));
        md.wfmind.server->streamFile(download, "application/octet-stream");
        download.close();
    }
    else
    {
        md.wfmind.server->send(500, "application/json", "{\"message\":\"Internal Server Error\"}");
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
            Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 71 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "71" "." "139" "\", \"aSM\", @progbits, 1 #"))) = (
# 71 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          "LittleFS mount failed"
# 71 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          ); &__pstr__[0];}))
# 71 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          ))));
            return;
        }
        // Delete existing file, otherwise the configuration is appended to the file
        if (LittleFS.remove("/mindgateway.json"))
        {
            Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 77 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "77" "." "140" "\", \"aSM\", @progbits, 1 #"))) = (
# 77 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          "File deleted"
# 77 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          ); &__pstr__[0];}))
# 77 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          ))));
        }
        else
        {
            Serial.println(((reinterpret_cast<const __FlashStringHelper *>(
# 81 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          (__extension__({static const char __pstr__[] __attribute__((__aligned__(4))) __attribute__((section( "\".irom0.pstr." "nah.ino" "." "81" "." "141" "\", \"aSM\", @progbits, 1 #"))) = (
# 81 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          "Delete failed"
# 81 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino" 3
                          ); &__pstr__[0];}))
# 81 "/home/junsee/Arduino/libraries/MindDevice/examples/nah/nah.ino"
                          ))));
        }
        fsUploadFile = LittleFS.open("/mindgateway.json", "w");
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
            md.wfmind.server->send(500, "application/json", "{\"message\":\"Internal Server Error\"}");
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
    }
}

void handleRestored()
{
    md.wfmind.server->send(200, "application/json", "{\"message\":\"Saved\"}");
}

void handleGetMqtt()
{
    md.wfmind.handleRequest();
    String buf;
    JsonObject network = md.config["mqtt"];
    serializeJson(network, buf);
    md.wfmind.server->send(200, "application/json", buf);
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
        md.wfmind.server->send(500, "application/json", "{\"message\":\"Internal Server Error\"}");
    }
    else
    {
        md.wfmind.server->send(200, "application/json", "{\"message\":\"Saved\"}");
        md.reloadMQTT();
    }

}

void handleGetNetwork()
{
    md.wfmind.handleRequest();
    String buf;
    JsonObject network = md.config["network"];
    serializeJson(network, buf);
    md.wfmind.server->send(200, "application/json", buf);
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
        md.wfmind.server->send(500, "application/json", "{\"message\":\"Internal Server Error\"}");
    }
    else
    {
        md.wfmind.server->send(200, "application/json", "{\"message\":\"Saved\"}");
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
        digitalWrite(2, 0x1);
        Serial.println("connected");
    }
    else
    {
        digitalWrite(2, 0x0);
        Serial.println("disconnected");
    }
}

void on_message(char *topic, byte *payload, unsigned int length)
{
    md.mqtt_on_message(topic, payload, length);
}
