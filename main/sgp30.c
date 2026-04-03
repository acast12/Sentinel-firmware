#include "sgp30.h"
#include "i2c_driver.h"
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>


uint8_t sgp30_crc(uint8_t *data, size_t len);

// sgp30_iaq_init 0x2003 
esp_err_t sgp30_init(void) {
    uint8_t cmd[2] = { 0x20, 0x03 };
    ESP_ERROR_CHECK(i2c_write(cmd,2));
    vTaskDelay(pdMS_TO_TICKS(10));
    return ESP_OK;
}

// sgp30_measure_iaq 0x2008 (output parameter is 6 bytes)
esp_err_t sgp30_measure(uint16_t *eco2, uint16_t *tvoc) {
    uint8_t cmd[2] = { 0x20, 0x08 };
    ESP_ERROR_CHECK(i2c_write(cmd,2));

    vTaskDelay(pdMS_TO_TICKS(25));

    uint8_t buf[6];
    //ESP_ERROR_CHECK(i2c_read(buf, 6));
    esp_err_t err = i2c_read(buf, 6);
    if (err != ESP_OK) return err;

    uint8_t crc = sgp30_crc(buf, 2);
    if (crc != buf[2]) return ESP_ERR_INVALID_CRC;

    crc = sgp30_crc(buf + 3, 2);
    if (crc != buf[5]) return ESP_ERR_INVALID_CRC;
    
    *eco2 = (buf[0] << 8) | buf[1];
    *tvoc = (buf[3] << 8) | buf[4];

    return ESP_OK;
}

uint8_t sgp30_crc(uint8_t *data, size_t len) {
    uint8_t crc = 0xFF; 
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
        }
    }
    return crc;
}

// sgp30_set_absolute_humidity 0x2061 (input parameter is 3 bytes)
esp_err_t sgp30_set_humidity_compensation(float temperature_c, float humidity_rh) {
    float ah = (humidity_rh / 100.0f)
        * 6.112f
        * expf((17.67f * temperature_c) / (temperature_c + 243.5f))
        * 2.1674f
        / (273.15f + temperature_c);

    uint8_t ah_int  = (uint8_t)ah;
    uint8_t ah_frac = (uint8_t)((ah - ah_int) * 256.0f);

    uint8_t humidity_bytes[2] = { ah_int, ah_frac };
    uint8_t crc = sgp30_crc(humidity_bytes, 2);

    uint8_t buf[5] = { 0x20, 0x61, ah_int, ah_frac, crc };
    //ESP_ERROR_CHECK(i2c_write(buf, 5));
    esp_err_t err = i2c_write(buf, 5);
    if (err != ESP_OK) return err;

    vTaskDelay(pdMS_TO_TICKS(10));

    return ESP_OK;
}