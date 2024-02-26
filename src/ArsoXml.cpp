// ARSO xml: https://meteo.arso.gov.si/met/sl/service/
// osrednja 3 dni: https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SLOVENIA_MIDDLE_latest.xml
// osrednja čez dan: https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SI_OSREDNJESLOVENSKA_latest.xml

#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include "utils.h"
#include "ArsoXml.h"
#include "display.h"

unsigned long LastTimeArsoRefreshed = 0;


// reference: "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\BasicHttpsClient\BasicHttpsClient.ino"


// root certificate of https://meteo.arso.gov.si/
// download:  25.2.2024
// valid by: 6/4/35, 1:04:38 PM GMT+2

const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\r\n" \
"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\r\n" \
"-----END CERTIFICATE-----\r\n";

// WiFiClientSecure checks the validity date of the certificate. 
// Setting clock for CA authorization...
void setClock() {
//  configTime(GMT_OFFSET*60*60, DST_OFFSET*60*60, "pool.ntp.org");
  configTime(0, 0, "pool.ntp.org");

  Serial.print("NTP time sync");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}




String XMLdata;

bool GetXmlDataFromServer(void) {
  bool result = false;
  
    if (WifiState != connected) {
        return false;
    }

  setClock(); 

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client -> setCACert(rootCACertificate);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      if (https.begin(*client, ARSO_SERVER_XML)) {  // HTTPS
        Serial.print("[HTTPS] GET...\r\n");
        // start connection and send HTTP header
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            XMLdata = https.getString();
            result = true;
            /*
            Serial.println("--- XML data begin ---");
            Serial.println(XMLdata);
            Serial.println("--- XML data end ---");
            */
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



String xmlFindParam(String& inStr, String needParam, unsigned int& Position)
{
    int indexStart = inStr.indexOf("<"+needParam+">", Position);
    if (indexStart > 0) {
        int indexStop = inStr.indexOf("</"+needParam+">", indexStart);  
        Position = indexStop;
        int CountChar = needParam.length();
        return inStr.substring(indexStart+CountChar+2, indexStop);
    }
    return "/";
}


ArsoWeather_t ArsoWeather[3];

bool GetARSOdata(void) {
    bool result = false;

    if (millis() < (LastTimeArsoRefreshed * 60*1000)) {  // check server every hour
      Serial.println("ARSO data is valid.");
      return true;  // data is already valid
    }

    Serial.println("Requesting data from ARSO server...");
    DisplayClear;
    DisplayText("Reading ARSO server...\n");
    if (!GetXmlDataFromServer()) {
        XMLdata = "";  // free memory
        DisplayText("FAILED!\n");
        return false;
    }

    DisplayText("OK\nParsing data...");
    unsigned int ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Day = utf8ascii(xmlFindParam(XMLdata, "valid_day", ParamPos).c_str());
        // remove text after space
        ArsoWeather[i].Day.remove(ArsoWeather[i].Day.indexOf(" "));
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].PartOfDay = utf8ascii(xmlFindParam(XMLdata, "valid_daypart", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Rain = utf8ascii(xmlFindParam(XMLdata, "wwsyn_shortText", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Sky = utf8ascii(xmlFindParam(XMLdata, "nn_shortText", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Temperature = utf8ascii(xmlFindParam(XMLdata, "t_degreesC", ParamPos).c_str());
    }
    result = true;
    LastTimeArsoRefreshed = millis();
    XMLdata = "";  // free memory

    for (uint8_t i = 0; i < 3; i++)
    {
      Serial.println("------------");
      Serial.println(ArsoWeather[i].Day + "  " +ArsoWeather[i].PartOfDay);
      Serial.println(ArsoWeather[i].Sky);
      Serial.println(ArsoWeather[i].Rain);
      Serial.println(ArsoWeather[i].Temperature);
    }

    DisplayText("Finished\n");
    return result;
}
