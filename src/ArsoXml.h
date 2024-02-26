#ifndef __ARSO_XML_H_
#define __ARSO_XML_H_

bool GetARSOdata(void);

struct ArsoWeather_t
{
    String Day;
    String PartOfDay;
    String Sky;
    String Rain;
    String Temperature;
};


extern ArsoWeather_t ArsoWeather[3];

#endif // __ARSO_XML_H_