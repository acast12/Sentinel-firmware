#include "processing_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "shared_data.h"
#include "config.h"
#include "esp_log.h"

void processing_task(void *pvParameters) {
    float temp_ema;
    float humidity_ema;
    float eco2_ema;
    float tvoc_ema;
    bool seeded = false;

    for(;;) {
        sensor_reading_t reading;
        processed_reading_t writing = {0};
        xQueueReceive(g_sensor_queue, &reading, portMAX_DELAY);

        if (!seeded) {
            temp_ema = reading.temperature_c;
            humidity_ema = reading.humidity;
            eco2_ema = reading.eco2;
            tvoc_ema = reading.tvoc;
            seeded = true;
        } else {
            temp_ema = 0.2f * reading.temperature_c + 0.8f * temp_ema;
            humidity_ema = 0.2f * reading.humidity + 0.8f * humidity_ema;
            eco2_ema = 0.2f * reading.eco2 + 0.8f * eco2_ema;
            tvoc_ema = 0.2f * reading.tvoc + 0.8f * tvoc_ema;
        }

        writing.temp_smoothed = temp_ema;
        writing.humidity_smoothed = humidity_ema;
        writing.eco2_smoothed = eco2_ema;
        writing.tvoc_smoothed = tvoc_ema;
        writing.timestamp_ms = reading.timestamp_ms;

        writing.alert_temp_high = temp_ema > ALERT_TEMP_HIGH_C;
        writing.alert_temp_low = temp_ema < ALERT_TEMP_LOW_C;
        writing.alert_humidity_high = humidity_ema > ALERT_HUMIDITY_HIGH;
        writing.alert_humidity_low = humidity_ema < ALERT_HUMIDITY_LOW;
        writing.alert_eco2_warn = eco2_ema > ALERT_ECO2_WARN_PPM;
        writing.alert_eco2_bad = eco2_ema > ALERT_ECO2_BAD_PPM;
        writing.alert_tvoc_warn = tvoc_ema > ALERT_TVOC_WARN_PPB;
        writing.alert_tvoc_bad = tvoc_ema > ALERT_TVOC_BAD_PPB;

        xQueueSend(g_processed_queue, &writing, 0);

        ESP_LOGI("PROC", "T=%.2f H=%.2f eCO2=%.0f TVOC=%.0f", 
         writing.temp_smoothed, writing.humidity_smoothed,
         writing.eco2_smoothed, writing.tvoc_smoothed);
    }
}