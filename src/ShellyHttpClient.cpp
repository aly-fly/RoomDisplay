#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <utils.h>
#include "__CONFIG.h"
#include "myWiFi.h"

String sTotalPower;
float TotalPower;

bool ShellyGetData(void) {
    bool result = false;
    sTotalPower = "n/a";
    TotalPower = 0;
    if (WifiState != connected) {
        return false;
    }
        HTTPClient http;

        Serial.println("Shelly connect...");
        String URL = "http://" + (String)SHELLY_3EM_HOST + (String)"/status";  // DOC: https://shelly-api-docs.shelly.cloud/gen1/#shelly-3em-settings-emeter-index
        if (http.begin(URL)) {
            Serial.println("[HTTP] GET...");
            // start connection and send HTTP header
            int httpCode = http.GET();
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                String JsonData = http.getString();
                // Serial.println(JsonData);
                // do nout use JSON parser, as we are only interested in one segment ("total_power":74.62,)
                int idx = JsonData.indexOf("total_power");
                if (idx > 0) {
                    unsigned int idxBegin = ((unsigned int) idx) + 13;
                    unsigned int idxEnd = JsonData.indexOf(",", idxBegin);
                    sTotalPower = JsonData.substring(idxBegin, idxEnd);
                    TrimNumDot(sTotalPower);
                    TotalPower = sTotalPower.toFloat();
                    Serial.print("Data: ");
                    Serial.print(sTotalPower);
                    Serial.print(" = ");
                    Serial.print(TotalPower);
                    Serial.println();
                    result = true;
                    Serial.println("[SHELLY] Free memory: " + String(esp_get_free_heap_size()) + " bytes");
                }
               JsonData = "";  // free memory
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
        } else {
        Serial.println("Connect failed!");
        }
        http.end();
        return result;
    }
