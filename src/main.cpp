
#include <Arduino.h>
#include <stdint.h>
#include <SPIFFS.h>
#include "Version.h"
#include "__CONFIG.h"
#include "display.h"
#include "myWiFi.h"
#include "Clock.h"
#include "TCPclient.h"
#include "ArsoXml.h"
#include "ArsoPlotMeteogram.h"
#include "ShellyHttpClient.h"
#include "CaptivePortalLogin.h"
#include "CoinCapAPI.h"
#include "myPing.h"

uint16_t ScreenNumber = 0;
int Hour;
bool NightMode = false;
uint16_t LDRvalue;
String TempOutdoor1, TempOutdoor2;
bool ok;

void setup() {
  Serial.begin(115200);
  // delay 2 sec on the start to connect from programmer to serial terminal
  int i;
  for (i=0; i<10; i++){
    Serial.print("*");
    delay(100);
  }
  Serial.println();

  Serial.println("Project: github.com/aly-fly/RoomDisplay");
  Serial.print("Version: ");
  Serial.println(VERSION);
  Serial.print("Build: ");
  Serial.println(BUILD_TIMESTAMP);

  DisplayInit();
//      DisplayTest();
//      DisplayFontTest();
  DisplayClear();

  DisplayText("Init...\n", CLYELLOW);
  DisplayText("Project: github.com/aly-fly/RoomDisplay\n", CLWHITE);
  DisplayText("Version: ", CLWHITE);
  DisplayText(VERSION, CLCYAN);
  DisplayText("\n", CLWHITE);
  DisplayText("Build: ", CLWHITE);
  DisplayText(BUILD_TIMESTAMP, CLCYAN);
  DisplayText("\n", CLWHITE);

#ifdef LDR_PIN
  pinMode(LDR_PIN, ANALOG);
  adcAttachPin(LDR_PIN);
  analogSetAttenuation(ADC_0db);
#endif

  DisplayText("SPIFFS start...");
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    DisplayText("FAILED!\n", CLRED);
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS available!");
  DisplayText("OK\n", CLGREEN);

  WifiInit();

  if (!inHomeLAN) {
   bool connOk = CheckConnectivityAndHandleCaptivePortalLogin();
   if (!connOk) {
      Serial.println("=== REBOOT ===");
      DisplayText("=== REBOOT ===\n", CLORANGE);
      delay (30000);
      ESP.restart();  // retry everything from the beginning
   }
  }

  String sPingIP;
  IPAddress pingIP;
  sPingIP = "216.58.205.46"; // google.com
  pingIP.fromString(sPingIP);
  Serial.println(sPingIP);
  ppiinngg(pingIP, true);

  setClock(); 

  if (inHomeLAN) {
    TCPclientConnect();
  } else {
    ScreenNumber = 1;   // skip heat pump display
  }

  DisplayInitFonts();

  DisplayText("Init finished.", CLGREEN)    ;
  delay(2000);
  DisplayClear();
  Serial.println("INIT FINISHED.");
}


// ===============================================================================================================================================================

