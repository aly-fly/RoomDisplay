#ifndef __ARSO_XML_H_
#define __ARSO_XML_H_

bool GetARSOdata(void);
bool GetARSOmeteogram(void);

struct ArsoWeather_t
{
    String Day;
    String PartOfDay;
    String Sky;
    String Rain;
    String WeatherIcon;
    String WindIcon;
    String Temperature;
    float TemperatureN;
    float RainN;
    float SnowN;
};


extern ArsoWeather_t ArsoWeather[4];
extern ArsoWeather_t ArsoMeteogram[26];
extern String SunRiseTime, SunSetTime;

#endif // __ARSO_XML_H_