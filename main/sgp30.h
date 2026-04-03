#pragma once

#include "esp_err.h"
#include <stdint.h>

esp_err_t sgp30_init(void);
esp_err_t sgp30_measure(uint16_t *eco2, uint16_t *tvoc);
esp_err_t sgp30_set_humidity_compensation(float temperature_c, float humidity_rh);
