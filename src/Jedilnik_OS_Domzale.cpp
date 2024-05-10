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
#include "GlobalVariables.h"
#include "Zamzar.h"
#include <FS.h>
#include <SPIFFS.h>
#include "ArsoXml.h"

#define FileName "/jOsDom.txt"
String Pon, Tor, Sre, Cet, Pet;

unsigned long LastTimeJedilnikRefreshed = 0; // data is not valid

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
String PDF_URL, Saved_PDF_URL;


bool GetPdfLinkFromMainWebsite(void) {
  bool result = false;
  bool Finished = false;
  unsigned int NoMoreData = 0;
  PDF_URL = "";

  if (!WiFi.isConnected()) {
      return false;
  }

  setClock(); 

  DisplayText("Contacting: ");
  DisplayText(OSD_URL.c_str());
  DisplayText("\n");
  String sBufOld;

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
                while (https.connected() && (DocumentLength > 0 || DocumentLength == -1) && (!Finished)) {
                    yield(); // watchdog reset
                    // get available data size
                    size_t StreamAvailable = stream->available();
                    size_t BytesRead;

                    if (StreamAvailable) {
                        // read up to 3000 bytes
                        BytesRead = stream->readBytes(gBuffer, ((StreamAvailable > sizeof(gBuffer)) ? sizeof(gBuffer) : StreamAvailable));
                        Serial.print("/");
                        DisplayText(".");

                        if(DocumentLength > 0) {
                            DocumentLength -= BytesRead;
                        }
                        // process received data chunk
                        if (BytesRead > 0) {
                          // covert data to String
                          String sBuf;
                          sBuf = sBufOld;
                          sBuf.concat(String(gBuffer, BytesRead));
                          // glue last section of the previous buffer

                          int idx = sBuf.indexOf("Pomembne povezave");
                          if (idx >= 0) {
                            Serial.println();
                            Serial.print("Header found at ");
                            Serial.println(idx);
                            DisplayText("\nHeader found.\n", CLGREEN);

                            // scan all "<a href=...</a>" strings for the one containing "Jedilnik"
                            int idx1, idx2;
                            String aHref;
                            idx1 = sBuf.indexOf("<a href=", idx);
                            idx2 = sBuf.indexOf("</a>", idx1);
                            while ((idx1 > 0) && (idx2 > idx1) && (!Finished)) {
                              aHref = sBuf.substring(idx1, idx2+4);
                              Serial.println(idx1);
                              Serial.println(idx2);
                              Serial.println(aHref);
                              if (aHref.indexOf("Jedilnik") > 0) {
                                Serial.println("Link found");
                                DisplayText("Link found.\n", CLGREEN);

                                idx1 = aHref.indexOf("\"");
                                idx2 = aHref.indexOf("\"", idx1+1);
                                if ((idx1 > 0) && (idx2 > idx1)) {
                                  PDF_URL = aHref.substring(idx1+1, idx2);
                                  Serial.println("URL found");
                                  DisplayText("URL found.\n", CLGREEN);
                                  Finished = true;
                                } else { // fail
                                  NoMoreData = 999;
                                }
                              } // Jedilnik
                              if (Finished) {break;}
                              // look for next one
                              idx1 = sBuf.indexOf("<a href=", idx2+4);
                              idx2 = sBuf.indexOf("</a>", idx1);
                            } // while href scan
                          } // header found
                          sBufOld = String(gBuffer, BytesRead);
                        } // process data (BytesRead > 0)
                    } // data available in stream
                    delay(20);
                    // No more data being received? 10 retries..
                    if (StreamAvailable == 0) {
                      NoMoreData++;
                      delay(150);
                      Serial.print("+");
                      //DisplayText("+");
                    }
                    else {
                      NoMoreData = 0;
                    }
                    if (Finished || (NoMoreData > 100)) { break; }
                } // connected or document still has data
                Serial.println();
                if (NoMoreData > 100) {
                  Serial.println("[HTTPS] Timeout.");
                  DisplayText("\nTimeout.\n", CLRED);
                } else {
                  Serial.println("[HTTPS] Connection closed or file end.");
                }
// streaming end
            //DisplayText("\n");
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
      Serial.println(PDF_URL);
      DisplayText(PDF_URL.c_str(), CLCYAN);
      DisplayText("\n");
      Serial.println("Website read OK");
      DisplayText("Website read OK\n", CLGREEN);
    } else {
      Serial.println("Website read & PDF search FAILED");
      DisplayText("Website read & PDF search FAILED\n", CLRED);
    }
    return result;
}



bool ReadSavedFile(void){
  Serial.println("Reading local file");
  DisplayText("Opening local file...");

  fs::File file = SPIFFS.open(FileName);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    DisplayText("Fail\n", CLRED);
    return false;
  }

  DisplayText(" Reading...");
  Serial.println("- reading from file");
  Saved_PDF_URL = file.readStringUntil(13);
  Pon = file.readStringUntil(13);
  Tor = file.readStringUntil(13);
  Sre = file.readStringUntil(13);
  Cet = file.readStringUntil(13);
  Pet = file.readStringUntil(13);
  file.close();

  Pon.trim();
  Tor.trim();
  Sre.trim();
  Cet.trim();
  Pet.trim();

  Serial.println("-------------------");
  Serial.println(Saved_PDF_URL);
  Serial.println(Pon);
  Serial.println(Tor);
  Serial.println(Sre);
  Serial.println(Cet);
  Serial.println(Pet);
  Serial.println("-------------------");

  
  bool ok;
  ok = (Saved_PDF_URL.length() > 0) && (Pon.length() > 0) && (Tor.length() > 0) && (Sre.length() > 0) && (Cet.length() > 0) && (Pet.length() > 0);

  if (ok){
    Serial.println("OK");
    DisplayText("OK\n", CLGREEN);
  } else {
    Serial.println("FAIL");
    DisplayText("FAIL\n", CLRED);
  }

  return ok;  
}  


