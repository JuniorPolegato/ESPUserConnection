#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  // https://github.com/JuniorPolegato/ESPAsyncWebServer

#include "ESPUserConnection.h"
#include "fs_operations.h"

AsyncWebServer webserver(80);
bool webserver_configured = false;
void user_request_data(AsyncWebServerRequest *request, bool restart=true);

// This is an example to request also a key to the user via html page
#ifdef CUSTOM_USER_REQUEST_DATA
void custom_user_request_data(AsyncWebServerRequest *request) {
    int params = request->params();

    if (params >= 3) {
        String key = request->getParam(2)->value();

        if (key.length() == 32)
            writeFile("/key.txt", key, true);

        else if (key == "delete")
            deleteFile("/key.txt");

        user_request_data(request);
    }
    else
        request->send(500);
}
#endif  // CUSTOM_USER_REQUEST_DATA

void user_request_data(AsyncWebServerRequest *request, bool restart) {
    int params = request->params();

    if (params >= 2) {
        String ssid = request->getParam(0)->value();
        String passwd = request->getParam(1)->value();
        int i = -1, n;

        if (passwd.length() > 0) {
            String file_data = readFile("/known_wifis.txt");
            renameFile("/known_wifis.txt", "/known_wifis.txt.bak");

            if (file_data.startsWith(ssid + '\t') || (i = file_data.indexOf('\n' + ssid + '\t')) > 0) {
                n = file_data.indexOf('\n', ++i);
                file_data = (passwd == "delete" ? String() : ssid + '\t' + passwd + '\n') +
                            file_data.substring(0, i) +
                            (n == -1 ? String() : file_data.substring(n + 1));
            }
            else if (passwd != "delete")
                file_data = ssid + '\t' + passwd + '\n' + file_data;

            if (writeFile("/known_wifis.txt", file_data, true))
                deleteFile("/known_wifis.txt.bak");
            else
                renameFile("/known_wifis.txt.bak", "/known_wifis.txt");
        }

        request->send(200, "text/html", go_back_html);

        if (restart) ESP.restart();
    }
    else
        request->send(500);
}

// Settings for your TFT device and/or Serial
#ifdef OUTPUT_IS_TFT
    #include <TFT_eSPI.h>
    TFT_eSPI *output;
#else
    #define output (&Serial)
    // Configure how to clear screen on your serial terminal
    // This line clear screen for https://github.com/JuniorPolegato/gtkterm
    #define SERIAL_CLEAR_SCREEN_COMMAND Serial.print('\x1b')
#endif  // OUTPUT_IS_TFT


void clear(){
#ifdef OUTPUT_IS_TFT
    output->fillRect(0, 0, output->getViewportWidth(), output->getViewportHeight(), TFT_BACKGROUND);
    output->setTextColor(TFT_TEXT_COLOR);
    output->setCursor(0, 0);
#else
    SERIAL_CLEAR_SCREEN_COMMAND;
#endif  // OUTPUT_IS_TFT
    output->println(PROJECT_NAME);
    output->println();
}

String scan(){
    String resp = "[";
    String ssid;
    int scan_status = WiFi.scanComplete();

    clear();
    output->println("Searching Wi-Fi...\n");

    if (scan_status == WIFI_SCAN_FAILED) {
        output->println("Starting...\n");
        WiFi.scanNetworks(true);
        delay(1000);
    }

    while ((scan_status = WiFi.scanComplete()) == WIFI_SCAN_RUNNING) {
        output->print('.'); output->flush();
        delay(100);
    }
    output->println();

    if (scan_status == WIFI_SCAN_FAILED) {
        output->println("Wait a moment...");
        return "[{\"name\":\"Trying again...\","
                 "\"quality\":\"Wait a moment...\","
                 "\"security\":\"Wait a moment...\"}]";
    }

    output->println("Networks found: " + String(scan_status));
    for (int  i = 0; i < scan_status; i++) {
        resp += String(i ? ",{" : "{") +
                "\"name\":\"" + WiFi.SSID(i) + "\","
                "\"quality\":" + WiFi.RSSI(i) + ","
                "\"security\":" + WiFi.encryptionType(i) + "}";
        if (i < 20) output->println(WiFi.SSID(i) + '[' + WiFi.RSSI(i) + ']');
    }

    WiFi.scanDelete();

    return resp + ']';
}

void upload_handler(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    output->println(filename + '[' + len + ']');

    if (!index) {
        String filepath = filename;

        if (!filename.startsWith("/"))
            filepath = "/" + filename;

        request->_tempFile = LittleFS.open(filepath, FILE_WRITE);
    }

    if (len)
        request->_tempFile.write(data, len);

    if (final) {
        request->_tempFile.close();
        request->send(200, "text/html", go_back_html);
    }
}

