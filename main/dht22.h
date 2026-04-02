#pragma once

#include "esp_err.h"

esp_err_t dht22_init(void);
esp_err_t dht22_read(float *temperature_c, float *humidity);