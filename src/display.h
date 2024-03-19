#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "__CONFIG.h"

#ifdef DISPLAY_LCD_ST7735_Adafruit
  #include <Adafruit_ST77xx.h>
  #define CLWHITE   ST7735_WHITE
  #define CLORANGE  ST77XX_ORANGE
  #define CLBLUE    ST77XX_BLUE
  #define CLYELLOW  ST77XX_YELLOW
  #define CLCYAN    ST77XX_CYAN
  #define CLGREEN   ST77XX_GREEN
  #define CLBLACK   ST77XX_BLACK
#endif  
#ifdef DISPLAY_LCD_SPI_Bodmer
  #include <TFT_eSPI.h>
  #define CLWHITE    TFT_WHITE
  #define CLORANGE   TFT_ORANGE
  #define CLRED      TFT_RED
  #define CLGREEN    TFT_GREEN
  #define CLBLUE     TFT_BLUE
  #define CLYELLOW   TFT_YELLOW
  #define CLCYAN     TFT_CYAN
  #define CLGREY     TFT_DARKGREY
  #define CLBLACK    TFT_BLACK
  #define CLDARKBLUE 0x000F         // 0x001F -> 0F

  extern TFT_eSPI tft; // for graphical plot
#endif

  extern signed int DspH;
  extern signed int DspW;

  void DisplayInit(void);
  void DisplaySetBrightness(uint8_t Brightness = 255);
  void DisplayClearCanvas(void);
  void DisplayClear(uint16_t Color = TFT_BLACK);
  void DisplayText(const char Text[]);
  void DisplayText(const char Text[], uint16_t color);
  void DisplayText(const char Text[], uint8_t FontSize, int16_t X, int16_t Y, uint16_t Color = 0xFFFF, bool Show=true);
  void DisplayUpdate(void);

  void DisplayShowImage(const char *filename, int16_t x, int16_t y, int16_t imgScaling = 1);


  void DisplayTest(void);
  void DisplayFontTest(void);



/*
FreeMono12pt7b		FreeSansBoldOblique12pt7b
FreeMono18pt7b		FreeSansBoldOblique18pt7b
FreeMono24pt7b		FreeSansBoldOblique24pt7b
FreeMono9pt7b			FreeSansBoldOblique9pt7b
FreeMonoBold12pt7b		FreeSansOblique12pt7b
FreeMonoBold18pt7b		FreeSansOblique18pt7b
FreeMonoBold24pt7b		FreeSansOblique24pt7b
FreeMonoBold9pt7b		FreeSansOblique9pt7b
FreeMonoBoldOblique12pt7b	FreeSerif12pt7b
FreeMonoBoldOblique18pt7b	FreeSerif18pt7b
FreeMonoBoldOblique24pt7b	FreeSerif24pt7b
FreeMonoBoldOblique9pt7b	FreeSerif9pt7b
FreeMonoOblique12pt7b		FreeSerifBold12pt7b
FreeMonoOblique18pt7b		FreeSerifBold18pt7b
FreeMonoOblique24pt7b		FreeSerifBold24pt7b
FreeMonoOblique9pt7b		FreeSerifBold9pt7b
FreeSans12pt7b		FreeSerifBoldItalic12pt7b
FreeSans18pt7b		FreeSerifBoldItalic18pt7b
FreeSans24pt7b		FreeSerifBoldItalic24pt7b
FreeSans9pt7b			FreeSerifBoldItalic9pt7b
FreeSansBold12pt7b		FreeSerifItalic12pt7b
FreeSansBold18pt7b		FreeSerifItalic18pt7b
FreeSansBold24pt7b		FreeSerifItalic24pt7b
FreeSansBold9pt7b		FreeSerifItalic9pt7b
*/



#endif // __DISPLAY_H_