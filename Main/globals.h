#ifndef GLOBALS_H
#define GLOBALS_H

#include <ESP8266WebServer.h>
#include <WebSocketsClient.h>

struct Config {
  char ssid[32];
  char password[64];
  char wsUrl[256];
  char wsToken[64];
};

extern Config config;
extern ESP8266WebServer server;
extern WebSocketsClient webSocket;
extern bool wifiConnected;
extern bool wsConnected;
extern unsigned long wsLastConnectedTime;

extern const char* apSSID;
extern const char* apPassword;

#endif // GLOBALS_H