/*
 * Author: Aljaz Ogrin
 * Project: 
 * Hardware: ESP32
 * File description: Global configuration for the ESP32-Roomba project
 */
 
#ifndef __CONFIG_H_
#define __CONFIG_H_

// ************ General config *********************
#define DEBUG_OUTPUT

#define DEVICE_NAME "Display_v1"
//#define WIFI_CONNECT_TIMEOUT_SEC 240  // How long to wait for WiFi

#define DISPLAY_LCD_SPI_Bodmer                                //  TFT SPI
//#define DISPLAY_LCD_ST7735_Adafruit                         // 128x160 TFT SPI
//#define DISPLAY_OLED_SSD1306                                // 128x64 OLED I2C

#define TIME_SERVER  "si.pool.ntp.org"  // "pool.ntp.org"
#define GMT_OFFSET  1
#define DST_OFFSET  0

// ************ WiFi config *********************
//#define WIFI_SSID  "..." -> enter into the file __CONFIG_SECRETS.h
//#define WIFI_PASSWD "..."
#define WIFI_RETRY_CONNECTION_SEC  90


// Client 
#define CLIENT1_HOST "10.38.44.221" // toplotna
#define CLIENT1_PORT  21212

// Shelly
#define SHELLY_3EM_HOST "10.38.22.111" // Poraba TC

// ARSO:
#define ARSO_SERVER_CURRENT_XML_URL "https://meteo.arso.gov.si/uploads/probase/www/observ/surface/text/sl/observation_LJUBL-ANA_BRNIK_latest.xml"
#define ARSO_SERVER_FORECAST_XML_URL "https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SI_OSREDNJESLOVENSKA_latest.xml"
// also validate: include\Arso_https_certificate.h


// ************ Hardware *********************

// črna plata
// Start I2C Communication SDA = 5 and SCL = 4 on Wemos Lolin32 ESP32 with built-in SSD1306 OLED
//#define OLED_SDA  5
//#define OLED_SCL  4

// bela plata
// Start I2C Communication SDA = 4 and SCL = 15
//#define OLED_SDA  4
//#define OLED_SCL  15


  #define TFT_RST       -1 // Or set to -1 and connect to RESET pin / ESP32 EN pin
  #define TFT_DC         4 // GPIO04 
  #define TFT_CS         5 // VSPI CS0 (GPIO05)
  #define TFT_SCK       18 // VSPI SCK (GPIO18)
  #define TFT_MISO      19 // VSPI Q   (GPIO19)
  #define TFT_MOSI      23 // VSPI D   (GPIO23)
  #define TFT_BL        25 // GPIO25  Backlight EN

// ************ BODMER LIBRARY CONFIG *********************

#define ST7789_DRIVER  // 2.8 inch LCD  320 x 240
//#define ILI9341_DRIVER  // 2.8 inch LCD
//#define ST7796_DRIVER  // 4 inch LCD


#ifdef ST7789_DRIVER
  #define TFT_INVERSION_OFF
  #define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red
#endif



#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
//#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
//#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
//#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
//#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
//#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
//#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT
//  FONT_FS_AVAILABLE    // while examining the library, looks like this should be enabled because fonts are in the file system, but it crashes with this enabled..

#define SPI_FREQUENCY  40000000

#define DISABLE_ALL_LIBRARY_WARNINGS
#define USER_SETUP_LOADED

#endif /* __CONFIG_H_ */
