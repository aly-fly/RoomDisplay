// ARSO xml: https://meteo.arso.gov.si/met/sl/service/
// osrednja 3 dni: https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SLOVENIA_MIDDLE_latest.xml
// osrednja čez dan: https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SI_OSREDNJESLOVENSKA_latest.xml

#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include <utils.h>
#include "Clock.h"
#include "ArsoXml.h"
#include "Arso_https_certificate.h"
#include "display.h"

unsigned long LastTimeArsoRefreshed = 0; // data is not valid


// reference: "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\BasicHttpsClient\BasicHttpsClient.ino"

String XMLdata;

bool GetXmlDataFromServer(void) {
  bool result = false;
  
    if (WifiState != connected) {
        return false;
    }

  setClock(); 

  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client -> setCACert(rootCACertificate_Arso);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      if (https.begin(*client, ARSO_SERVER_XML_URL)) {  // HTTPS
        Serial.print("[HTTPS] GET...\r\n");
        // start connection and send HTTP header
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
  
          // file found at server
          if ((httpCode == HTTP_CODE_OK) || (httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {
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


void TrimLogWords(String& Txt) {
  // trim long words
  if (Txt.length() > 4) {
    Txt.remove(5);
    Txt.concat('.');          
  }
}

ArsoWeather_t ArsoWeather[3];

bool GetARSOdata(void) {
    bool result = false;

    if ((millis() < (LastTimeArsoRefreshed + 60*60*1000)) && (LastTimeArsoRefreshed != 0)) {  // check server every hour
      Serial.println("ARSO data is valid.");
      return true;  // data is already valid
    }

    Serial.println("Requesting data from ARSO server...");
    DisplayClear();
    DisplayText("Reading ARSO server...\n", CLYELLOW);
    if (!GetXmlDataFromServer()) {
        XMLdata = "";  // free memory
        DisplayText("FAILED!\n", CLRED);
        return false;
    }

    Serial.println("[ARSO] Free memory: " + String(esp_get_free_heap_size()) + " bytes");

    DisplayText("OK\n", CLGREEN);
    DisplayText("Parsing data...\n");
    int ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Day = utf8ascii(FindXMLParam(XMLdata, "valid_day", ParamPos).c_str());
        // remove text after space
        ArsoWeather[i].Day.remove(ArsoWeather[i].Day.indexOf(" "));
        TrimLogWords(ArsoWeather[i].Day);
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].PartOfDay = utf8ascii(FindXMLParam(XMLdata, "valid_daypart", ParamPos).c_str());
        TrimLogWords(ArsoWeather[i].PartOfDay);
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Rain = utf8ascii(FindXMLParam(XMLdata, "wwsyn_shortText", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Sky = utf8ascii(FindXMLParam(XMLdata, "nn_shortText", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].Temperature = utf8ascii(FindXMLParam(XMLdata, "t_degreesC", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].WeatherIcon = utf8ascii(FindXMLParam(XMLdata, "nn_icon-wwsyn_icon", ParamPos).c_str());
    }
    ParamPos = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        ArsoWeather[i].WindIcon = utf8ascii(FindXMLParam(XMLdata, "ddff_icon", ParamPos).c_str());
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
      Serial.println(ArsoWeather[i].WeatherIcon);
      Serial.println(ArsoWeather[i].WindIcon);
      Serial.println("------------");
    }

    DisplayText("Finished\n");
    delay (500);
    return result;
}
