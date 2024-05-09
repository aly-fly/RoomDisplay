#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include <utils.h>
#include "Clock.h"
#include "OsDomzale_https_certificate.h"
#include "display.h"
#include "GlobalVariables.h" // gBuff3k

/*
 1. Load https://www.os-domzale.si/ (128 kB)
 2. Search for "<strong>Pomembne povezave</strong>" (located at ~53 kB; need ~600 bytes following this text)
 3. Search for "<li><a href="https://www.os-domzale.si/files/2024/04/jedilnik-2024-5-1.pdf">Jedilnik</a></li>"
 4. Extract URL of the PDF file
 5. Send to Zamzar cor conversion
 6. Download contents of the text file (4 kB)
 7. Get current day
 8. Analyze contents and extract data for today
*/

const String OSD_URL = "https://www.os-domzale.si/";
String PDF_URL;

bool GetPdfLinkFromMainWebsite(void) {
  bool result = false;
  bool Finished = false;
  unsigned int NoMoreData = 0;
  PDF_URL = "";

  if (!WiFi.isConnected()) {
      return false;
  }

  setClock(); 

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client -> setCACert(rootCACertificate_OsDomzale);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      DisplayText("HTTPS begin\n");
      if (https.begin(*client, OSD_URL)) {  // HTTPS
        yield(); // watchdog reset
        Serial.print("[HTTPS] GET...\r\n");
        DisplayText("HTTPS get request: ");
        // start connection and send HTTP header
        int httpCode = https.GET();
        yield(); // watchdog reset
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
          DisplayText(String(httpCode).c_str());
          DisplayText("\n");
  
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
                Serial.println("[HTTPS] Streaming data from server in 3k byte chunks.");
                DisplayText("Reading data");

// stream data in chunks
                // get length of document (is -1 when Server sends no Content-Length header)
                int DocumentLength = https.getSize();
                Serial.printf("[OSD] Document size: %d \r\n", DocumentLength);
                // get tcp stream
                WiFiClient * stream = https.getStreamPtr();

                // read all data from server
                while (https.connected() && (DocumentLength > 0 || DocumentLength == -1)) {
                    yield(); // watchdog reset
                    // get available data size
                    size_t StreamAvailable = stream->available();
                    size_t BytesRead;

                    if (StreamAvailable) {
                        // read up to 3000 bytes
                        BytesRead = stream->readBytes(gBuff3k, ((StreamAvailable > sizeof(gBuff3k)) ? sizeof(gBuff3k) : StreamAvailable));
                        Serial.print("/");
                        DisplayText(".");

                        if(DocumentLength > 0) {
                            DocumentLength -= BytesRead;
                        }
                        // process received data chunk
                        if (BytesRead > 0) {
                          // covert data to String
                          String sBuf;
                          sBuf = String(gBuff3k, BytesRead);
                          // glue last section of the previous buffer
                          // TO-DO wwwwwwwwwww

                          int idx = sBuf.indexOf("Pomembne povezave");
                          if (idx >= 0) {
                            Serial.println();
                            Serial.print("Header found at ");
                            Serial.print(idx);
                            Serial.println();
                            DisplayText("\nHeader found.\n", CLGREEN);

                            // scan all "<a href=...</a>" strings for the one containing "Jedilnik"
                            int idx1, idx2;
                            String aHref;
                            idx1 = sBuf.indexOf("<a href=", idx);
                            idx2 = sBuf.indexOf("</a>", idx1);
                            while ((idx1 > 0) && (idx2 > idx1)) {
                              aHref = sBuf.substring(idx1, idx2+4);
                              Serial.println(idx1);
                              Serial.println(idx2);
                              Serial.println(aHref);
                              if (aHref.indexOf("Jedilnik") > 0) {
                                Serial.println("Link found");

                                idx1 = aHref.indexOf("\"");
                                idx2 = aHref.indexOf("\"", idx1);
                                if ((idx1 > 0) && (idx2 > idx1)) {
                                  PDF_URL = aHref.substring(idx1+1, idx2);
                                  Serial.println(PDF_URL);
                                  DisplayText(PDF_URL.c_str(), CLCYAN);
                                  DisplayText("\n");
                                  Finished = true;
                                }
                                if (Finished) {break;}
                                } // Jedilnik
                              // look for next one
                              idx1 = sBuf.indexOf("<a href=", idx2+4);
                              idx2 = sBuf.indexOf("</a>", idx1);
                            } // while href scan
                          } // header found
                        } // process data (BytesRead > 0)
                    } // data available in stream
                    delay(10);
                    // No more data being received? 10 retries..
                    if (StreamAvailable == 0) {
                      NoMoreData++;
                      delay(150);
                    }
                    else {
                      NoMoreData = 0;
                    }
                    if (Finished || (NoMoreData > 10)) { break; }
                } // connected or document still has data
                Serial.println();
                if (NoMoreData > 10) {
                  Serial.println("[HTTPS] Timeout.");
                  DisplayText("Timeout.\n");
                } else {
                  Serial.println("[HTTPS] Connection closed or file end.");
                }
// streaming end
            DisplayText("\n");
            result = Finished;
          } // HTTP code > 0
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\r\n", https.errorToString(httpCode).c_str());
          DisplayText("Error: ");
          DisplayText(https.errorToString(httpCode).c_str());
          DisplayText("\n");
        }
  
        https.end();
      } else {
        Serial.println("[HTTPS] Unable to connect");
        DisplayText("Unable to connect.\n");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    Serial.println("Unable to create HTTPS client");
  }
    if (result){
      Serial.println("OK");
      DisplayText("OK\n", CLGREEN);
    }
    return result;
}


void GetJedilnik(void){
  GetPdfLinkFromMainWebsite();
}
