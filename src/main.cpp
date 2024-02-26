
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
  /*
  DisplayText("SMALL TEXT", 1, 0, 0);
  delay(1800);
  DisplayText("BIG", 2, 0, 0);
  delay(1800);
  DisplayTest();
  */
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
      DisplayText(TCPresponse.c_str(), 2, 6, 5, false);
    }
    if (TCPclientRequest("Outdoor HP")) {
      DisplayText(TCPresponse.c_str(), 1, 6, 50, false);
    }
    DisplayUpdate();
  } else {
    DisplayClearCanvas();
    DisplayText(ArsoWeather[ScreenNumber-1].Day.c_str());
    DisplayText(" ");
    DisplayText(ArsoWeather[ScreenNumber-1].PartOfDay.c_str());
    DisplayText("\nNebo: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Sky.c_str());
    DisplayText("\nDez: ");
    DisplayText(ArsoWeather[ScreenNumber-1].Rain.c_str());
    DisplayText("\nTemperatura:");
    DisplayText(ArsoWeather[ScreenNumber-1].Temperature.c_str());
    DisplayText("\n");
  }
  ScreenNumber++;
  if (ScreenNumber >= 4) ScreenNumber = 0;

  delay(5000);



  WifiReconnectIfNeeded();
} // loop