void GetJedilnikOsDomzale(void){
  bool PdfOk, FileOk, NeedFreshData;

  if ((millis() < (LastTimeJedilnikRefreshed + 60*60*1000)) && (LastTimeJedilnikRefreshed != 0)) {  // check server every hour
    Serial.println("Jedilnik OS: Data is valid.");
    return;  // data is already valid
  }


  DisplayClear();
  PdfOk = GetPdfLinkFromMainWebsite();
  FileOk = ReadSavedFile();
  NeedFreshData = false;

  if (PdfOk) LastTimeJedilnikRefreshed = millis();

  if (PdfOk && FileOk) {
    Serial.print("Comparing URLs.. ");
    DisplayText("Comparing URLs.. ");
    NeedFreshData = (Saved_PDF_URL != PDF_URL);

    if (NeedFreshData){
      Serial.println("NO match");
      DisplayText("NO match\n", CLRED);
    } else {
      Serial.println("match");
      DisplayText("match\n", CLGREEN);
    }
  }

  NeedFreshData = NeedFreshData || !FileOk;

  if (NeedFreshData && PdfOk) {
    if (ConvertPdfToTxt(PDF_URL)) {
      Serial.println("Conversion finished OK");
      DisplayText("Conversion finished OK\n", CLGREEN);
      //Serial.println(ZamzarData);
      String Jedilnik = utf8ascii(ZamzarData.c_str());
      ZamzarData.clear(); // free mem
      //Serial.println(Jedilnik);
      // remove footer
      Jedilnik.remove(Jedilnik.indexOf("Pridruzujemo"));
      // remove header
      Jedilnik.remove(0, Jedilnik.indexOf("PETEK") + 6);
      Serial.println(Jedilnik);

      // try fixed width
      unsigned int column = 0;
      char chr;
      for (unsigned int i = 0; i < Jedilnik.length(); i++)
      {
        chr = Jedilnik.charAt(i);
        if ((chr == 13) || (chr == 10)) column = 0; // new line
        if (column < 24) {
          // obrok
        } else 
        if (column < 54) {
          Pon.concat(chr);
        } else 
        if (column < 83) {
          Tor.concat(chr);
        } else 
        if (column < 109) {
          Sre.concat(chr);
        } else 
        if (column < 149) {
          Cet.concat(chr);
        } else {
          Pet.concat(chr);
        }
        column++;
      }
      Serial.println();

      Jedilnik.clear(); // free mem    
      
      TrimDoubleSpaces(Pon);
      TrimDoubleSpaces(Tor);
      TrimDoubleSpaces(Sre);
      TrimDoubleSpaces(Cet);
      TrimDoubleSpaces(Pet);

      Serial.println("-------------------");
      Serial.println(Pon);
      Serial.println("-------------------");
      Serial.println(Tor);
      Serial.println("-------------------");
      Serial.println(Sre);
      Serial.println("-------------------");
      Serial.println(Cet);
      Serial.println("-------------------");
      Serial.println(Pet);
      Serial.println("-------------------");


      fs::File file = SPIFFS.open(FileName, FILE_WRITE);
      if(!file){
          Serial.println("- failed to open file for writing");
          return;
      }
      if(file.println(PDF_URL)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      if(file.println(Pon)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      if(file.println(Tor)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      if(file.println(Sre)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      if(file.println(Cet)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      if(file.println(Pet)){
          Serial.println("- file written");
      } else {
          Serial.println("- write failed");
      }
      file.close();

    } else { // ConvertPdfToTxt OK
      return;
    }
  } // (NeedFreshData && PdfOk)
}


void DrawJedilnikOsDomzale(void) {
  DisplayClear();
  String sToday = ArsoWeather[0].DayName;
  sToday.toUpperCase();
  sToday.remove(3);

  DisplayText(sToday.c_str(), 1, 50, 5, CLBLUE);
  if (sToday == "PON") DisplayText(Pon.c_str(), 1, 1, 30, CLYELLOW, true); else
  if (sToday == "TOR") DisplayText(Tor.c_str(), 1, 1, 30, CLYELLOW, true); else
  if (sToday == "SRE") DisplayText(Sre.c_str(), 1, 1, 30, CLYELLOW, true); else
  if (sToday == "CET") DisplayText(Cet.c_str(), 1, 1, 30, CLYELLOW, true); else
  if (sToday == "PET") DisplayText(Pet.c_str(), 1, 1, 30, CLYELLOW, true); else {
    DisplayText(Pon.c_str(), CLYELLOW);
    DisplayText("\n");
    DisplayText(Tor.c_str(), CLGREEN);
    DisplayText("\n");
    DisplayText(Sre.c_str(), CLCYAN);
    DisplayText("\n");
  }

}
