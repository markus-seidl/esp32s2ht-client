//
// Created by Markus Seidl on 26.02.21.
//

#ifndef ESP32S2HT_WIFIHELPERS_H
#define ESP32S2HT_WIFIHELPERS_H

void startConnectingToWifi();

esp_err_t sendDataToServer(float temperature, float humidity, unsigned long executionTimeNormal,
                           unsigned long executionTimeWifi, int wakeupCount, int invalidMeasurementCount, int voltage);

int waitForWifi();

#endif //ESP32S2HT_WIFIHELPERS_H