void start_AP() {
    clear();
    output->println("Connect your\n"
                    "Wi-Fi device to:\n\n"
                    + String(PROJECT_NAME) + "\n\n"
                    "Password: 12345678\n");

    const char *ssid = PROJECT_NAME;
    const char *passwd = "12345678";
    WiFi.softAP(ssid, passwd);
    IPAddress ip = WiFi.softAPIP();
    delay(1000);

    webserver.begin();
    output->println("Then access:\n\n"
                  "http://" + ip.toString() + "/wifi\n\n"
                  "and select Wi-Fi\n"
                  "network for your\n"
                  + String(PROJECT_NAME) + "\n\n"
                  "Waiting for you...\n");
    delay(1000);
}

void config_webserver() {
    webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (findFile("/index.html"))
            request->send(LittleFS, "/index.html", "text/html");
        else {
            output->println("Send \"index.html\" file!");
            request->send(200, "text/html", sendfiles_html);
        }
    });

    webserver.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef CUSTOM_USER_REQUEST_DATA
        String user_config = "/custom_user_config.html";
#else
        String user_config = "/user_config.html";
#endif  // CUSTOM_USER_REQUEST_DATA
        if (findFile(user_config)) {
            output->println("Now select a Wi-Fi\n"
                            "network for ESP\n"
                            "connect to\n");
            request->send(LittleFS, user_config, "text/html");
        }
        else {
            output->println("First access.\n"
                            "It's needed to\n"
                            "send \"" + user_config.substring(1) + "\".\n");
            request->send(200, "text/html", sendfiles_html);
        }
    });

    webserver.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
        WiFi.scanNetworks(true);
        request->send(200, "application/json", scan());
    });

    webserver.on("/add_wifi_network", HTTP_POST, [](AsyncWebServerRequest *request) {
#ifdef CUSTOM_USER_REQUEST_DATA
        custom_user_request_data(request);
#else
        user_request_data(request);
#endif  // CUSTOM_USER_REQUEST_DATA
    });

    webserver.on("/send_file", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", sendfiles_html);
        output->println("Ready to send a file!");
    });

    webserver.on("/send_file", HTTP_POST, [](AsyncWebServerRequest *request) {
        clear();
        output->println("Uploading the file...");
        request->send(200);
    }, upload_handler);

    // Here you can add your custom endpoints

    webserver_configured = true;
}

#ifdef OUTPUT_IS_TFT
bool connect_wifi(void *tft, bool force_ap_mode){
    output = (TFT_eSPI*)tft;
#else
bool connect_wifi(bool force_ap_mode){
#endif  // OUTPUT_IS_TFT

    String list, wifi, passwd, known_wifis;
    int i = 0, d;  // i = inital line position | d = position of delimiter \t

    WiFi.scanNetworks(true);

    if (webserver_configured)
        webserver.end();
    else
        config_webserver();

    if (force_ap_mode) {
        output->println("Forcing AP Mode!\n");
        WiFi.disconnect(false, true);
        delay(1000);
        start_AP();
        return false;
    }

    while ((list = scan()).indexOf("Trying again...") > 0)
        delay(1000);

    delay(1000);
    known_wifis = readFile("/known_wifis.txt");
    while (WiFi.status() != WL_CONNECTED) {
        clear();
        output->println("Matching known WiFi:\n");
        for (;;) {
            d = known_wifis.indexOf('\t', i);
            if (d == -1)
                break;

            wifi = known_wifis.substring(i, d++);

            if (list.indexOf('"' + wifi + '"') != -1) {  // WiFi name match
                output->println("\n* " + wifi + " *\n");
                break;
            }
            output->println(wifi + "\n  \\--> out of range");

            i = known_wifis.indexOf('\n', d);
            if (i == -1)
                break;
            i++;
        }

        if (i == -1 || d == -1) {  // No known wi-fi found
            output->println("\n\nNo known WiFi\n\nmatches in the\n\nneighborhood.\n");
            WiFi.disconnect(false, true);
            delay(1000);
            start_AP();
            return false;
        }

        i = known_wifis.indexOf('\n', d);
        passwd = known_wifis.substring(d, i == -1 ? known_wifis.length() : i++);

        delay(1000);
        clear();
        output->println("* " + wifi + " *\n");
        // output->println("-*-|" + passwd + "|-*-\n");
        output->print("Connecting...");
        WiFi.begin(wifi, passwd == "open" ? (const char *)NULL : passwd.c_str());

        d = 0;
        while (WiFi.status() != WL_CONNECTED && d++ < 30) {
            delay(500);
            output->print("."); output->flush();
        }
    }

    output->println("\n\nWiFi connected!");
    output->println("\nIP address:");
    output->println(WiFi.localIP());

    webserver.begin();

    delay(1000);
    return true;
}
