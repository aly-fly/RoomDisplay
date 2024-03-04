#include <Arduino.h>
#include <stdint.h>
#include <time.h>
#include "__CONFIG.h"
#include "display.h"

unsigned long LastTimeClockSynced = 0; // data is not valid


// Required for WiFiClientSecure and for checks the validity date of the certificate. 
// Setting clock for CA authorization...
void setClock(void) {
    if ((millis() < (LastTimeClockSynced + 60*60*1000)) && (LastTimeClockSynced != 0)) {  // check every hour
      return;  // clock is already synced
    }

  configTime(GMT_OFFSET*60*60, DST_OFFSET*60*60, TIME_SERVER);

  Serial.print("NTP time sync");
  DisplayText ("NTP time sync");
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    DisplayText (".");
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  DisplayText ("\n");
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  DisplayText(asctime(&timeinfo), CLCYAN);
  
  LastTimeClockSynced = millis();
}

bool CurrentHour(int &Hour) {
  struct tm timeinfo;
  bool result = getLocalTime(&timeinfo);
  Hour = timeinfo.tm_hour;
  return result;
}
