/*
 * Author: Aljaz Ogrin
 * Project: 
 * Hardware: ESP32
 * File description: Global configuration for the ESP32-Roomba project
 */
 
#ifndef __CONFIG_H_
#define __CONFIG_H_

// ************ General config *********************
#define DEVICE_NAME "Display_v1"
#define WIFI_CONNECT_TIMEOUT_SEC 240  // How long to wait for WiFi, then switch to Access Point

#define DISPLAY_LCD_ST7735_Bodmer                             // 128x160 TFT SPI
//#define DISPLAY_LCD_ST7735_Adafruit                         // 128x160 TFT SPI
//#define DISPLAY_OLED_SSD1306                                // 128x64 OLED I2C

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
//#define TFT_BL        25 // GPIO25  Backlight EN - TODO

// ************ BODMER LIBRARY CONFIG *********************

// See SetupX_Template.h for all options available
#define USER_SETUP_ID 2

#define ST7735_DRIVER
// size in portrait configuration
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

// For ST7735, ST7789 and ILI9341 ONLY, define the colour order IF the blue and red are swapped on your display
// Try ONE option at a time to find the correct colour order for your display
//  #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
//  #define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
//#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

//#define SMOOTH_FONT

// #define SPI_FREQUENCY  20000000
#define SPI_FREQUENCY  27000000
// #define SPI_FREQUENCY  40000000

//#define SUPPORT_TRANSACTIONS

//  #define CGRAM_OFFSET      // Library will add offsets required
//  #define TFT_SDA_READ      // Read and write on the MOSI/SDA pin, no separate MISO pin


// For ST7735 ONLY, define the type of display, originally this was based on the
// colour of the tab on the screen protector film but this is not always true, so try
// out the different options below if the screen does not display graphics correctly,
// e.g. colours wrong, mirror images, or stray pixels at the edges.
// Comment out ALL BUT ONE of these options for a ST7735 display driver, save this
// this User_Setup file, then rebuild and upload the sketch to the board again:

// #define ST7735_INITB
// #define ST7735_GREENTAB
// #define ST7735_GREENTAB2
// #define ST7735_GREENTAB3
// #define ST7735_ROBOTLCD       // For some RobotLCD Arduino shields (128x160, BGR, https://docs.arduino.cc/retired/getting-started-guides/TFT)
// #define ST7735_REDTAB
 #define ST7735_BLACKTAB

#define DISABLE_ALL_LIBRARY_WARNINGS
#define USER_SETUP_LOADED

#endif /* __CONFIG_H_ */
