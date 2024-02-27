#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <Adafruit_ST77xx.h>

  void DisplayInit(void);
  void DisplayClearCanvas(void);
  void DisplayClear(void);
  void DisplayText(const char Text[]);
//void DisplayText(const String &Text, uint8_t FontSize, int16_t X, int16_t Y);  
  void DisplayText(const char Text[], uint8_t FontSize, int16_t X, int16_t Y, uint16_t Color = ST77XX_WHITE, bool Show=true);
  void DisplayUpdate(void);


  void DisplayTest(void);



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