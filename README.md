# ESP User Connection
Basic code to ask a user about ESP WiFi connection in an easy to handle way.
Thanks to [Ígor Polegato](https://github.com/igorpolegato).

You can provide to your project a user WiFi connection configuration way just by:
```
#include "ESPUserConnection.h"

void setup() {
	Serial.begin(115200);
	connect_wifi();
}

void loop () {
}
```
**PS: erasing all the flash memory is advisable**

## How it works

The process follow this steps:
- First of all, scan for WiFi networks on the neabor (01. firts_scan_wifi.png);
- Then read a list of known WiFi and try to connect the matches (02. no_wifi_match.png);
- If no connection is possible and/or no match WiFi name, enable AP Mode (03. enable_ap_mode.png);
- So the user can connect to AP and access the WiFi configuration page, but you need on first access (04. first_access.png) upload the files "user_config.html" (05. send_user_config_html.png) and "index.html" (06. send_index_html.png), just one time;
- Now the user can see the WiFi list on navigator (07. acess_wifi_list.png);
- And can click over one specific network name to connect to and enter the password (08. select_wifi_and_type_password_restart.png);
- User can also type a WiFi name, for example, if the ESP will be connect to smartphone hotspot, then first of all user must to connect to AP, so type the smartphone WiFi hotspot name and password;
- Restarting, the WiFi configured will be found, get match (09. wifi_match.png);
- Then the ESP will connect to configured WiFi (10. wifi_connected.png);
- Now the user can access ESP WebServer via new WLAN IP and edit/delete known WiFi (11. access_via_new_ip.png).

Users can also edit files and upload to ESP, via "/send_file" endpoint. So it's possible, for example, to edit "known_wifis.txt" with yours WiFis, in this case following template per line "name\tpasswd\n", as well as HTML files.

![01. firts_scan_wifi.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/01.%20firts_scan_wifi.png)
![02. no_wifi_match.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/02.%20no_wifi_match.png)
![03. enable_ap_mode.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/03.%20enable_ap_mode.png)
![04. first_access.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/04.%20first_access.png)
![05. send_user_config_html.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/05.%20send_user_config_html.png)
![06. send_index_html.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/06.%20send_index_html.png)
![07. acess_wifi_list.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/07.%20acess_wifi_list.png)
![08. select_wifi_and_type_password_restart.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/08.%20select_wifi_and_type_password_restart.png)
![09. wifi_match.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/09.%20wifi_match.png)
![10. wifi_connected.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/10.%20wifi_connected.png)
![11. access_via_new_ip.png](https://raw.githubusercontent.com/JuniorPolegato/ESPUserConnection/main/assets/11.%20access_via_new_ip.png)

## Customizations
I share this code template for you aggregate with or start your project to provide a easy way to user connect the ESP to a WLAN.

This code configure a WebServer, using [ESPAsyncWebServer](https://github.com/JuniorPolegato/ESPAsyncWebServer), so you can add some html/js/css and images to you project, customizing the index.html and/or creating new endpoints.

The endpoints "/wifi", "/scan", "/add_wifi_network" and "/send_file" are used by ESP User Connection.

You can also easy request more data to the user without edit original code by uncommenting the line "`#define CUSTOM_USER_REQUEST_DATA`" in "ESPUserConnection.h", so you can edit custom "sendfiles_html" and "go_back_html" also in "ESPUserConnection.h", plus a copy of "user_config.html" with name "custom_user_config.html" and also edit "custom_user_request_data" in "ESPUserConnection.cpp", there is here a example to request a field (key) to user with 32 characters.

The output can be changed from Serial to TFT, by uncommenting in "ESPUserConnection.h" the lines:
```
#define OUTPUT_IS_TFT
#define TFT_BACKGROUND TFT_DARKGREEN
#define TFT_TEXT_COLOR TFT_YELLOW
```

For a practical use of this code, customizations and TFT, you can see [TTGOWeatherStation](https://github.com/JuniorPolegato/TTGOWeatherStation)


## Simplified ESP file system operations
There is also a group of functions in "fs_operations.h" to access files easier:
```
bool writeFile(String path, String message, const bool overwrite=false);
String readFile(String path);
bool findFile(String path);
bool deleteFile(String path);
bool renameFile(String from, String to);
```
