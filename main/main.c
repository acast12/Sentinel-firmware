#include "esp_log.h"
#include "sensor_task.h"
#include "processing_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_driver.h"
#include "shared_data.h"

//static const char *TAG = "MAIN";
QueueHandle_t g_sensor_queue;
QueueHandle_t g_processed_queue;

void app_main(void) {
    i2c_master_init();
    g_sensor_queue = xQueueCreate(10, sizeof(sensor_reading_t));
    g_processed_queue = xQueueCreate(10, sizeof(processed_reading_t));
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
    xTaskCreate(processing_task, "processing_task", 4096, NULL, 4, NULL);
}