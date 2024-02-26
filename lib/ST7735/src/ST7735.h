//**************************************************************************************************
// ST7735.h                                                                                       *
//**************************************************************************************************
// Separated from the main sketch to allow several display types.                                  *
// Includes for various ST7735 displays.  Size is 160 x 128.  Select INITR_BLACKTAB                *
// for this and set dsp_getwidth() to 160.                                                         *
// Works also for the 128 x 128 version.  Select INITR_144GREENTAB for this and                    *
// set dsp_getwidth() to 128.                                                                      *
//**************************************************************************************************
#ifndef ST7735_H
#define ST7735_H
#include <Adafruit_ST7735.h>


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
#include "FreeSans9pt7b_a.h"
#define Font2Name FreeSans9pt7b_a
#define Font2YcursorShift 12  // for details see: https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts   search "baseline"

#define TIMEPOS     -52                         // Position (column) of time in topline relative to end
#define INIPARS     ini_block.tft_cs_pin, ini_block.tft_dc_pin  // Prameters for dsp_begin
#define DISPLAYTYPE "BLUETFT"

// Color definitions for the TFT screen (if used)
// TFT has bits 6 bits (0..5) for RED, 6 bits (6..11) for GREEN and 4 bits (12..15) for BLUE.
#define BLACK   ST7735_BLACK
#define BLUE    ST7735_BLUE
#define RED     ST7735_RED
#define GREEN   ST7735_GREEN
#define CYAN    GREEN | BLUE
#define MAGENTA RED | BLUE
#define YELLOW  RED | GREEN
#define WHITE   RED | BLUE | GREEN
#define GREY    0x7bf0

#define DEFTXTSIZ  1                                  // Default text size


struct scrseg_struct                                  // For screen segments
{
  bool     update_req ;                               // Request update of screen
  uint16_t color ;                                    // Textcolor
  uint16_t size ;                                     // Text size
  uint16_t y ;                                        // Begin of segment row
  uint16_t height ;                                   // Height of segment
  uint16_t str_max_len ;                              // Max chars in a segment
  String   str ;                                      // String to be displayed
} ;


// Data to display.  There are TFTSECS sections
#define TFTSECS 5

#define tftdata             bluetft_tftdata
#define displaybattery      bluetft_displaybattery
#define displayvolume       bluetft_displayvolume
#define displaytime         bluetft_displaytime

extern Adafruit_ST7735*     bluetft_tft ;                                 // For instance of display driver

void SetFont(uint8_t Size);
void SetCursor(int16_t x, int16_t y);

// Various macro's to mimic the ST7735 version of display functions
#define dsp_setRotation()       bluetft_tft->setRotation ( 3 )            // Use landscape format (3 for upside down)
#define dsp_print(a)            bluetft_tft->print ( a )                  // Print a string 
#define dsp_println(b)          bluetft_tft->println ( b )                // Print a string followed by newline 
#define dsp_fillRect(a,b,c,d,e) bluetft_tft->fillRect ( a, b, c, d, e ) ; // Fill a rectange
//#define dsp_setTextSize(a)      bluetft_tft->setTextSize(a)               // Set the text size
//#define dsp_setCursor(a,b)      bluetft_tft->setCursor ( a, b )           // Position the cursor
#define dsp_setTextSize(a)      SetFont(a)                         // Change font (text size)
#define dsp_setCursor(a,b)      SetCursor ( a, b )                 // Position the cursor
#define dsp_setTextColor(a)     bluetft_tft->setTextColor(a)              // Set the text color
#define dsp_erase()             bluetft_tft->fillScreen ( BLACK ) ;       // Clear the screen
#define dsp_getwidth()          160                                       // Adjust to your display
#define dsp_getheight()         128                                       // Get height of screen
#define dsp_update(a)                                                     // Updates to the physical screen
#define dsp_begin               bluetft_dsp_begin                         // Init driver

extern scrseg_struct     bluetft_tftdata[TFTSECS] ;                       // Screen divided in segments

//void bluetft_dsp_fillRect   ( int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color ) ;
void bluetft_displaybattery ( uint16_t bat0, uint16_t bat100, uint16_t adcval ) ;
void bluetft_displayvolume  ( uint8_t vol ) ;
void bluetft_displaytime    ( const char* str, uint16_t color = 0xFFFF ) ;
bool bluetft_dsp_begin      ( int8_t cs, int8_t dc ) ;
#endif
