
#include <Arduino.h>
#include <stdint.h>
#include <SPIFFS.h>
#include "__CONFIG.h"
#include "display.h"
#include "myWiFi.h"
#include "Clock.h"
#include "TCPclient.h"
#include "ArsoXml.h"
#include "ShellyHttpClient.h"
#include "CaptivePortalLogin.h"
#include "CoinCapAPI.h"
#include "myPing.h"


uint16_t ScreenNumber = 0;
int Hour;
bool NightMode;
uint16_t ArsoBgColor = CLWHITE;
String TempOutdoor1, TempOutdoor2;

void setup() {
  Serial.begin(115200);
  // delay 2 sec on the start to connect from programmer to serial terminal
  int i;
  for (i=0; i<10; i++){
    Serial.print("*");
    delay(100);
  }
  Serial.println();
//  DisplayInit();
//      DisplayTest();
//      DisplayFontTest();
  DisplayClear();
  DisplayText("Init...\n", CLYELLOW);

  DisplayText("SPIFFS start...");
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    DisplayText("FAILED!\n");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS available!");

  WifiInit();

  if (!inHomeLAN) {
   bool connOk = CheckConnectivityAndHandleCaptivePortalLogin();
   if (!connOk){
      Serial.println("=== HALT ===");   
      while (1) {}
   }
  }


  //uint32_t pingIP = (216 << 24) | (58 << 16) | (205 << 8) | (46 << 0);  // google.com [216.58.205.46]
  String sPingIP;
  IPAddress pingIP;
/*
  sPingIP = "216.58.205.46"; // google.com
  pingIP.fromString(sPingIP);
  Serial.println(sPingIP);
  ppiinngg(pingIP);
*/
  sPingIP = TIME_SERVER;
  pingIP.fromString(sPingIP);
  Serial.println(sPingIP);
  ppiinngg(pingIP);


  setClock(); 

  if (inHomeLAN) {
    TCPclientConnect();
  }

  DisplayText("Init finished.", CLGREEN)    ;
  delay(2000);
  DisplayClear();
  Serial.println("INIT FINISHED.");
}


