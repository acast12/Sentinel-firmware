#include "dht22.h"
#include "config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static int wait_for_level(int level, int timeout_us);

esp_err_t dht22_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT22_GPIO_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,  // open-drain, (OD), allows the pin to go low instead of actively driving high
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(DHT22_GPIO_PIN, 1);
    return ESP_OK;
}

esp_err_t dht22_read(float *temperature_c, float *humidity) {
    gpio_set_level(DHT22_GPIO_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT22_GPIO_PIN, 1);

    esp_rom_delay_us(30);
        
    if (wait_for_level(0, 100) < 0) return ESP_ERR_TIMEOUT;
    if (wait_for_level(1, 100) < 0) return ESP_ERR_TIMEOUT;
    if (wait_for_level(0, 100) < 0) return ESP_ERR_TIMEOUT;

    // DATA = 16 bits RH data | 16 bits Temperature data | 8 bits check-sum 
    uint8_t data[5] = {0};

    for (int bytes = 0; bytes < 5; bytes++) {
        for (int bit = 0; bit < 8; bit++) {
            wait_for_level(1, 100); // wait for high pulse to start
            int duration = wait_for_level(0, 100); // measure how long it stays high
            if (duration > 28) {
                // bit is 1
                data[bytes] |= (1 << (7 - bit));
            }  
        }
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return ESP_ERR_INVALID_CRC;

    *humidity      = ((data[0] << 8) | data[1]) / 10.0f;
    *temperature_c = (((data[2] & 0x7F) << 8) | data[3]) / 10.0f;
    if (data[2] & 0x80) *temperature_c = -*temperature_c;
    return ESP_OK;
}

static int wait_for_level(int level, int timeout_us) {
    int elapsed = 0;
    while (gpio_get_level(DHT22_GPIO_PIN) != level) {
        if (elapsed >= timeout_us) return -1; // timeout
        esp_rom_delay_us(1);
        elapsed++;
    }
    return elapsed;
}