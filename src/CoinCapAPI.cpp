

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

#define MAX_DATA_POINTS_1H (31*24 + 10)
#define MAX_DATA_POINTS_5M ( 1440 + 10)

#define REQ_DATA_POINTS_1H (31*24 - 70)
#define REQ_DATA_POINTS_5M ( 1440 - 70)

float_t CoinCapData_1H[MAX_DATA_POINTS_1H];  // data for 1 month = 3 kB (4B / point)
float_t CoinCapData_5M[MAX_DATA_POINTS_5M];  // data = 5,8 kB
unsigned int CoinCapDataLength_1H = 0;
unsigned int CoinCapDataLength_5M = 0;

unsigned long LastTimeCoinCapRefreshed_1H = 0; // data is not valid
unsigned long LastTimeCoinCapRefreshed_5M = 0; // data is not valid

// reference: "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\BasicHttpsClient\BasicHttpsClient.ino"
//            "C:\Users\yyyyy\.platformio\packages\framework-arduinoespressif32\libraries\HTTPClient\examples\StreamHttpClient\StreamHttpClient.ino"

// value every 5 min: https://api.coincap.io/v2/assets/bitcoin/history?interval=m5
// value every hour:  https://api.coincap.io/v2/assets/bitcoin/history?interval=h1
// average every day: https://api.coincap.io/v2/assets/bitcoin/history?interval=d1

#define COINCAP_1H_URL  "https://api.coincap.io/v2/assets/bitcoin/history?interval=h1"
#define COINCAP_5M_URL  "https://api.coincap.io/v2/assets/bitcoin/history?interval=m5"


// create static buffer for reading stream from the server
uint8_t buff[3000] = { 0 }; // 3 kB


bool GetDataFromCoinCapServer(bool Refresh_5M) {
  bool result = false;
  unsigned long StartTime = millis();
  bool Timeout = false;
  bool Finished = false;
  unsigned int NoMoreData = 0;
  unsigned int JsonDataSize = 0;
  String COINCAP_URL;

  if (Refresh_5M) {
    CoinCapDataLength_5M = 0;
    COINCAP_URL = COINCAP_5M_URL;
  } else {
    CoinCapDataLength_1H = 0;
    COINCAP_URL = COINCAP_1H_URL;
  }
  
  if (!WiFi.isConnected()) {
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
      DisplayText("HTTPS begin\n");
      if (https.begin(*client, COINCAP_URL)) {  // HTTPS
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
                Serial.printf("[COINCAP] Document size: %d \r\n", DocumentLength);
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
                        // read up to 3000 bytes
                        BytesRead = stream->readBytes(buff, ((StreamAvailable > sizeof(buff)) ? sizeof(buff) : StreamAvailable));
                        JsonDataSize += BytesRead;
                        #ifdef DEBUG_OUTPUT
                        // write it to Serial
                        if (firstBuffer) { Serial.write(buff, BytesRead);  Serial.println(); }
                          else { Serial.print("/"); }
                        #endif
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
                              #ifdef DEBUG_OUTPUT
                              if (Refresh_5M) {
                                if(CoinCapDataLength_5M < 5) { Serial.println(sVal); }
                              } else {
                                if(CoinCapDataLength_1H < 5) { Serial.println(sVal); }
                              }
                              #endif
                              if (Refresh_5M) {
                                CoinCapData_5M[CoinCapDataLength_5M] = sVal.toFloat();
                                CoinCapDataLength_5M++;
                                if (CoinCapDataLength_5M > MAX_DATA_POINTS_5M) {
                                  Serial.println();
                                  Serial.println("MAX NUMBER OF DATA POINTS REACHED!");
                                  DisplayText("\nMax number of data points reached!\n");
                                  Finished = true;
                                  break;
                                }
                              } else {
                                CoinCapData_1H[CoinCapDataLength_1H] = sVal.toFloat();
                                CoinCapDataLength_1H++;
                                if (CoinCapDataLength_1H > MAX_DATA_POINTS_1H) {
                                  Serial.println();
                                  Serial.println("MAX NUMBER OF DATA POINTS REACHED!");
                                  DisplayText("\nMax number of data points reached!\n");
                                  Finished = true;
                                  break;
                                }
                              }
                            } // if data found
                          } // while data found
                          // last section was received; it finishes off with "timestamp"
                          if (sBuf.indexOf("timestamp") >= 0) {
                            Serial.println();
                            Serial.println("End identifier found.");
                            DisplayText("\nEnd identifier found.\n", CLGREEN);
                            Finished = true;
                            }
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
                    // timeout
                    Timeout = (millis() > (StartTime + 20 * 1000));  // 20 seconds
                    if (Timeout || Finished || (NoMoreData > 10)) { break; }
                } // connected or document still has data
                Serial.println();
                if (Timeout) {
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

  if (!result) {
    if (Refresh_5M) {
      CoinCapDataLength_5M = 0;
    } else {
      CoinCapDataLength_1H = 0;
      }
  }
    Serial.print("Time needed (ms): ");
    Serial.println(millis() - StartTime);
    Serial.print("Json data size (bytes): ");
    Serial.println(JsonDataSize);
    return result;
}






