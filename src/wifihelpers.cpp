//
// Created by Markus Seidl on 26.02.21.
//
#include <esp_http_client.h>
#include "WiFi.h"
#include "WiFiConfig.h"
#include "utility.h"
#include "timehelpers.h"

void startConnectingToWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.config(IPAddress(192, 168, 20, 110),
                IPAddress(192, 168, 20, 1),
                IPAddress(255, 255, 255, 0),
                IPAddress(192, 168, 20, 1),
                IPAddress(192, 168, 20, 1));

    WiFi.begin(SSID, WiFiPassword);
    ESP_LOGI(TAG, "Connecting to %s...", SSID);
}

esp_err_t sendDataToServer(float temperature, float humidity, unsigned long executionTimeNormal,
                      unsigned long executionTimeWifi, int wakeupCount, int invalidMeasurementCount, int voltage) {
    esp_http_client_config_t config_disabled = {
            .host = "192.168.1.121",
            .port = 8888,
            .path = "/",
            .query = ("temperature=" + String(temperature) + ",humidity=" + String(humidity)).c_str(),
    };
    // unsigned long diff = millis() - startupTime;
    String url = "http://192.168.1.121:8888/?t="
                 + String(temperature)
                 + "&h=" + String(humidity)
                 + "&wtn=" + String(executionTimeNormal)
                 + "&wtwifi=" + String(executionTimeWifi)
                 + "&wc=" + String(wakeupCount)
                 + "&imc=" + String(invalidMeasurementCount)
                 + "&v=" + String(voltage)
                 + "&n=" + String(WiFi.getHostname())
                 + "&time=" + String(time_current_timestamp());
    ESP_LOGI(TAG, "Using url %s", url.c_str());
    esp_http_client_config_t config = {
            .url = url.c_str(),
            .timeout_ms = 1000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Send successfully! Status %d, content_length %d",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    esp_http_client_cleanup(client);

    return err;
}

int waitForWifi() {
    uint8_t i = 0;
    ESP_LOGI(TAG, "Waiting for WiFi to come online.");
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);

        if ((++i % 64) == 0) {
            ESP_LOGE(TAG, "No connection yet. This takes too long, going to sleep.");
            return ESP_FAIL;
        }
    }

    ESP_LOGI(TAG, "Connected. My IP address is: %s", String(WiFi.localIP().toString()).c_str());

    return ESP_OK;
}
