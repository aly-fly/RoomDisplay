
#include <Arduino.h>
#include <stdint.h>
#include "__CONFIG.h"
#include "display.h"
#include "myWiFi.h"
#include "TCPclient.h"
#include "ArsoXml.h"

uint16_t ScreenNumber = 0;

void setup() {
  Serial.begin(115200);
  // delay 2 sec on the start to connect from programmer to serial terminal
  int i;
  for (i=0; i<10; i++){
    Serial.print("*");
    delay(200);
  }
  DisplayInit();


  DisplayText("SMALL TEXT", 0, 0, 0);
  delay(1800);
  DisplayText("MEDIUM TEXT", 1, 0, 0);
  delay(1800);
  DisplayText("1234", 2, 0, 0);
  delay(1800);

//  DisplayTest();

  DisplayClear();
  DisplayText("Init...\n");
  WifiInit();

  TCPclientConnect();
    
  delay(2000);
  DisplayClear();
  Serial.println("INIT FINISHED.");
}


void loop() {
  GetARSOdata();

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
    DisplayUpdate();
  } else {
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
    
    ArsoWeather[ScreenNumber-1].Sky.toUpperCase();
    Line = ArsoWeather[ScreenNumber-1].Sky;
    DisplayText(Line.c_str(), 1, 1, 30, CLCYAN);

    ArsoWeather[ScreenNumber-1].Rain.toUpperCase();
    Line = "Dez: " + ArsoWeather[ScreenNumber-1].Rain;
    DisplayText(Line.c_str(), 1, 1, 60, CLGREEN);

    Line = "Temp: " + ArsoWeather[ScreenNumber-1].Temperature + " C";
    DisplayText(Line.c_str(), 1, 1, 90, CLORANGE);
    #endif    
  }
  ScreenNumber++;
  if (ScreenNumber >= 4) ScreenNumber = 0;

  delay(5000);



  WifiReconnectIfNeeded();
} // loop


