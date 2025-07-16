#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "globals.h"
#include "url_parser.h"
#include "wol.h"

void handleWebSocketMessage(uint8_t * payload, size_t length);

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      wsConnected = false;
      Serial.println("WebSocket disconnected");
      Serial.print("Connection duration: ");
      Serial.print((millis() - wsLastConnectedTime) / 1000);
      Serial.println(" seconds");
      break;
      
    case WStype_CONNECTED: {
      wsConnected = true;
      wsLastConnectedTime = millis();
      Serial.println("WebSocket connected");
      Serial.print("Server address: ");
      Serial.println((char*)payload);
      
      DynamicJsonDocument doc(128);
      doc["type"] = "auth";
      doc["token"] = config.wsToken;
      String authMsg;
      serializeJson(doc, authMsg);
      webSocket.sendTXT(authMsg);
      break;
    }
      
    case WStype_TEXT:
      Serial.printf("Received message: %s\n", payload);
      handleWebSocketMessage(payload, length);
      break;
      
    case WStype_BIN:
      Serial.println("Received binary data");
      break;
      
    case WStype_ERROR:
      Serial.print("WebSocket error: ");
      if (length > 0) {
        Serial.write(payload, length);
        Serial.println();
      }
      break;
      
    case WStype_PING:
      Serial.println("Received Ping");
      break;
      
    case WStype_PONG:
      Serial.println("Received Pong");
      break;
      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      Serial.println("Received fragment message");
      break;
  }
}

void handleWebSocketMessage(uint8_t * payload, size_t length) {
  String message = String((char*)payload);
  Serial.print("Received message: ");
  Serial.println(message);
  
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* type = doc["type"];
  if (!type) {
    Serial.println("Message has no type field");
    return;
  }
  
  if (strcmp(type, "ping") == 0) {
    StaticJsonDocument<128> response;
    response["type"] = "pong";
    response["timestamp"] = millis();
    
    String responseStr;
    serializeJson(response, responseStr);
    webSocket.sendTXT(responseStr);
  } 
  else if (strcmp(type, "wol") == 0) {
    const char* mac = doc["mac"];
    if (!mac) {
      Serial.println("WOL command missing MAC address");
      return;
    }
    
    Serial.print("Sending WOL to MAC: ");
    Serial.println(mac);
    
    bool success = wol.sendMagicPacketBroadcast(String(mac));
    
    StaticJsonDocument<256> response;
    response["type"] = "wol_result";
    response["mac"] = mac;
    response["success"] = success;
    response["timestamp"] = millis();
    
    String responseStr;
    serializeJson(response, responseStr);
    webSocket.sendTXT(responseStr);
  }
  else {
    Serial.print("Unknown message type: ");
    Serial.println(type);
  }
}

void connectWebSocket() {
  if (strlen(config.wsUrl) == 0) {
    Serial.println("WebSocket URL not configured");
    return;
  }
  
  ParsedUrl url = parseWebSocketUrl(config.wsUrl);
  if (!url.valid) {
    Serial.println("WebSocket URL invalid");
    return;
  }
  
  Serial.print("Connecting to WebSocket server: ");
  Serial.println(config.wsUrl);
  Serial.print("Host: ");
  Serial.print(url.host);
  Serial.print(", Port: ");
  Serial.print(url.port);
  Serial.print(", Path: ");
  Serial.println(url.path);
  
  webSocket.disconnect();
  
  webSocket.begin(url.host, url.port, url.path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void sendStatusUpdate() {
  if (wsConnected) {
    DynamicJsonDocument doc(256);
    doc["type"] = "status";
    doc["status"]["wifiConnected"] = wifiConnected;
    doc["status"]["ip"] = WiFi.localIP().toString();
    doc["status"]["rssi"] = WiFi.RSSI();
    doc["status"]["freeHeap"] = ESP.getFreeHeap();
    
    String statusMsg;
    serializeJson(doc, statusMsg);
    webSocket.sendTXT(statusMsg);
  }
}

#endif // WEBSOCKET_MANAGER_H