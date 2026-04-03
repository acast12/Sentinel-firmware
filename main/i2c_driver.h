#pragma once

#include "esp_err.h"
#include <stdint.h>

esp_err_t i2c_master_init(void);

// register model
esp_err_t i2c_write_reg(uint8_t reg_addr, uint8_t data);
esp_err_t i2c_read_regs(uint8_t reg_addr, uint8_t *buf, size_t len);


// reads raw bytes
esp_err_t i2c_write(uint8_t *buf, size_t len);
esp_err_t i2c_read(uint8_t *buf, size_t len);