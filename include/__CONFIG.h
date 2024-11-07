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
#define DEBUG_OUTPUT_DATA  // include received data

#define DEVICE_NAME "Display_v1"
#define WIFI_CONNECT_TIMEOUT_SEC 240  // How long to wait for WiFi

#define DISPLAY_LCD_SPI_Bodmer                                //  TFT SPI
//#define DISPLAY_LCD_ST7735_Adafruit                         // 128x160 TFT SPI
//#define DISPLAY_OLED_SSD1306                                // 128x64 OLED I2C

#define TIME_SERVER  "si.pool.ntp.org"  // "pool.ntp.org"
#define GMT_OFFSET  1
#define DST_OFFSET  0

#define DAY_TIME     7
#define NIGHT_TIME  22

// ************ WiFi config *********************
//#define WIFI_SSID  "..." -> enter into the file __CONFIG_SECRETS.h
//#define WIFI_PASSWD "..."
#define WIFI_RETRY_CONNECTION_SEC  90


// Client 
#define HEATPUMP_HOST "10.38.44.221" // toplotna
#define HEATPUMP_PORT  21212

#define SMOOTHIE_HOST "10.38.11.101" // 3D printer
#define SMOOTHIE_PORT  23

// Shelly
// DOC: https://shelly-api-docs.shelly.cloud/gen1/#shelly-3em-settings-emeter-index
#define SHELLY_3EM_URL "http://10.38.22.111/status"  // Poraba TC
// DOC: https://shelly-api-docs.shelly.cloud/gen2/Addons/ShellySensorAddon#sensoraddongetperipherals-example
// DOC: https://shelly-api-docs.shelly.cloud/gen2/ComponentsAndServices/Temperature#temperaturegetstatus-example
#define SHELLY_1PM_ADDON_URL "http://10.38.22.112/rpc/Temperature.GetStatus?id=101" // Bazen temperatura
// DOC: https://shelly-api-docs.shelly.cloud/gen2/ComponentsAndServices/Switch#switchgetstatus-example
#define SHELLY_1PM_SW1_URL "http://10.38.22.112/rpc/Switch.GetStatus?id=0" // Bazen pumpa
#define SHELLY_1PM_SW2_URL "http://10.38.22.113/rpc/Switch.GetStatus?id=0" // Bazen ogrevanje

// ARSO:
#define ARSO_SERVER_CURRENT_XML_URL   "https://meteo.arso.gov.si/uploads/probase/www/observ/surface/text/sl/observation_LJUBL-ANA_BRNIK_latest.xml"
#define ARSO_SERVER_FORECAST_XML_URL  "https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/fcast_SI_OSREDNJESLOVENSKA_latest.xml"
#define ARSO_SERVER_METEOGRAM_XML_URL "https://meteo.arso.gov.si/uploads/probase/www/fproduct/text/sl/forecast_SI_OSREDNJESLOVENSKA_int3h.xml"
#define MTG_NUMPTS 25
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

//#define BOARD_CYD

#ifdef BOARD_CYD
  #define ILI9341_2_DRIVER  // 2.8 inch LCD 320 x 240 (*CYD*) - Alternative ILI9341 driver, see https://github.com/Bodmer/TFT_eSPI/issues/1172

  #define SD_MOSI       23
  #define SD_MISO       19
  #define SD_SCK        18
  #define SD_CS          5

  #define IMAGES_ON_SD_CARD

  #define LDR_PINx      34  
#else

  //#define ST7789_DRIVER   // 2.8 inch LCD  320 x 240 (firma)
  //#define ILI9341_DRIVER  // 2.8 inch LCD  320 x 240 (doma)
  //#define ST7796_DRIVER   // 4 inch LCD

  #ifndef ST7789_DRIVER // defined in ENV settings
    #define ILI9341_2_DRIVER  // 2.8 inch LCD 320 x 240 (doma & CYD) - Alternative ILI9341 driver, see https://github.com/Bodmer/TFT_eSPI/issues/1172
  #endif

#endif

  // common TFT pins

  #define USE_HSPI_PORT  // for TFT display;  VSPI is used for SD card

  #define TFT_BL        21
  #define TFT_RST       -1 // connect to RESET pin / ESP32 EN pin
  #define TFT_DC         2
  
#ifdef FREE_JTAG_PINS
  #define TFT_CS        0
  #define TFT_SCLK      0
  #define TFT_MISO      0
  #define TFT_MOSI      0
#else
  #define TFT_CS        15  // JTAG !!
  #define TFT_SCLK      14  // JTAG !!
  #define TFT_MISO      12  // JTAG !!
  #define TFT_MOSI      13  // JTAG !!
#endif

// ************ BODMER LIBRARY CONFIG *********************

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

#define SPI_FREQUENCY  20000000

#define DISABLE_ALL_LIBRARY_WARNINGS
#define USER_SETUP_LOADED

// ************ SD CARD CONFIG *********************
#define SPI_FREQ_SDCARD 20000000  // default 4 MHz; max 25 MHz
#define SPI_DMA_MAX 4095 

#endif /* __CONFIG_H_ */
