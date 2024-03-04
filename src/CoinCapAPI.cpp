

// https://docs.coincap.io/
// Free Tier (No API Key): 200 requests per minute

#include <Arduino.h>
#include <stdint.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "__CONFIG.h"
#include "myWiFi.h"
#include <utils.h>
#include "Clock.h"
#include "CoinCap_https_certificate.h"
#include "display.h"

#define MAX_DATA_POINTS (31*24 + 10)

//uint32_t CoinCapData[MAX_DATA_POINTS];  // data for 1 month = 3 kB (plus headroom for safety)
float_t CoinCapData[MAX_DATA_POINTS];  // data for 1 month = 3 kB
unsigned int CoinCapDataLength = 0;

unsigned long LastTimeCoinCapRefreshed = 0; // data is not valid

// reference: "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\BasicHttpsClient\BasicHttpsClient.ino"
//            "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\StreamHttpClient\StreamHttpClient.ino"

// value every 5 min: https://api.coincap.io/v2/assets/bitcoin/history?interval=m5
// value every hour:  https://api.coincap.io/v2/assets/bitcoin/history?interval=h1
// average every day: https://api.coincap.io/v2/assets/bitcoin/history?interval=d1

#define COINCAP_URL  "https://api.coincap.io/v2/assets/bitcoin/history?interval=h1"

