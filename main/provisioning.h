#pragma once

#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stddef.h>

esp_err_t provisioning_check();
esp_err_t provisioning_get_topic(char *buf, size_t buf_len);