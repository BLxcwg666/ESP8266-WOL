#ifndef WOL_H
#define WOL_H

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class WakeOnLan {
private:
  WiFiUDP udp;
  
public:
  WakeOnLan() {
    udp.begin(9);
  }
  
  bool parseMacAddress(const String& macStr, uint8_t mac[6]) {
    int values[6];
    int count = 0;
    
    String cleanMac = macStr;
    cleanMac.replace(":", "");
    cleanMac.replace("-", "");
    cleanMac.toUpperCase();
    
    if (cleanMac.length() != 12) {
      return false;
    }
    
    for (int i = 0; i < 6; i++) {
      String byteStr = cleanMac.substring(i * 2, i * 2 + 2);
      char* endPtr;
      values[i] = strtol(byteStr.c_str(), &endPtr, 16);
      
      if (*endPtr != '\0' || values[i] < 0 || values[i] > 255) {
        return false;
      }
      
      mac[i] = (uint8_t)values[i];
    }
    
    return true;
  }
  
  bool sendMagicPacket(const String& macAddress) {
    uint8_t mac[6];
    
    if (!parseMacAddress(macAddress, mac)) {
      Serial.println("Invalid MAC address format");
      return false;
    }
    
    uint8_t magicPacket[102];
    
    for (int i = 0; i < 6; i++) {
      magicPacket[i] = 0xFF;
    }
    
    for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 6; j++) {
        magicPacket[6 + i * 6 + j] = mac[j];
      }
    }
    
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255;
    
    udp.beginPacket(broadcastIP, 9);
    udp.write(magicPacket, 102);
    int result = udp.endPacket();
    
    if (result) {
      Serial.print("WOL magic packet sent to MAC: ");
      Serial.println(macAddress);
      return true;
    } else {
      Serial.println("Failed to send WOL magic packet");
      return false;
    }
  }
  
  bool sendMagicPacketBroadcast(const String& macAddress) {
    uint8_t mac[6];
    
    if (!parseMacAddress(macAddress, mac)) {
      Serial.println("Invalid MAC address format");
      return false;
    }
    
    uint8_t magicPacket[102];
    
    for (int i = 0; i < 6; i++) {
      magicPacket[i] = 0xFF;
    }
    
    for (int i = 0; i < 16; i++) {
      for (int j = 0; j < 6; j++) {
        magicPacket[6 + i * 6 + j] = mac[j];
      }
    }
    
    IPAddress localIP = WiFi.localIP();
    IPAddress subnetMask = WiFi.subnetMask();
    
    IPAddress subnetBroadcast(
      localIP[0] | (~subnetMask[0] & 0xFF),
      localIP[1] | (~subnetMask[1] & 0xFF),
      localIP[2] | (~subnetMask[2] & 0xFF),
      localIP[3] | (~subnetMask[3] & 0xFF)
    );
    
    IPAddress broadcasts[] = {
      IPAddress(255, 255, 255, 255),
      subnetBroadcast,
      IPAddress(192, 168, 1, 255),
      IPAddress(192, 168, 0, 255),
    };
    
    bool success = false;
    for (int i = 0; i < 4; i++) {
      udp.beginPacket(broadcasts[i], 9);
      udp.write(magicPacket, 102);
      if (udp.endPacket()) {
        Serial.print("WOL packet sent to: ");
        Serial.println(broadcasts[i]);
        success = true;
      }
    }
    
    return success;
  }
};

WakeOnLan wol;

#endif // WOL_H