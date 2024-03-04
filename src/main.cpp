
#include <Arduino.h>
#include <stdint.h>
#include "__CONFIG.h"
#include "display.h"
#include "myWiFi.h"
#include "Clock.h"
#include "TCPclient.h"
#include "ArsoXml.h"
#include "ShellyHttpClient.h"
#include "CaptivePortalLogin.h"
#include "CoinCapAPI.h"

uint16_t ScreenNumber = 0;
int Hour;
bool NightMode;

void setup() {
  Serial.begin(115200);
  // delay 2 sec on the start to connect from programmer to serial terminal
  int i;
  for (i=0; i<10; i++){
    Serial.print("*");
    delay(100);
  }
  Serial.println();
  DisplayInit();

/*
  DisplayText("SMALL TEXT", 0, 0, 0);
  delay(1800);
  DisplayText("MEDIUM TEXT", 1, 0, 0);
  delay(1800);
  DisplayText("1234", 2, 0, 0);
  delay(1800);
*/
//  DisplayTest();

  DisplayClear();
  DisplayText("Init...\n", CLYELLOW);
  WifiInit();

//  HandleCaptivePortalLogin();

  setClock(); 

  TCPclientConnect();

  DisplayText("Init finished.", CLYELLOW)    ;
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
  if (ScreenNumber == 0) {
    DisplayClearCanvas();
    if (TCPclientRequest("Outdoor")) {
      TCPresponse.replace(",", ".");
      DisplayText(TCPresponse.c_str(), 2, 6, 5, CLORANGE, false);
    }
    if (TCPclientRequest("Outdoor HP")) {
      TCPresponse.replace(",", ".");
      DisplayText(TCPresponse.c_str(), 1, 6, 70, CLBLUE, false);
    }
    if (ShellyGetData()) {
//      DisplayText(sTotalPower.c_str(), 1, 60, 100, CLRED, false);
      char Txt[10];
      sprintf(Txt, "%.2f kW", TotalPower/1000);
      DisplayText(Txt, 1, 60, 107, CLRED, false);
    }

    DisplayUpdate();
  }
  // COIN CAP DATA PLOT
  else if (ScreenNumber == 4) {
    GetCoinCapData();
    PlotCoinCapData();
    delay(5000); // additional delay
}
// WEATHER FORECAST  
  else { // 1..3
    GetARSOdata();

    DisplayClearCanvas();
    String Line;
    
    #ifdef DISPLAY_OLED_SSD1306
    Line = ArsoWeather[ScreenNumber-1].Day + " " + ArsoWeather[ScreenNumber-1].PartOfDay; 
    DisplayText(Line.c_str());
    DisplayText("\nNebo: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Sky.c_str());
    DisplayText("\nDez: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Rain.c_str());
    DisplayText("\nTemperatura:");
    DisplayText(ArsoWeather[ScreenNumber-1].Temperature.c_str());
    DisplayText("\n");
    #else
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
    #endif    
  }

  if (!NightMode) {
    ScreenNumber++;
    if (ScreenNumber >= 5) ScreenNumber = 0;
    DisplaySetBrightness(); // full power
  } else { // fix temperature only with lower brightness at night
    ScreenNumber = 0;
    DisplaySetBrightness(30);
  }

  delay(5000);



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


