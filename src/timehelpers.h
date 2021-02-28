//
// Created by Markus Seidl on 26.02.21.
//

#ifndef ESP32S2HT_TIMEHELPERS_H
#define ESP32S2HT_TIMEHELPERS_H


int time_is_valid();

long time_current_timestamp();

int time_current_hour();

void time_debug_print();

void time_obtain();

#endif //ESP32S2HT_TIMEHELPERS_H
