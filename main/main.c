#include "esp_log.h"
#include "sensor_task.h"
#include "processing_task.h"
#include "wifi_manager.h"
#include "mqtt_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_driver.h"
#include "shared_data.h"
#include "nvs_flash.h"

static const char *TAG = "MAIN";

QueueHandle_t g_sensor_queue;
QueueHandle_t g_processed_queue;

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    i2c_master_init();

    g_sensor_queue = xQueueCreate(10, sizeof(sensor_reading_t));
    g_processed_queue = xQueueCreate(10, sizeof(processed_reading_t));

    // WiFi must be up before MQTT can connect
    esp_err_t wifi_err = wifi_manager_init();
    if (wifi_err != ESP_OK) {
        ESP_LOGE(TAG, "WiFi failed — mqtt_task exiting");
        vTaskDelete(NULL);
        return;
    }

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(processing_task, "processing_task", 4096, NULL, 4, NULL);
    xTaskCreate(mqtt_task, "mqtt_task", 8192, NULL, 3, NULL);
}