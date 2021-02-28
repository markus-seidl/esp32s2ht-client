//
// Created by Markus Seidl on 26.02.21.
//
#include "Arduino.h"
#include "BitBang_I2C.h"
#include "utility.h"
#include "sht30.h"

BBI2C bbi2c;

static float temperature = -1.0f;
static float humidity = -1.0f;

// Investigate: https://github.com/Risele/SHT3x/blob/master/SHT3x.cpp

void initSensor(uint8_t sensorAddr) {
    bbi2c.bWire = 0;
    bbi2c.iSDA = 13;
    bbi2c.iSCL = 14;

    ESP_LOGI(TAG, "Init i2c...");
    I2CInit(&bbi2c, 100000);

    uint8_t data_request[2];
    data_request[0] = 0x00;
    data_request[1] = 0x06;
    I2CWrite(&bbi2c, sensorAddr, data_request, 2);
    ESP_LOGI(TAG, "Init i2c...done.");
}

esp_err_t startReadDataFromSensor(uint8_t sensorAddr) {
    uint8_t data_request[2];
    data_request[0] = 0x2C;
    data_request[1] = 0x06;
    int ret = I2CWrite(&bbi2c, sensorAddr, data_request, 2);
    if (ret == 0) {
        ESP_LOGE(TAG, "Failure to send, NACK received.");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void readDataFromSensor(uint8_t sensorAddr) {
    uint8_t data_return[6];
    data_return[0] = 1;
    data_return[1] = 1;
    int ret = I2CRead(&bbi2c, sensorAddr, data_return, 6);
    if (ret != 1) {
        ESP_LOGE(TAG, "I2C read error %d", ret);
        temperature = -1;
        humidity = -1;
        return;
    }

    temperature = (((((float) data_return[0] * 256.0f) + (float) data_return[1]) * 175.0f) / 65535.0f) - 45.0f;
    humidity = (((((float) data_return[3] * 256.0f) + (float) data_return[4]) * 100.0f) / 65535.0f);
    ESP_LOGI(TAG, "Read from sensor: Temperature %f C, Humidity %f %%", temperature, humidity);

    if(temperature > 100 || humidity >= 100) {
        // the measurements are out of range - there must be something wrong
        temperature = -1;
        humidity = -1;
    }
}

float getTemperature() {
    return temperature;
}

float getHumidity() {
    return humidity;
}
