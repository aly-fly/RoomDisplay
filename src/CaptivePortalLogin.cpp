#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include <utils.h>
#include "Clock.h"
#include "RLSwifi_https_certificate.h"
#include "display.h"


#define   TestHost1 "connectivitycheck.gstatic.com"
IPAddress TestHost1IP(142,251,208,131);

#define   TestHost2 "clients3.google.com"
IPAddress TestHost2IP(142,251,39,78);

bool CaptivePortalCheckPortalPresence(void) {
    Serial.print("Checking for captive portal..");
    DisplayText("Checking for captive portal..");
    if (WifiState != connected) {
        Serial.println("NO WiFi!");
        DisplayText("NO WiFi!\n", CLRED);
        return false;
    }

    IPAddress ResolvedIP((uint32_t)0);
    if (WiFi.hostByName(TestHost1, ResolvedIP)) {
        if (ResolvedIP == TestHost1IP) {
            Serial.println("OK1");
            DisplayText("OK1\n", CLGREEN);
            return false;
        }
    }
    Serial.print("...");
    DisplayText("...");
    if (WiFi.hostByName(TestHost2, ResolvedIP)) {
        if (ResolvedIP == TestHost2IP) {
            Serial.println("OK2");
            DisplayText("OK2\n", CLGREEN);
            return false;
        }
    }
    Serial.println("Login required!");
    DisplayText("Login required!\n");
    return true;
}

String ResponseFromServer;
int httpCode;

bool HTTPSconnect(String URL) {
  bool result = false;
  Serial.println("Connecting to: " + URL);

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client -> setCACert(rootCACertificate_RlsWifi);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      if (https.begin(*client, URL)) {  // HTTPS
        delay(1); // watchdog reset
        Serial.print("[HTTPS] GET...\r\n");
        // start connection and send HTTP header
        httpCode = https.GET();
        delay(1); // watchdog reset

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
  
          // file found at server
          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {
            ResponseFromServer = https.getString();
            result = true;
            
            Serial.println("--- data begin ---");
            Serial.println(ResponseFromServer);
            Serial.println("--- data data end ---");
            
          }
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
        }
  
        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\r\n");
      }
      // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("Unable to create HTTPS client");
  }
    return result;
}


bool HandleCaptivePortalLogin(void) {
  bool result = false;

  if (CaptivePortalCheckPortalPresence()) {
    Serial.println("Logging onto Captive portal");
    DisplayText("Logging onto Captive portal...");
    
      if (WifiState != connected) {
          return false;
      }

    setClock(); 

    HTTPSconnect(TestHost1);

    // ......
    
  }
  return result;
}



