#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>
#include "globals.h"

bool isWiFiConfigValid() {
  if (config.ssid[0] < 32 || config.ssid[0] > 126) {
    return false;
  }
  
  int ssidLen = strlen(config.ssid);
  if (ssidLen == 0 || ssidLen > 31) {
    return false;
  }
  
  return true;
}

void loadConfig() {
  EEPROM.begin(512);
  EEPROM.get(0, config);
  
  if (!isWiFiConfigValid()) {
    Serial.println("Invalid WiFi config, resetting...");
    memset(&config, 0, sizeof(config));
  }
}

void saveConfig() {
  EEPROM.put(0, config);
  EEPROM.commit();
}

void resetConfig() {
  memset(&config, 0, sizeof(config));
  saveConfig();
}

#endif // CONFIG_H