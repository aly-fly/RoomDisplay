#include "__CONFIG.h"

#ifdef DISPLAY_LCD_ST7735_Bodmer

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "display.h"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
 
  // PWM backlight
  const int freq = 1234;  // 1 kHz
  const int ledChannel = 0;
  const int resolution = 8; 


void DisplaySetBrightness(uint8_t Brightness) {
  // changing the LED brightness with PWM
  ledcWrite(ledChannel, Brightness);
}

void DisplayInit(void) {
  Serial.println("Starting LCD...");
  tft.init();
  tft.setRotation(3);
  DisplayClear();

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(TFT_BL, ledChannel);
  // full power
  DisplaySetBrightness();
}


void DisplayClearCanvas(void) {
    DisplayClear();
}

void DisplayClear(void) {
  tft.setCursor(0, 0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(1);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void DisplayText(const char Text[]) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print(Text);
//  tft.drawString(Text, tft.getCursorX(), tft.getCursorY());
}

void DisplayText(const char Text[], uint16_t color) {
  tft.setTextColor(color, TFT_BLACK);
  tft.print(Text);
//  tft.drawString(Text, tft.getCursorX(), tft.getCursorY());
}

void DisplayText(const char Text[], uint8_t FontSize, int16_t X, int16_t Y, uint16_t Color, bool Show) {
// font size mapping
uint8_t FontSize1;
  switch (FontSize)  {
  case 1:  FontSize1 = 4; // 1 -> Font 4. Medium 26 pixel high font
    break;
  case 2:  FontSize1 = 6; // 2 -> Font 6. Large 48 pixel font
    break;
  default: FontSize1 = 1; // 0 -> Font 1. Original Adafruit 8 pixel font
    break;
  }
  
  tft.setTextColor(Color, TFT_BLACK);
  tft.drawString(Text, X, Y, FontSize1);
}

void DisplayUpdate(void) {}




/*
 Font draw speed and flicker test, draws all numbers 0-999 in each font
 (0-99 in font 8)
 Average time in milliseconds to draw a character is shown in red
 A total of 2890 characters are drawn in each font (190 in font 8)
 
 Needs fonts 2, 4, 6, 7 and 8

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 Note that yield() or delay(0) must be called in long duration for/while
 loops to stop the ESP8266 watchdog triggering.
 */
unsigned long drawTime = 0;

void DisplayTest(void) {

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  drawTime = millis();

  for (int i = 0; i < 1000; i++) {
    tft.drawNumber(i, 0, 0, 1);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 2890.0, 3, 0, 80, 4);
 
  delay(4000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawTime = millis();

  for (int i = 0; i < 1000; i++) {
    tft.drawNumber(i, 0, 0, 2);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 2890.0, 3, 0, 80, 4);

  delay(4000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawTime = millis();

  for (int i = 0; i < 1000; i++) {
    tft.drawNumber(i, 0, 0, 4);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 2890.0, 3, 0, 80, 4);

  delay(4000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawTime = millis();

  for (int i = 0; i < 1000; i++) {
    yield(); tft.drawNumber(i, 0, 0, 6);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 2890.0, 3, 0, 80, 4);
 
  delay(4000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawTime = millis();

  for (int i = 0; i < 1000; i++) {
    yield(); tft.drawNumber(i, 0, 0, 7);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 2890.0, 3, 0, 80, 4);

  delay(4000);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  drawTime = millis();

  for (int i = 0; i < 100; i++) {
    yield(); tft.drawNumber(i, 0, 0, 8);
  }

  drawTime = millis() - drawTime;

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawFloat(drawTime / 190.0, 3, 0, 80, 4);

  delay(4000);
}



#endif // DISPLAY_LCD_ST7735