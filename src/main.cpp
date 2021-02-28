#include <esp_adc_cal.h>
#include <driver/adc.h>
#include "Arduino.h"
#include "utility.h"
#include "wifihelpers.h"
#include "sht30.h"
#include "timehelpers.h"
#include "WiFi.h"

#define SENSOR_ADDR 0x44

// Time to sleep between measurements
#define SLEEP_SECONDS (120 - 1)

#define MIN_REPORT_TEMP 1
#define MIN_REPORT_HUMID 5

unsigned long startupTime = -1;

RTC_DATA_ATTR static float lastReportedTemperature = -1.0f;
RTC_DATA_ATTR static float lastReportedHumidity = -1.0f;
RTC_DATA_ATTR static unsigned long lastReportedTime = 0;
RTC_DATA_ATTR static int lastReportedHour = -1;

RTC_DATA_ATTR static int wakeupCount = 0;
RTC_DATA_ATTR static unsigned long wakeTimeWifi = 0;
RTC_DATA_ATTR static unsigned long wakeTimeNormal = 0;
RTC_DATA_ATTR static int invalidMeasurementCount = 0;

void updateWakeUpTime(boolean useWifi) {
    if (startupTime <= 0) {
        startupTime = millis();
    } else {
        unsigned long temp = millis();
        unsigned long add = temp - startupTime;
        if (useWifi) {
            wakeTimeWifi += add;
        } else {
            wakeTimeNormal += add;
        }
        startupTime = temp;

        debugln("Added " + String(add) + " to the current wake up time.");
        ESP_LOGI(TAG, "Added %d ms to the current wake up time.", add);
    }
}

void setup() {
    updateWakeUpTime(false);
    wakeupCount++;

#ifdef DEBUG
    Serial.begin(115200);
    while (!Serial) { ; // wait for serial port to connect. Needed for native USB port only
    }
#endif

    ESP_LOGI(TAG, "Time valid? %s", time_is_valid() == ESP_OK ? "true" : "false");
    ESP_LOGI(TAG, "Wake up Count: %d", wakeupCount);

    debugln("On.");
}

int readBatteryVoltage() {
    esp_err_t err = adc1_config_width(ADC_WIDTH_BIT_13);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "adc1_config_width returned: %d", err);
    }
    err = adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "adc1_config_channel_atten returned: %d", err);
    }

    return adc1_get_raw(ADC1_CHANNEL_0);
}

boolean isTimeToReportTempAnyway() {
    if (lastReportedTime == 0) {
        return true;
    }
    if (time_current_hour() != lastReportedHour) {
        return true;
    }
    return false;
}

void loop() {
    // Determine if we need to spin up the Wifi Module
    boolean useWifi = (time_is_valid() != ESP_OK);
    useWifi = useWifi || isTimeToReportTempAnyway();
    ESP_LOGI(TAG, "Determined to use wifi? %s", useWifi ? "true" : "false");
    int voltageMeasurement = -1;
    if (useWifi) {
        voltageMeasurement = readBatteryVoltage();
        startConnectingToWifi();
    }

    // Initialize Sensor readout
    initSensor(SENSOR_ADDR);
    startReadDataFromSensor(SENSOR_ADDR);

    // Wait for Wifi (and NTP) to come online (in the meantime the sensor can measure the temperature)
    if (useWifi) {
        if (waitForWifi() != ESP_OK) {
            updateWakeUpTime(useWifi);
            doDeepSleep(SLEEP_SECONDS);
        }
        if (time_is_valid() != ESP_OK) {
            time_obtain();
        }
    } else {
        delay(15); // give the temperature sensor time to measure
    }

    readDataFromSensor(SENSOR_ADDR);
    float temperature = getTemperature();
    float humidity = getHumidity();
    if (temperature == -1 || humidity == -1) {
        ESP_LOGE(TAG, "Invalid data - sleeping...");
        invalidMeasurementCount++;

        updateWakeUpTime(useWifi);
        doDeepSleep(SLEEP_SECONDS);
    }

    boolean sendData = abs(lastReportedTemperature - temperature) > MIN_REPORT_TEMP;
    sendData = sendData || abs(lastReportedHumidity - humidity) > MIN_REPORT_HUMID;
    sendData = sendData || useWifi;

    if (!useWifi && sendData) {
        useWifi = true;
        // no wifi is up, but we need to send data.
        ESP_LOGI(TAG, "No wifi is set up, but we need to send data. Do it now.");
        startConnectingToWifi();
        if (waitForWifi() != ESP_OK) {
            updateWakeUpTime(useWifi);
            doDeepSleep(SLEEP_SECONDS);
        }
        if (time_is_valid() != ESP_OK) {
            time_obtain();
        }
    }

    if (sendData) {
        ESP_LOGI(TAG, "Sending data to server.");

        updateWakeUpTime(true);
        sendDataToServer(temperature, humidity, wakeTimeNormal, wakeTimeWifi,
                         wakeupCount, invalidMeasurementCount, voltageMeasurement);
        lastReportedHumidity = humidity;
        lastReportedTemperature = temperature;
        lastReportedTime = time_current_timestamp();
        lastReportedHour = time_current_hour();

        wakeupCount = 0;
        wakeTimeNormal = 0;
        wakeTimeWifi = 0;
    }

    if (useWifi) {
        ESP_LOGI(TAG, "Disconnecting...");
        WiFi.disconnect(true, false);
    }

    // sleep
    updateWakeUpTime(useWifi);
    doDeepSleep(SLEEP_SECONDS);
}
