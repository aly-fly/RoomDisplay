#ifndef MYWIFI_H
#define MYWIFI_H

#include "__CONFIG.h"

enum WifiState_t {disconnected, connected};
void WifiInit(void);
void WifiReconnectIfNeeded(void);

extern WifiState_t WifiState;


#endif // MYWIFI_H
