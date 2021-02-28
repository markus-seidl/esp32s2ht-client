//
// Created by Markus Seidl on 26.02.21.
//

#ifndef ESP32S2HT_SHT30_H
#define ESP32S2HT_SHT30_H

#include "Arduino.h"
#include "BitBang_I2C.h"

void initSensor(uint8_t sensorAddr);

esp_err_t startReadDataFromSensor(uint8_t sensorAddr);

void readDataFromSensor(uint8_t sensorAddr);

float getTemperature();

float getHumidity();

#endif //ESP32S2HT_SHT30_H
