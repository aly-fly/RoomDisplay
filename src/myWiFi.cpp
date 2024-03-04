#include <Arduino.h>
#include <stdint.h>
#include <WiFi.h>
#include "esp_wps.h"
#include "myWiFi.h"
#include "display.h"




WifiState_t WifiState = disconnected;
uint32_t TimeOfWifiReconnectAttempt = 0;


void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info){
  switch(event){
    case ARDUINO_EVENT_WIFI_STA_START:
      WifiState = disconnected;
      Serial.println("Station Mode Started");
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED: // IP not yet assigned
      Serial.println("Connected to AP: " + String(WiFi.SSID()));
      break;     
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("Got IP: ");
      Serial.println(WiFi.localIP());
      WifiState = connected;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      WifiState = disconnected;
      Serial.print("WiFi lost connection. Reason: ");
      Serial.println(info.wifi_sta_disconnected.reason);
      break;
    default:
      break;
  }
}



void WifiInit(void)  {
  WifiState = disconnected;
  DisplayText("WiFi start");
  Serial.print("WiFi start");

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);  
  WiFi.setHostname(DEVICE_NAME);  

  WiFi.begin(WIFI_SSID, WIFI_PASSWD); 
  WiFi.onEvent(WiFiEvent);
  unsigned long StartTime = millis();
  while ((WiFi.status() != WL_CONNECTED)) {
    delay(500);
    DisplayText(".");
    Serial.print(".");
    if ((millis() - StartTime) > (WIFI_CONNECT_TIMEOUT_SEC * 1000)) {
      Serial.println("\r\nWiFi connection timeout!");
      DisplayText("\nTIMEOUT!", CLRED);
      WifiState = disconnected;
      return; // exit loop, exit procedure, continue startup
    }
  }
  
  WifiState = connected;

  DisplayText("\n Connected to: ", CLBLUE);
  DisplayText(WiFi.SSID().c_str(), CLCYAN);
  DisplayText("\n IP: ", CLORANGE);
  DisplayText(WiFi.localIP().toString().c_str(), CLYELLOW);
  DisplayText("\n");
  
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
  delay(200);
}

void WifiReconnectIfNeeded(void) {
  if ((WifiState == disconnected) && ((millis() - TimeOfWifiReconnectAttempt) > WIFI_RETRY_CONNECTION_SEC * 1000)) {
    Serial.println("Attempting WiFi reconnection...");
    DisplayClear();
    DisplayText("WiFi reconnect...", CLCYAN);
    WiFi.reconnect();
    TimeOfWifiReconnectAttempt = millis();
  }    
}


