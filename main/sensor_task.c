#include "sensor_task.h"
#include "dht22.h"
#include "sgp30.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "shared_data.h"
#include <sys/time.h>




void sensor_task(void *pvParameters) {
    float temperature_c, humidity;
    uint16_t eco2, tvoc;    
    dht22_init();
    sgp30_init();
    for(;;) {
        if (dht22_read(&temperature_c, &humidity) == ESP_OK) {
            sgp30_set_humidity_compensation(temperature_c, humidity);
            sgp30_measure(&eco2, &tvoc);

            
            ESP_LOGI("SENSOR", "T=%.1f°C H=%.1f%% eCO2=%uppb tvoc=%uppb", temperature_c, humidity, eco2, tvoc);

            struct timeval tv;
            gettimeofday(&tv, NULL);

            sensor_reading_t reading = {
                .temperature_c = temperature_c,
                .humidity = humidity,
                .eco2 = eco2,
                .tvoc = tvoc,
                .valid = true,
                .timestamp_ms = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000,
            };
            xQueueSend(g_sensor_queue, &reading, 0);
        }
        // Keep one second cadence for SGP30 calibration
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}