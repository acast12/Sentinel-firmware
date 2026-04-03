#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    float temperature_c;
    float humidity;
    uint16_t eco2;
    uint16_t tvoc;
    bool valid;
    uint32_t timestamp_ms;
} sensor_reading_t;

typedef struct {
    float temp_smoothed;
    float humidity_smoothed;
    float eco2_smoothed;
    float tvoc_smoothed;
    bool alert_temp_high;
    bool alert_temp_low;
    bool alert_humidity_high;
    bool alert_humidity_low;
    bool alert_eco2_warn;
    bool alert_eco2_bad;
    bool alert_tvoc_warn;
    bool alert_tvoc_bad;
    uint32_t timestamp_ms;
} processed_reading_t;

extern QueueHandle_t g_sensor_queue;     // sensor_task → processing_task
extern QueueHandle_t g_processed_queue;  // processing_task → mqtt_task