void loop() {
  if (inHomeLAN) {
    if(CurrentHour(Hour)) {
      Serial.println("Hour: " + String(Hour));
      NightMode = ((Hour >= NIGHT_TIME) || (Hour < DAY_TIME));
    } else {
      Serial.println("Getting current time failed!");
      NightMode = false;
    }
  }

#ifdef LDR_PIN
  LDRvalue = analogRead(LDR_PIN); //     0 is full brightness, higher values == darker; Default is 12 bits (range from 0 to 4096).
  Serial.print("LDR: ");
  Serial.println(LDRvalue);

  DisplayText(String (LDRvalue).c_str());
  DisplayText("\n");

  delay(500);
  return;

#else
  if (NightMode) {
    DisplaySetBrightness(30);
  } else { // lower brightness at night
    DisplaySetBrightness(); // full power
  }
#endif

  
  //  HEAT PUMP DATA
  if (ScreenNumber == 0) {  // -------------------------------------------------------------------------------------------------------------------------
    TempOutdoor1 = "- - -";
    TempOutdoor2 = "- - -";
    char ShellyTxt[10];
    sprintf(ShellyTxt, "---");

    if (inHomeLAN) {
      if (TCPclientRequest("Outdoor HP")) {
        TCPresponse.replace(",", ".");
        // remove trailing "0"
        TCPresponse.remove(TCPresponse.indexOf(".")+2);
        TCPresponse.concat(" C");
        TempOutdoor1 = TCPresponse;
      }
      if (TCPclientRequest("Outdoor")) {
        TCPresponse.replace(",", ".");
        // remove trailing "0"
        TCPresponse.remove(TCPresponse.indexOf(".")+2);
        TCPresponse.concat(" C");
        TempOutdoor2 = TCPresponse;
      }
      if (ShellyGetData()) {
        //DisplayText(sTotalPower.c_str(), 1, 60, 107, CLRED, false);
        sprintf(ShellyTxt, "%.2f kW", TotalPower/1000);
      }
    } // inHomeLAN

    DisplayClear(CLBLACK);
    char FileName[30];
    sprintf(FileName, "/bg_grass_%dx%d.bmp", DspW, DspH);
    DisplayShowImage(FileName,   0, 0);
    DisplayText("Temperatura pred hiso", 1,  20,   8, CLWHITE);
    DisplayText(TempOutdoor1.c_str(),    2,  92,  46, CLBLACK); // shadow
    DisplayText(TempOutdoor1.c_str(),    2,  90,  44, CLORANGE);
    DisplayText(TempOutdoor2.c_str(),    2,  92, 102, CLBLACK); // shadow
    DisplayText(TempOutdoor2.c_str(),    2,  90, 100, CLCYAN);
    DisplayText(ShellyTxt,               1, 102, 162, CLBLACK); // shadow
    DisplayText(ShellyTxt,               1, 100, 160, CLRED);

    DisplayText(SunRiseTime.c_str(), 1,   5+3,       240-45-26-3+3, CLBLACK);  // shadow
    DisplayText(SunRiseTime.c_str(), 1,   5,         240-45-26-3,   CLYELLOW);
    DisplayText(SunSetTime.c_str(),  1,  320 - 75+3, 240-45-26-3+3, CLBLACK);  // shadow
    DisplayText(SunSetTime.c_str(),  1,  320 - 75,   240-45-26-3,   CLYELLOW);

    DisplayShowImage("/sunrise.bmp",  0,      240-45);
    DisplayShowImage("/sunset.bmp",   320-53, 240-45);
    delay(6000);
  }

// WEATHER FORECAST  
  if (ScreenNumber == 1) {  // -------------------------------------------------------------------------------------------------------------------------    
    ok = GetARSOdata();

    String Line;
    
    DisplayClear(CLWHITE);

    char FileName[30];
    sprintf(FileName, "/bg_sky_%dx%d.bmp", DspW, DspH);
    DisplayShowImage(FileName,   0, 0);

    // zgoraj - dnevi
    Line = ArsoWeather[0].DayName;
    DisplayText(Line.c_str(), 1,   10, 10, CLDARKBLUE);

    Line = ArsoWeather[2].DayName;
    DisplayText(Line.c_str(), 1, 180, 10, CLDARKBLUE);


    // na sredini so slikce (original 32x32, povečane x2)
    String FN;
    FN = "/w/" + ArsoWeather[0].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),   6, 44, 2);

    FN = "/w/" + ArsoWeather[1].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  82, 44, 2);

    FN = "/w/" + ArsoWeather[2].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  174, 44, 2);

    FN = "/w/" + ArsoWeather[3].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  249, 44, 2);

    // wind data
    DisplayText(ArsoWeather[0].WindIcon.c_str(), 0,   6, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[1].WindIcon.c_str(), 0,  82, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[2].WindIcon.c_str(), 0, 174, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[3].WindIcon.c_str(), 0, 249, 111, CLDARKBLUE);    

    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,   44+2, 175+2, CLGREY); // shadow
    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,   44,   175,   CLBLUE);
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,   44+2, 132+2, CLGREY); // shadow
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,   44,   132,   CLRED);
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  201+2, 175+2, CLGREY); // shadow
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  201,   175,   CLBLUE);
    DisplayText(ArsoWeather[3].Temperature.c_str(), 2,  201+2, 132+2, CLGREY); // shadow
    DisplayText(ArsoWeather[3].Temperature.c_str(), 2,  201,   132,   CLRED);

    DisplayText(SunRiseTime.c_str(), 1,    5,        240-23,   CLDARKGREEN);
    DisplayText(SunSetTime.c_str(),  1,  320 - 80,   240-23,   CLDARKGREEN);

    if (ok) delay(8000);
  }

  // Arso meteogram
  if (ScreenNumber == 2) {  // -------------------------------------------------------------------------------------------------------------------------
    ok = GetARSOmeteogram();
    if (ok) ArsoPlotMeteogram();
    if (ok) delay(15000);
  }

  // COIN CAP DATA PLOT
  if (ScreenNumber == 3) {  // -------------------------------------------------------------------------------------------------------------------------
    ok = GetCoinCapData_1H();
    PlotCoinCapData_1H();
    if (ok) delay(5000);
  }

  // COIN CAP DATA PLOT
  if (ScreenNumber == 4) {  // -------------------------------------------------------------------------------------------------------------------------
    ok = GetCoinCapData_5M();
    PlotCoinCapData_5M();
    if (ok) delay(3000);
  }


  ScreenNumber++;
  if (ScreenNumber >= 5) { // housekeeping at the end of display cycles
    if (inHomeLAN)
      ScreenNumber = 0; else
      ScreenNumber = 1;   // skip heat pump

    WifiReconnectIfNeeded();

    // check connectivity
    if (!inHomeLAN) {
      //DisplayClear();
      //DisplayText("Test connectivity...\n", 0, 0, 10, CLGREY);
      String sPingIP;
      IPAddress pingIP;
      sPingIP = "216.58.205.46"; // google.com
      pingIP.fromString(sPingIP);
      Serial.println(sPingIP);
      bool PingOK = ppiinngg(pingIP, false);
      if (!PingOK) {
        DisplayClear();
        DisplayText("Test connectivity FAILED\n", CLORANGE);

        bool connOk = CheckConnectivityAndHandleCaptivePortalLogin();
        if (!connOk){
            Serial.println("=== REBOOT ===");
            DisplayText("=== REBOOT ===\n", CLORANGE);
            delay (10000);
            ESP.restart();  // retry everything from the beginning
        }
      }
    }

    // debug
    Serial.println("[IDLE] Free memory: " + String(esp_get_free_heap_size()) + " bytes");

    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // internal RAM, memory capable to store data or to create new task
    /*
    info.total_free_bytes;   // total currently free in all non-continues blocks
    info.minimum_free_bytes;  // minimum free ever
    info.largest_free_block;   // largest continues block to allocate big array
    */
    Serial.println("[IDLE] Largest available block: " + String(info.largest_free_block) + " bytes");
    Serial.println("[IDLE] Minimum free ever: " + String(info.minimum_free_bytes) + " bytes");
  }

  delay(1000);

} // loop


