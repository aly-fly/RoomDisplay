#ifndef __DISPLAY_H_
#define __DISPLAY_H_

  void DisplayInit(void);
  void DisplayClearCanvas(void);
  void DisplayClear(void);
  void DisplayText(const char Text[]);
//void DisplayText(const String &Text, uint8_t FontSize, int16_t X, int16_t Y);  
  void DisplayText(const char Text[], uint8_t FontSize, int16_t X, int16_t Y, bool Show=true);
  void DisplayUpdate(void);


  void DisplayTest(void);



#endif // __DISPLAY_H_