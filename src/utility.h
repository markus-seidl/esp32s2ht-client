//
// Created by Markus Seidl on 26.02.21.
//

#ifndef ESP32S2HT_UTILITY_H
#define ESP32S2HT_UTILITY_H

// #define DEBUG

#ifdef DEBUG
#define debugln(a) Serial.println(a);
#define debug(a) Serial.print(a);
#else
#define debugln(a) ;
#define debug(a) ;
#endif


void doDeepSleep(int sleep_seconds);
int isWakeupFromSleep();

#endif //ESP32S2HT_UTILITY_H
