/*
   wifi connection configuration Library
   This library provided the simple utility class for Wifi connection configuration.
   Written by Dr.Paween Khoenkaw
   Copyright (C) 2020 All right reserved.
*/

#include "ApClientConfig.h"
#include <WebServer.h>

#include "HtmlPage.h"


char* ssid;
char* password;
char* ui_user;
char* ui_password;
char* ssid_name;
char* ssid_password;
// IP Address details
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 0, 0);
WebServer server(80);  // Object of WebServer(HTTP port, 80 is defult)
char Buffer[4048];
bool isAuth = false;
bool isRunning = true;

void handle_root() {
  server.send(200, "text/html", Index);
}

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (0):
      return "Open";
    case (1):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (3):
      return "WPA2_PSK";
    case (4):
      return "WPA_WPA2_PSK";
    case (5):
      return "WPA2_ENTERPRISE";
    default:
      return "UNKOWN";
  }
}
void scan_ssid() {
  if (isAuth == false)
  {
    handle_root();
    return;
  }
  char temp[4000];
  //strcpy(Buffer,"<html><body> Please select SSID for internet connection:<p> <form action=\"/set_ssid\" method=\"post\">");
  strcpy(Buffer, "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style></head><body style=background-color:#afdbd2> Please select SSID for internet connection:<p> <form action=\"/set_ssid\" method=\"post\">");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      if (i > 15)continue;
      sprintf(temp, "<br><input type=\"radio\" id=\"male\" name=\"ssid_name\" value=\"%s\"> %s (%d dB)      ", WiFi.SSID(i).c_str(), WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ");
      Serial.print(" [");
      Serial.print(WiFi.channel(i));

      strcat(Buffer, temp);
      if (WiFi.encryptionType(i) > 0)strcat(Buffer, "<span style=\"color:green\"> &#9919;</span>");
      if (WiFi.RSSI(i) > -50)strcat(Buffer, "<span style=\"color:green\"> (Good signal)</span> ");
      if ((WiFi.RSSI(i) < -50) && (WiFi.RSSI(i) > -66)  )strcat(Buffer, "<span style=\"color:yellow\"> (Fair signal)</span> ");
      if (WiFi.RSSI(i) <= -66)strcat(Buffer, "<span style=\"color:red\"> (Weak signal)</span> ");
      Serial.print("] ");
      String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
      Serial.println(encryptionTypeDescription);
    }
    strcat(Buffer, " <br>Wifi password (optional)&#9919;:<input type=\"text\" name=\"password\"><br><input type=\"submit\" value=\"Save and reboot\" /></form></body></html>");
    server.send(200, "text/html", Buffer);
  }
}

void handle_login() {
  String user = server.arg("user");
  String pass = server.arg("password");
  if (strcmp(user.c_str(), ui_user) || strcmp(pass.c_str(), ui_password)) {
    // login fail
    Serial.println("login fail");
    server.send(200, "text/html", Login_fail);
  } else {
    // login success
    isAuth = true;
    Serial.println("login success");
    scan_ssid();
  }
}

void handle_set_ssid() {
  if (isAuth == false)
  {
    handle_root();
    return;
  }
  String _ssid_name = server.arg("ssid_name");
  String _ssid_password = server.arg("password");
  strcpy(ssid_name, _ssid_name.c_str());
  strcpy(ssid_password, _ssid_password.c_str());
  server.send(200, "text/html", "Config is saved");
  isRunning = false;
}


void setupRoutes() {
  server.on("/", handle_root);
  server.on("/login", HTTP_POST, handle_login);
  server.on("/set_ssid", HTTP_POST, handle_set_ssid);
}
ApClientConfig:: ApClientConfig(char* Apssid, char* Appassword, char* UI_user, char* UI_password, char* ClientSSID, char* ClientPassword)
{
  ssid = Apssid;
  password = Appassword;
  ui_user = UI_user;
  ui_password = UI_password;
  ssid_name = ClientSSID;
  ssid_password = ClientPassword;




}
void ApClientConfig::setDoneCallback(void(*callback)())
{
  Done = callback;

}
void ApClientConfig::Begin() {
  Serial.begin(115200);
  // Create SoftAP
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.print("Connect to My access point: ");
  Serial.println(ssid);
  setupRoutes();
  server.begin();
  Serial.println("HTTP server started");
  delay(100);
  while (isRunning)server.handleClient();
  Done();
}
