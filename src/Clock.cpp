#include <Arduino.h>
#include <stdint.h>
#include <esp_sntp.h>
#include <time.h>
#include "__CONFIG.h"
#include "display.h"
#include "myWiFi.h"

unsigned long LastTimeClockSynced = 0; // data is not valid


// Required for WiFiClientSecure and for checks the validity date of the certificate. 
// Setting clock for CA authorization...
void setClock(void) {

  if (!inHomeLAN) {

    return;    // SNTP configTime does not work. Fortunately HTTPS functions also without the clock.


    sntp_servermode_dhcp(1); //try to get the ntp server from dhcp
    sntp_setservername(1, TIME_SERVER); //fallback server
    sntp_init();

    /*
    test sntp server
    https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino
    */

    // lwIP v2 is able to list more details about the currently configured SNTP servers
    for (int i = 0; i < SNTP_MAX_SERVERS; i++) {
      ip_addr_t sntp = *sntp_getserver(i);
      if (sntp.type == IPADDR_TYPE_V6) {
        Serial.printf("sntp%d:     is IPv6 only", i);
      }

      IPAddress sntpIP = sntp.u_addr.ip4.addr;
      const char* name = sntp_getservername(i);
      if (sntp.u_addr.ip4.addr != 0) {
        Serial.printf("sntp%d:     ", i);
        if (name) {
          Serial.printf("%s (%s) ", name, sntpIP.toString().c_str());
        } else {
          Serial.printf("%s ", sntpIP.toString().c_str());
        }
        Serial.printf("- Reachability: %o\n", sntp_getreachability(i));
      }
    }

  // Serial.println ("Manually setting the internal clock...");
  // https://github.com/fbiego/ESP32Time

      return;    // SNTP configTime does not work
    }


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
