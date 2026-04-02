#include "sensor_task.h"
#include "dht22.h"
#include "sgp30.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"



void sensor_task(void *pvParameters) {
    float temperature_c, humidity;
    uint16_t eco2, tvoc;    
    dht22_init();
    sgp30_init();
    for(;;) {
        dht22_read(&temperature_c, &humidity);
        sgp30_measure(&eco2, &tvoc);

        sgp30_set_humidity_compensation(temperature_c, humidity);
        ESP_LOGI("SENSOR", "T=%.1f°C H=%.1f%% eCO2=%u tvoc=%u", temperature_c, humidity, eco2, tvoc);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}