bool GetCoinCapData_1H(void) {
    bool result = false;

    if ((millis() < (LastTimeCoinCapRefreshed_1H + 30*60*1000)) && (LastTimeCoinCapRefreshed_1H != 0)) {  // check server every 1/2 hour
      Serial.println("CoinCap data 1H is valid.");
      return true;  // data is already valid
    }

    Serial.println("Requesting data 1H from CoinCap server...");
    DisplayClear();
    DisplayText("Contacting COINCAP server (1H)\n", CLYELLOW);
    if (!GetDataFromCoinCapServer(false)) {
        DisplayText("FAILED!\n", CLRED);
        delay (500);
        return false;
    }
    Serial.println("Number of data points: " + String(CoinCapDataLength_1H));
    char Txt[20];
    sprintf(Txt, "Data points: %u\n", CoinCapDataLength_1H);
    DisplayText(Txt);

    if (CoinCapDataLength_1H > REQ_DATA_POINTS_1H) {
      LastTimeCoinCapRefreshed_1H = millis();
      result = true;
    } else {
      Serial.println("Not enough data points!");
      DisplayText("Not enough data points!\n", CLRED);
      delay (500);
      return false;
    }

    DisplayText("Finished\n", CLGREEN);
    delay (500);
    return result;
}




bool GetCoinCapData_5M(void) {
    bool result = false;

    if ((millis() < (LastTimeCoinCapRefreshed_5M + 5*60*1000)) && (LastTimeCoinCapRefreshed_5M != 0)) {  // check server every 1/2 hour
      Serial.println("CoinCap data 5M is valid.");
      return true;  // data is already valid
    }

    Serial.println("Requesting data 5M from CoinCap server...");
    DisplayClear();
    DisplayText("Contacting COINCAP server (5M)\n", CLYELLOW);
    if (!GetDataFromCoinCapServer(true)) {
        DisplayText("FAILED!\n", CLRED);
        delay (500);
        return false;
    }
    Serial.println("Number of data points: " + String(CoinCapDataLength_5M));
    char Txt[20];
    sprintf(Txt, "Data points: %u\n", CoinCapDataLength_5M);
    DisplayText(Txt);

    if (CoinCapDataLength_5M > REQ_DATA_POINTS_5M) {
      LastTimeCoinCapRefreshed_5M = millis();
      result = true;
    } else {
      Serial.println("Not enough data points!");
      DisplayText("Not enough data points!\n", CLRED);
      delay (500);
      return false;
    }

/*
    Serial.println("------------");
    for (uint16_t i = 0; i < CoinCapDataLength; i++)
    {
      Serial.println(String(CoinCapData[i]));
    }
    Serial.println("------------");
*/    

    DisplayText("Finished\n", CLGREEN);
    delay (500);
    return result;
}










#ifdef DISPLAY_LCD_SPI_Bodmer
  #include <TFT_eSPI.h>

void PlotCoinCapData(const float *DataArray, const int DataLen, const int LineSpacing, const char BgImage) {
  DisplayClear();

  if (DataLen < DspW) {
    Serial.println("BTC: Not enough data to plot!");
    DisplayText("BTC: NOT ENOUGH DATA", 1, 5, 20, CLRED);
    return;
  }

  char FileName[30];
  sprintf(FileName, "/bg_btc_%dx%d_%c.bmp", DspW, DspH, BgImage);
  DisplayShowImage(FileName,   0, 0);

  // vertical line
  int LineX = DspW - LineSpacing;
  String sTxt;
  if (BgImage == 'd') {
    sTxt = "day";
  } else {
    sTxt = "week";
  }
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString(sTxt, LineX + 4, DspH - 22, 1);
  int numDots = DspH / 6;
  for (int i = 0; i < numDots; i++)
  {
    tft.drawFastVLine(LineX, i*6, 3, TFT_DARKGREY);
  }  

  float_t Minn, Maxx;
  Minn =  999999999;
  Maxx = -999999999;
  uint16_t IgnoreFirst = DataLen - DspW;
  Serial.print("All data: ");
  Serial.println(DataLen);
  Serial.print("Ignored first points: ");
  Serial.println(IgnoreFirst);

  for (uint16_t i = IgnoreFirst; i < DataLen; i++)
  {
    if (DataArray[i] > Maxx) {Maxx = DataArray[i];}
    if (DataArray[i] < Minn) {Minn = DataArray[i];}
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
  Serial.printf("Scaling: %f\r\n", Scaling);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  for (X = 0; X < DspW; X++) {
    fY = (DataArray[IgnoreFirst + X] - MinnD) * Scaling;
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
  tft.drawNumber(round(Maxx), 75, 2, 1);
  tft.drawNumber(round(Minn), 75, DspH - 12, 1);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.loadFont(FONT_SIZE_1);
  X = tft.drawNumber(round(DataArray[DataLen-1]), 170, 2);
  tft.drawString("USD", 170 + X + 11, 2);
  tft.unloadFont();
}


void PlotCoinCapData_5M(void) {
  PlotCoinCapData(CoinCapData_5M, CoinCapDataLength_5M, 288, 'd'); // 1 px = 5 min. 288 px = 24 h.
}

void PlotCoinCapData_1H(void) {
  PlotCoinCapData(CoinCapData_1H, CoinCapDataLength_1H, 168, 'w'); // 1 px = 1 h. 168 px = 1 week.
}


#endif  // tft bodmer
