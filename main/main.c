#include "esp_log.h"
#include "sensor_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_driver.h"

static const char *TAG = "MAIN";

void app_main(void) {
    i2c_master_init();
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}