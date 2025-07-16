#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#include "globals.h"
#include "config.h"
#include "webpages.h"
#include "wifi_manager.h"
#include "websocket_manager.h"
#include "url_parser.h"
#include "wol.h"

Config config;
ESP8266WebServer server(80);
WebSocketsClient webSocket;
bool wifiConnected = false;
bool wsConnected = false;
unsigned long wsLastConnectedTime = 0;

const char* apSSID = "ESP8266-Wol";
const char* apPassword = "12345678";

void setupWebServer();
void handleRoot();
void handleWiFiConfig();
void handleWiFiScan();
void handleWiFiConnect();
void handleWSConfigPage();
void handleWSConfigGet();
void handleWSConfigSave();
void handleWSTest();
void handleStatus();
void handleReset();

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=== Device Starting ===");
  
  Serial.println("Press any key to enter configuration mode...");
  unsigned long startTime = millis();
  bool forceAPMode = false;
  
  while (millis() - startTime < 3000) {
    if (Serial.available() > 0) {
      Serial.read();
      forceAPMode = true;
      Serial.println("Enter configuration mode!");
      break;
    }
    delay(100);
  }
  
  Serial.println("Loading configuration...");
  loadConfig();
  
  Serial.print("Current SSID: ");
  Serial.println(config.ssid);
  Serial.print("SSID length: ");
  Serial.println(strlen(config.ssid));
  
  if (forceAPMode) {
    Serial.println("Force AP mode...");
    setupAP();
  } else {
    if (strlen(config.ssid) > 0) {
      Serial.println("Found saved WiFi config, trying to connect...");
      connectWiFi();
    } else {
      Serial.println("No saved WiFi config");
    }
    
    if (!wifiConnected) {
      Serial.println("WiFi not connected, starting AP mode...");
      setupAP();
    }
  }
  
  setupWebServer();
  
  if (wifiConnected && strlen(config.wsUrl) > 0) {
    Serial.println("WiFi connected, trying to connect WebSocket...");
    connectWebSocket();
  } else if (wifiConnected) {
    Serial.println("WiFi connected, but WebSocket not configured");
  }
}

void loop() {
  server.handleClient();
  
  webSocket.loop();
  
  static bool wsInitialized = false;
  static unsigned long lastInitAttempt = 0;
  static int reconnectAttempts = 0;
  
  if (!wsConnected && wifiConnected && strlen(config.wsUrl) > 0) {
    unsigned long timeSinceLastAttempt = millis() - lastInitAttempt;
    
    unsigned long reconnectDelay = min(5000UL * (1UL << reconnectAttempts), 60000UL);
    
    if (!wsInitialized || timeSinceLastAttempt > reconnectDelay) {
      lastInitAttempt = millis();
      wsInitialized = true;
      reconnectAttempts++;
      
      Serial.print("Initializing WebSocket connection (attempt #");
      Serial.print(reconnectAttempts);
      Serial.print(", next delay: ");
      Serial.print(reconnectDelay / 1000);
      Serial.println(" seconds)...");
      
      connectWebSocket();
    }
  } else if (wsConnected) {
    if (reconnectAttempts > 0) {
      Serial.println("WebSocket reconnected successfully!");
      reconnectAttempts = 0;
    }
    wsInitialized = true;
  } else if (!wifiConnected && wsInitialized) {
    wsInitialized = false;
    reconnectAttempts = 0;
  }
  
  checkWiFiConnection();
  
  static unsigned long lastStatusUpdate = 0;
  if (wsConnected && millis() - lastStatusUpdate > 30000) {
    lastStatusUpdate = millis();
    sendStatusUpdate();
  }
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/reset", handleReset);
  
  server.on("/wifi/config", handleWiFiConfig);
  server.on("/wifi/connect", HTTP_POST, handleWiFiConnect);
  server.on("/scan", handleWiFiScan);
  
  server.on("/ws/config", HTTP_GET, handleWSConfigPage);
  server.on("/ws/config", HTTP_POST, handleWSConfigSave);
  server.on("/ws/current", HTTP_GET, handleWSConfigGet);
  server.on("/ws/test", HTTP_POST, handleWSTest);
  
  server.begin();
  Serial.println("Web server started");
}

void handleRoot() {
  if (wifiConnected) {
    server.send(200, "text/html", statusPage);
  } else {
    server.send(200, "text/html", wifiConfigPage);
  }
}

void handleWiFiConfig() {
  server.send(200, "text/html", wifiConfigPage);
}

void handleWiFiScan() {
  Serial.println("Scanning WiFi...");
  int n = WiFi.scanNetworks();
  
  DynamicJsonDocument doc(2048);
  JsonArray networks = doc.to<JsonArray>();
  
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["secure"] = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWiFiConnect() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      strlcpy(config.ssid, doc["ssid"], sizeof(config.ssid));
      strlcpy(config.password, doc["password"], sizeof(config.password));
      saveConfig();
      
      WiFi.begin(config.ssid, config.password);
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        String ip = WiFi.localIP().toString();
        String response = "{\"success\":true,\"redirect\":\"http://" + ip + "/\"}";
        server.send(200, "application/json", response);
        
        delay(1000);
        WiFi.mode(WIFI_STA);
      } else {
        WiFi.disconnect();
        server.send(200, "application/json", "{\"success\":false,\"message\":\"Connection failed, please check password\"}");
      }
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"JSON parse error\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing data\"}");
  }
}

void handleWSConfigPage() {
  if (wifiConnected) {
    server.send(200, "text/html", wsConfigPage);
  } else {
    server.send(403, "text/plain", "Please connect WiFi first");
  }
}

void handleWSConfigGet() {
  DynamicJsonDocument doc(256);
  doc["wsUrl"] = config.wsUrl;
  doc["wsToken"] = config.wsToken;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleWSConfigSave() {
  if (server.hasArg("plain")) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      strlcpy(config.wsUrl, doc["wsUrl"], sizeof(config.wsUrl));
      strlcpy(config.wsToken, doc["wsToken"], sizeof(config.wsToken));
      saveConfig();
      
      server.send(200, "application/json", "{\"success\":true}");
      
      if (wsConnected) {
        webSocket.disconnect();
        delay(100);
        connectWebSocket();
      }
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"JSON parse error\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing data\"}");
  }
}

void handleWSTest() {
  if (strlen(config.wsUrl) == 0) {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"WebSocket server not configured\"}");
    return;
  }
  
  if (wsConnected) {
    webSocket.disconnect();
    delay(100);
  }
  
  connectWebSocket();
  
  int attempts = 0;
  while (!wsConnected && attempts < 50) {
    webSocket.loop();
    delay(100);
    attempts++;
  }
  
  if (wsConnected) {
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Connection failed\"}");
  }
}

void handleStatus() {
  DynamicJsonDocument doc(512);
  doc["wifiConnected"] = wifiConnected;
  doc["ssid"] = wifiConnected ? WiFi.SSID() : "";
  doc["ip"] = wifiConnected ? WiFi.localIP().toString() : "";
  doc["rssi"] = wifiConnected ? WiFi.RSSI() : 0;
  doc["wsConnected"] = wsConnected;
  doc["wsUrl"] = config.wsUrl;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleReset() {
  resetConfig();
  server.send(200, "text/html", "<html><body><h1>Device reset</h1><p>Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}
