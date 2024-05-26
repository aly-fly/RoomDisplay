#include <Arduino.h>
#include "display.h"
#include "ArsoXml.h"

void ArsoPlotForecast(void) {
    String Line;

    DisplayClear(CLWHITE);

    char FileName[30];
    sprintf(FileName, "/bg_sky_%dx%d.bmp", DspW, DspH);
    DisplayShowImage(FileName,   0, 0);

    // zgoraj - dnevi
    Line = ArsoWeather[0].DayName;
    DisplayText(Line.c_str(), 1,   10, 10, CLDARKBLUE);

    Line = ArsoWeather[2].DayName;
    DisplayText(Line.c_str(), 1, 180, 10, CLDARKBLUE);


    // na sredini so slikce (original 32x32, povečane x2)
    String FN;
    FN = "/w/" + ArsoWeather[0].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),   6, 44, 2);

    FN = "/w/" + ArsoWeather[1].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  82, 44, 2);

    FN = "/w/" + ArsoWeather[2].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  174, 44, 2);

    FN = "/w/" + ArsoWeather[3].WeatherIcon + ".bmp";
    DisplayShowImage(FN.c_str(),  249, 44, 2);

    // wind data
    DisplayText(ArsoWeather[0].WindIcon.c_str(), 0,   6, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[1].WindIcon.c_str(), 0,  82, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[2].WindIcon.c_str(), 0, 174, 111, CLDARKBLUE);    
    DisplayText(ArsoWeather[3].WindIcon.c_str(), 0, 249, 111, CLDARKBLUE);    

    //tft.setTextDatum(TR_DATUM); // top right

    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,   44+2, 175+2, CLGREY); // shadow
    DisplayText(ArsoWeather[0].Temperature.c_str(), 2,   44,   175,   CLBLUE);
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,   44+2, 132+2, CLGREY); // shadow
    DisplayText(ArsoWeather[1].Temperature.c_str(), 2,   44,   132,   CLRED);
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  201+2, 175+2, CLGREY); // shadow
    DisplayText(ArsoWeather[2].Temperature.c_str(), 2,  201,   175,   CLBLUE);
    DisplayText(ArsoWeather[3].Temperature.c_str(), 2,  201+2, 132+2, CLGREY); // shadow
    DisplayText(ArsoWeather[3].Temperature.c_str(), 2,  201,   132,   CLRED);

    DisplayText(SunRiseTime.c_str(), 1,    5,        240-23,   CLDARKGREEN);
    DisplayText(SunSetTime.c_str(),  1,  320 - 80,   240-23,   CLDARKGREEN);

    tft.setTextDatum(TL_DATUM); // top left (default)
    }

