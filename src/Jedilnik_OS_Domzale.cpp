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
String Jedilnik[5];
String JedilnikDatum;

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
  Serial.println("GetPdfLinkFromMainWebsite()");
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
  sBufOld.clear(); // free mem
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
  Serial.println("ReadSavedFile()");
  DisplayText("Opening local file...");

  fs::File file = SPIFFS.open(FileName);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    DisplayText("Fail\n", CLRED);
    return false;
  }

  DisplayText(" Reading...");
  Serial.println("Reading from file");
  Saved_PDF_URL = file.readStringUntil(13);
  JedilnikDatum = file.readStringUntil(13);
  TrimOnlyPrintable(JedilnikDatum);
  for (int i = 0; i < 5; i++)
  {
    Jedilnik[i] = file.readStringUntil(13);
    TrimOnlyPrintable(Jedilnik[i]);
  }
  file.close();

  Serial.println("-------------------");
  Serial.println(Saved_PDF_URL);
  Serial.println("-------------------");
  Serial.println(JedilnikDatum);
  Serial.println("-------------------");
  for (int i = 0; i < 5; i++)
  {
    Serial.println(Jedilnik[i]);
  }
  Serial.println("-------------------");

  
  bool ok;
  ok = (Saved_PDF_URL.length() > 0) && (JedilnikDatum.length() > 0) && (Jedilnik[0].length() > 0) && (Jedilnik[1].length() > 0) && (Jedilnik[2].length() > 0) && (Jedilnik[3].length() > 0) && (Jedilnik[4].length() > 0);

  if (ok){
    Serial.println("File read OK");
    DisplayText("File read OK\n", CLGREEN);
  } else {
    Serial.println("File read FAIL");
    DisplayText("File read FAIL\n", CLRED);
  }

  return ok;  
}  


void GetJedilnikOsDomzale(void){
  Serial.println("GetJedilnikOsDomzale()");
  bool PdfOk, FileOk, NeedFreshData;

  if ((millis() < (LastTimeJedilnikRefreshed + 2*60*60*1000)) && (LastTimeJedilnikRefreshed != 0)) {  // check server every 2 hours
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
      String CelJedilnik = utf8ascii(ZamzarData.c_str());
      ZamzarData.clear(); // free mem
      //Serial.println(Jedilnik);

      JedilnikDatum.clear();
      int idx = CelJedilnik.indexOf("2024");
      if (idx > 0) {
        JedilnikDatum = CelJedilnik.substring(idx-15, idx+4);
        TrimDoubleSpaces(JedilnikDatum);
      }

      // remove footer
      CelJedilnik.remove(CelJedilnik.indexOf("Pridruzujemo"));
      // remove header
      CelJedilnik.remove(0, CelJedilnik.indexOf("PETEK") + 6);
      Serial.println("===== File contents ==========================================");
      Serial.println(CelJedilnik);
      Serial.println("==============================================================");

      for (int i = 0; i < 5; i++)
      {
        Jedilnik[i].clear();
      }
      // try extraction with fixed width
      unsigned int column = 0;
      char chr;
      for (unsigned int i = 0; i < CelJedilnik.length(); i++)
      {
        chr = CelJedilnik.charAt(i);
        if ((chr == 13) || (chr == 10)) column = 0; // new line
        if (column < 24) {
          // obrok
        } else 
        if (column < 54) {
          Jedilnik[0].concat(chr);
        } else 
        if (column < 83) {
          Jedilnik[1].concat(chr);
        } else 
        if (column < 109) {
          Jedilnik[2].concat(chr);
        } else 
        if (column < 149) {
          Jedilnik[3].concat(chr);
        } else {
          Jedilnik[4].concat(chr);
        }
        column++;
      }
      Serial.println();

      CelJedilnik.clear(); // free mem    

      // clean un-wanted words and chars
      for (int i = 0; i < 5; i++)
      {
        int idx = Jedilnik[i].indexOf("SSSZ:");
        if (idx > 0) Jedilnik[i].remove(idx, 5);
        idx = Jedilnik[i].indexOf("*");
        if (idx > 0) Jedilnik[i].remove(idx, 1);
        idx = Jedilnik[i].indexOf("1");
        if (idx > 0) Jedilnik[i].remove(idx, 1);
      }

      // list extracted data
      Serial.println("-------------------");
      Serial.println(PDF_URL);
      Serial.println("-------------------");
      Serial.println(JedilnikDatum);
      Serial.println("-------------------");
      for (int i = 0; i < 5; i++)
      {
        TrimDoubleSpaces(Jedilnik[i]);
        Serial.println(Jedilnik[i]);
      }
      Serial.println("-------------------");

      Serial.println("Saving data into local file...");
      DisplayText("Saving data into local file...");
      fs::File file = SPIFFS.open(FileName, FILE_WRITE);
      if(!file){
          Serial.println("- failed to open file for writing");
          DisplayText("FAIL\n", CLRED);
          delay(1500);
          return;
      }
      bool ok = true;
      if(file.println(PDF_URL)){
          Serial.println("- file written");
          DisplayText(".");
      } else {
          Serial.println("- write failed");
          ok = false;         
      }
      if(file.println(JedilnikDatum)){
          Serial.println("- file written");
          DisplayText(".");
      } else {
          Serial.println("- write failed");
          ok = false;         
      }

      for (int i = 0; i < 5; i++)
      {
        if(file.println(Jedilnik[i])){
            Serial.println("- file written");
            DisplayText(".");
        } else {
            Serial.println("- write failed");
            ok = false;         
        }
      }
      file.close();
      if(ok) {
          Serial.println("File write OK");
          DisplayText("OK\n", CLGREEN);
      } else {
          Serial.println("File write FAIL");
          DisplayText("FAIL\n", CLRED);
          delay(1500);
          return;
      }

    } else { // ConvertPdfToTxt OK
      return;
    }
  } // (NeedFreshData && PdfOk)
}



