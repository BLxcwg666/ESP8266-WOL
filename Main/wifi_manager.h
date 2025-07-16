#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include "globals.h"

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(config.ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.password);
  
  Serial.println("Connecting... (press any key to skip)");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (Serial.available() > 0) {
      Serial.read();
      Serial.println("\nConnection skipped");
      break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi connection failed");
    WiFi.disconnect();
  }
}

void setupAP() {
  Serial.println("Starting AP mode");
  WiFi.mode(WIFI_AP);
  
  WiFi.disconnect();
  delay(100);

  bool result = WiFi.softAP(apSSID, apPassword);
  if (result) {
    Serial.print("AP created successfully! SSID: ");
    Serial.println(apSSID);
    Serial.print("Password: ");
    Serial.println(apPassword);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP creation failed!");
  }
}

void checkWiFiConnection() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    lastCheck = millis();
    
    if (WiFi.status() == WL_CONNECTED) {
      if (!wifiConnected) {
        wifiConnected = true;
        Serial.println("WiFi reconnected successfully");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      }
    } else {
      if (wifiConnected) {
        wifiConnected = false;
        Serial.println("WiFi disconnected");
      }
      
      if (strlen(config.ssid) > 0) {
        connectWiFi();
      }
    }
  }
}

#endif // WIFI_MANAGER_H