bool GetDataFromCoinCapServer(void) {
  bool result = false;
  unsigned long StartTime = millis();
  bool Timeout = false;
  unsigned int NoMoreData = 0;
  unsigned int JsonDataSize = 0;
  CoinCapDataLength = 0;
  
  if (WifiState != connected) {
      return false;
  }

  setClock(); 

  WiFiClientSecure *client = new WiFiClientSecure;
  if (client) {
    client -> setCACert(rootCACertificate_CoinCap);

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      Serial.print("[HTTPS] begin...\r\n");
      if (https.begin(*client, COINCAP_URL)) {  // HTTPS
        yield(); // watchdog reset
        Serial.print("[HTTPS] GET...\r\n");
        // start connection and send HTTP header
        int httpCode = https.GET();
        yield(); // watchdog reset
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);
  
          // file found at server
          if (httpCode == HTTP_CODE_OK) {
                Serial.println("[HTTPS] Streaming data from server in 2k byte chunks.");
                DisplayText("Reading data");

// stream data in chunks
                // get length of document (is -1 when Server sends no Content-Length header)
                int DocumentLength = https.getSize();
                Serial.printf("[COINCAP] Document size: %d \r\n", DocumentLength);

                // create buffer for read
                uint8_t buff[2000] = { 0 };
                bool firstBuffer = true;

                // get tcp stream
                WiFiClient * stream = https.getStreamPtr();

                // read all data from server
                while (https.connected() && (DocumentLength > 0 || DocumentLength == -1)) {
                    yield(); // watchdog reset
                    // get available data size
                    size_t StreamAvailable = stream->available();
                    size_t BytesRead;

                    if (StreamAvailable) {
                        // read up to 2000 bytes
                        BytesRead = stream->readBytes(buff, ((StreamAvailable > sizeof(buff)) ? sizeof(buff) : StreamAvailable));
                        JsonDataSize += BytesRead;

                        // write it to Serial
                        if (firstBuffer) { Serial.write(buff, BytesRead);  Serial.println(); }
                          else { Serial.print("/"); }
                        firstBuffer = false;
                        DisplayText(".");

                        if(DocumentLength > 0) {
                            DocumentLength -= BytesRead;
                        }
                        // process received data chunk
                        if (BytesRead > 0) {
                          // covert data to String
                          String sBuf;
                          sBuf = String(buff, BytesRead);
                          // glue last section of the previous buffer
                          // TO-DO wwwwwwwwwww

                          int pos = 0;
                          String sVal;

                          while (pos >= 0) {
                            sVal = FindJsonParam(sBuf, "priceUsd", pos);
                            if (pos >= 0) {
                              //Serial.println(pos);
                              TrimNumDot(sVal);  // delete everything except numbers and "."
                              //Serial.println(sVal);
                              CoinCapData[CoinCapDataLength] = sVal.toFloat();
                              CoinCapDataLength++;
                              if (CoinCapDataLength > MAX_DATA_POINTS) {
                                Serial.println("MAX NUMBER OF DATA POINTS REACHED!");
                                NoMoreData = 1000;
                                break;
                              }
                            } // if data found
                          } // while data found
                        } // process data
                    } // data available in stream
                    delay(10);
                    // No more data being received? 10 retries..
                    if (StreamAvailable == 0) NoMoreData++;
                    if (NoMoreData > 10) { break; }
                    // timeout
                    Timeout = (millis() > (StartTime + 20 * 1000));  // 20 seconds
                    if (Timeout) { break; }
                } // connected or document still has data
                Serial.println();
                if (Timeout) {
                  Serial.println("[HTTPS] Timeout.");
                } else {
                  Serial.println("[HTTPS] Connection closed or file end.");
                }
// streaming end
            DisplayText("\n");
            result = true;
          } // HTTP code ok
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
    Serial.print("Time needed (ms): ");
    Serial.println(millis() - StartTime);
    Serial.print("Json data size (bytes): ");
    Serial.println(JsonDataSize);
    return result;
}






bool GetCoinCapData(void) {
    bool result = false;

    if ((millis() < (LastTimeCoinCapRefreshed + 30*60*1000)) && (LastTimeCoinCapRefreshed != 0)) {  // check server every 1/2 hour
      Serial.println("CoinCap data is valid.");
      return true;  // data is already valid
    }

    Serial.println("Requesting data from CoinCap server...");
    DisplayClear();
    DisplayText("Contacting COINCAP server...\n", CLYELLOW);
    if (!GetDataFromCoinCapServer()) {
        DisplayText("FAILED!\n", CLRED);
        return false;
    }
    LastTimeCoinCapRefreshed = millis();
    result = true;

    Serial.println("Number of data points: " + String(CoinCapDataLength));
    char Txt[20];
    sprintf(Txt, "Data points: %d\n");
    DisplayText(Txt);
/*
    Serial.println("------------");
    for (uint16_t i = 0; i < CoinCapDataLength; i++)
    {
      Serial.println(String(CoinCapData[i]));
    }
    Serial.println("------------");
*/    

    DisplayText("Finished\n", CLGREEN);
    delay (1500);
    return result;
}










#ifdef DISPLAY_LCD_ST7735_Bodmer
  #include <TFT_eSPI.h>

// display 128 x 160
// TFT_HEIGHT TFT_WIDTH -> vertical orientation!
#define DspH  TFT_WIDTH
#define DspW  TFT_HEIGHT




void PlotCoinCapData(void) {
  DisplayClear();
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawString("BTC", 5, (DspH/2 - 5), 2);

  if (CoinCapDataLength < DspW) {
    Serial.println("Not enough data to plot!");
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("NOT ENOuGH DATA", 10, 20, 2);
    return;
  }

  float_t Minn, Maxx;
  Minn =  999999999;
  Maxx = -999999999;
  uint16_t IgnoreFirst = CoinCapDataLength - DspW;

  for (uint16_t i = IgnoreFirst; i < CoinCapDataLength; i++)
  {
    if (CoinCapData[i] > Maxx) {Maxx = CoinCapData[i];}
    if (CoinCapData[i] < Minn) {Minn = CoinCapData[i];}
  }
  Serial.println("Min: " + String(Minn));
  Serial.println("Max: " + String(Maxx));

  // for display only
  float MinnD = Minn - 200;
  float MaxxD = Maxx + 200;
  
  uint32_t color;
  int diff;
  uint16_t X, iY, oldY, Y, h;
  float_t Scaling, fY;
  Scaling = (float(DspH) / (MaxxD - MinnD));
  Serial.printf("Scaling: %f.5\r\n", Scaling);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (X = 0; X < DspW; X++) {
    fY = (CoinCapData[IgnoreFirst + X] - MinnD) * Scaling;
    iY = round(fY);
    if (iY < 0) iY = 0;
    if (iY >= DspH) iY = DspH-1;
    if (X == 0) {oldY = iY-1;}
    diff = iY - oldY;
    h = abs(diff);
    if (h == 0) {h = 1;}
     // Y0 on display is on the top -> mirror Y
    if (diff < 0) {
      color = TFT_RED; 
      Y = DspH - oldY;
    } else {
      color = TFT_GREEN; 
      Y = DspH - iY;
    }
    tft.drawFastVLine(X, Y, h, color);
    oldY = iY;
  }
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawNumber(round(Maxx), 30, 2, 1);
  tft.drawNumber(round(Minn), 30, DspH - 12, 1);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  X = tft.drawNumber(round(CoinCapData[CoinCapDataLength-1]), 90, 2, 1);
  tft.drawString("USD", 90 + X + 3, 2, 1);
}

#endif  // tft bodmer
