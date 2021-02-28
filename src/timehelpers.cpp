//
// Created by Markus Seidl on 26.02.21.
//
#include "esp_sntp.h"
#include "Arduino.h"
#include "utility.h"
#include "timehelpers.h"

int time_is_valid() {
    time_t now;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

/** in seconds */
long time_current_timestamp() {
    time_t now = 0;
    time(&now);

    return now;
}

int time_current_hour() {
    time_t now;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_hour;
}

void time_debug_print() {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "Current date-time %02d:%02d:%02d %02d.%02d.%d, timestamp %d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
             timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900, now);
}

void time_obtain() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pfsense.koala.haus");
    // sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();

    // wait for time to be set
    int retry = 0;
    const int retry_count = 100;
    ESP_LOGI(TAG, "Waiting for system time to be set...");
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && ++retry < retry_count) {
        delay(100);

        if (retry % 10 == 0) {
            ESP_LOGI(TAG, "Still syncing with ntp.");
        }
    }

    time_debug_print();
    sntp_stop();
}