int FindUppercaseChar(String &Str, const int StartAt = 0) {
  char chr;
  for (unsigned int i = StartAt; i < Str.length(); i++)
  {
    chr = Str.charAt(i);
    if ((chr >= 65) && (chr <= 90)) // A..Z
    {
      return i;
    }    
  }
  return -1;  
}


const char* DAYS1[] = { "PON", "TOR", "SRE", "CET", "PET", "SOB", "NED" };

void DrawJedilnikOsDomzale(void) {
  Serial.println("DrawJedilnikOsDomzale()");
  DisplayClear();
  String sToday = ArsoWeather[0].DayName;
  sToday.toUpperCase();
  sToday.remove(3);
  Serial.print("Today = ");
  Serial.println(sToday);
  
  int idx1, idx2;
  String Jed[4];
  bool Workday = false;
  int Hr;
  for (int dan = 0; dan < 5; dan++) { // PON..PET
    if (sToday.indexOf(DAYS1[dan]) == 0) 
    {
      Workday = true;
      // show next day, if clock is available
      if (inHomeLAN) {
        if (CurrentHour(Hr)) {
          if ((Hr > 17) && (dan < 4)) {
            dan++;
          }
        }
      }

      Serial.println("----------------------");
      // split to separate meals
      idx1 = 0;
      idx2 = 0;
      for (int jed = 0; jed < 4; jed++)  // ZAJTRK, DOP. MALICA, KOSILO, POP. MALICA
      {
        Jed[jed].clear();
        idx2 = FindUppercaseChar(Jedilnik[dan], idx1+1);
        Serial.print("idx2 = ");
        Serial.println(idx2);
        if (idx2 < idx1) idx2 = Jedilnik[dan].length();
        Jed[jed] = Jedilnik[dan].substring(idx1, idx2);
        idx1 = idx2;
        Serial.println(Jed[jed]);
        Serial.println("----------------------");
      }
      // backup, if splitting does not work correctly
      if ((Jed[1].length() < 5) || (Jed[2].length() < 5)) {
        Jed[1] = Jedilnik[dan];
        Jed[2].clear();
      }
    }
  }
  if (Workday) {
    DisplayText(JedilnikDatum.c_str(), 1, 110, 15, CLBLUE);
    DisplayText(sToday.c_str(), 1, 20, 15, CLGREY);
    DisplayText(Jed[1].c_str(), 1, 1,  50, CLYELLOW, true);
    DisplayText(Jed[2].c_str(), 1, 1, 130, CLCYAN, true);
  } else { // weekend
    DisplayText(JedilnikDatum.c_str(), 1, 10, 1, CLBLUE);
    DisplayText("\n\n\n======================================\n", CLGREY);
    for (int i = 0; i < 5; i++) {
      DisplayText(Jedilnik[i].c_str(), CLYELLOW);
      DisplayText("\n======================================\n", CLGREY);
    }
    delay(1500);
  }
}
