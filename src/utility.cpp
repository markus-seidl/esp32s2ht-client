//
// Created by Markus Seidl on 26.02.21.
//

#include "Arduino.h"
#include "utility.h"

void doDeepSleep(int sleep_seconds) {
    ESP_LOGI(TAG, "Sleeping...");

    debugln("Off");


    esp_deep_sleep(sleep_seconds * 1000000);
}

int isWakeupFromSleep() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    if (cause == ESP_SLEEP_WAKEUP_TIMER) {
        return ESP_OK;
    }
    return ESP_FAIL;
}
