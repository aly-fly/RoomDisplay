/*
 * Author: Aljaz Ogrin
 * Project: 
 * Hardware: ESP32
 * File description: Global configuration for the ESP32-Roomba project
 */
 
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "__CONFIG_SECRETS.h"

// ************ General config *********************
#define DEVICE_NAME "Display_v1"
#define WIFI_CONNECT_TIMEOUT_SEC 240  // How long to wait for WiFi, then switch to Access Point

#define DISPLAY_LCD_ST7735                                    // 128x160 TFT SPI
//#define DISPLAY_OLED_SSD1306                                    // 128x64 OLED I2C

#define TIME_SERVER "pool.ntp.org"
#define GMT_OFFSET  1
#define DST_OFFSET  0

// ************ WiFi config *********************
//#define WIFI_SSID  "..." -> enter into the file __CONFIG_SECRETS.h
//#define WIFI_PASSWD "..."
#define WIFI_RETRY_CONNECTION_SEC  90


// Client
#define CLIENT1_HOST "10.38.44.221" // toplotna
#define CLIENT1_PORT  21212

// ARSO:
#define ARSO_SERVER_XML "https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SI_OSREDNJESLOVENSKA_latest.xml"
// also validate: include\Arso_https_certificate.h

#endif /* __CONFIG_H_ */