void loop() {
  if(CurrentHour(Hour)) {
    Serial.println("Hour: " + String(Hour));
    NightMode = ((Hour > 22) || (Hour < 7));
  } else {
    Serial.println("Getting current time failed!");
    NightMode = false;
  }
  
  //  HEAT PUMP DATA
  if (ScreenNumber == 0) {  // -------------------------------------------------------------------------------------------------------------------------
    TempOutdoor1 = "- - -";
    TempOutdoor2 = "- - -";
    char ShellyTxt[10];
    printf(ShellyTxt, "---");

    if (inHomeLAN) {
      if (TCPclientRequest("Outdoor")) {
        TCPresponse.replace(",", ".");
        // remove trailing "0"
        TCPresponse.remove(TCPresponse.indexOf(".")+2);
        TCPresponse.concat(" C");
        TempOutdoor1 = TCPresponse;
      }
      if (TCPclientRequest("Outdoor HP")) {
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
    DisplayShowImage("/bg_grass.bmp",    0, 0);
    DisplayText("Temperatura pred hiso", 0, 10,   2, CLWHITE);
    DisplayText(TempOutdoor1.c_str(),    2, 32,  22, CLBLACK); // shadow
    DisplayText(TempOutdoor1.c_str(),    2, 30,  20, CLORANGE);
    DisplayText(TempOutdoor2.c_str(),    1, 42,  72, CLBLACK); // shadow
    DisplayText(TempOutdoor2.c_str(),    1, 40,  70, CLCYAN);
    DisplayText(ShellyTxt,               1, 62, 107, CLBLACK); // shadow
    DisplayText(ShellyTxt,               1, 60, 105, CLRED);
  }

// WEATHER FORECAST  
  if (ScreenNumber == 1) {  // -------------------------------------------------------------------------------------------------------------------------
    GetARSOdata();

    String Line;
    
    #ifdef DISPLAY_OLED_SSD1306
    DisplayClearCanvas();
    Line = ArsoWeather[ScreenNumber-1].Day + " " + ArsoWeather[ScreenNumber-1].PartOfDay; 
    DisplayText(Line.c_str());
    DisplayText("\nNebo: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Sky.c_str());
    DisplayText("\nDez: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Rain.c_str());
    DisplayText("\nTemperatura:");
    DisplayText(ArsoWeather[ScreenNumber-1].Temperature.c_str());
    DisplayText("\n");
    DisplayUpdate();
    #else
    /*
    Line = ArsoWeather[ScreenNumber-1].Day + " " + ArsoWeather[ScreenNumber-1].PartOfDay; 
    DisplayText(Line.c_str(), 1, 1, 00, CLYELLOW);
    
    //ArsoWeather[ScreenNumber-1].Sky.toUpperCase();
    Line = ArsoWeather[ScreenNumber-1].Sky;
    DisplayText(Line.c_str(), 1, 1, 30, CLCYAN);

    ArsoWeather[ScreenNumber-1].Rain.toUpperCase();
    Line = "Dez: " + ArsoWeather[ScreenNumber-1].Rain;
    DisplayText(Line.c_str(), 1, 1, 60, CLGREEN);

    Line = "Temp: " + ArsoWeather[ScreenNumber-1].Temperature + " C";
    DisplayText(Line.c_str(), 1, 1, 90, CLORANGE);
    */


    // grafika
    DisplayClear(ArsoBgColor);

    // zgoraj - dnevi
    Line = ArsoWeather[0].Day;
    DisplayText(Line.c_str(), 0, 6, 3, CLDARKBLUE);

//    Line = ArsoWeather[1].Day + ArsoWeather[1].PartOfDay; 
//    DisplayText(Line.c_str(), 0, 56, 1, CLDARKBLUE);

    Line = ArsoWeather[2].Day; // + ArsoWeather[2].PartOfDay; 
    DisplayText(Line.c_str(), 0, 93, 3, CLDARKBLUE);


    // sredina slike
    String FN;
    FN = "/w/" + ArsoWeather[0].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),   21, 12);

    FN = "/w/" + ArsoWeather[1].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  88, 12);

    FN = "/w/" + ArsoWeather[2].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(), 124, 12);

    // spodaj temperatura
    // remove trailing ".x C"
    int p = TempOutdoor1.indexOf(".");
    if (p > -1) { TempOutdoor1.remove(p); }

    DisplayText(TempOutdoor1.c_str(),                2,  12, 93, CLGREY); // shadow
    DisplayText(TempOutdoor1.c_str(),                2,  10, 91, CLBLUE);
    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,  12, 52, CLGREY); // shadow
    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,  10, 50, CLRED);
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,  99, 93, CLGREY); // shadow
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,  97, 91, CLBLUE);
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  99, 52, CLGREY); // shadow
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  97, 50, CLRED);
/*
    // na tleh ikonca
    DisplayText(ArsoWeather[0].WeatherIcon.c_str(), 0,   0, 119, CLGREY);
    DisplayText(ArsoWeather[1].WeatherIcon.c_str(), 0,  50, 110, CLGREY);
    DisplayText(ArsoWeather[2].WeatherIcon.c_str(), 0,  90, 119, CLGREY);
*/
    #endif    
    delay(4000); // additional delay
  }
  // COIN CAP DATA PLOT
  if (ScreenNumber == 2) {  // -------------------------------------------------------------------------------------------------------------------------
    GetCoinCapData();
    PlotCoinCapData();
    delay(4000); // additional delay
  }


  ScreenNumber++;
  if (ScreenNumber >= 3) ScreenNumber = 0;

  if (!NightMode) {
    ArsoBgColor = CLWHITE;
    DisplaySetBrightness(); // full power
  } else { // fix temperature only with lower brightness at night
    ArsoBgColor = CLBLACK;
    DisplaySetBrightness(30);
  }

  delay(6000);



  WifiReconnectIfNeeded();

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
} // loop


