#include "i2c_driver.h"
#include "config.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;

esp_err_t i2c_master_init(void) {   // configure and install i2c driver on port
    i2c_master_bus_config_t bus_config = {
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .i2c_port = I2C_MASTER_NUM,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SGP30_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));
    return ESP_OK;
}


esp_err_t i2c_write_reg(uint8_t reg_addr, uint8_t data) {
    uint8_t write_buf[2] = { reg_addr, data };
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS));
    return ESP_OK;
}

esp_err_t i2c_read_regs(uint8_t reg_addr, uint8_t *buf, size_t len) {
    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &reg_addr, 1, buf, len, I2C_MASTER_TIMEOUT_MS));
    return ESP_OK;
}

/**
 * @brief Write a byte to a SGP30 sensor register
 */
esp_err_t i2c_write(uint8_t *buf, size_t len) {
    esp_err_t err = i2c_master_transmit(dev_handle, buf, len, 100);
    if (err != ESP_OK) {
        ESP_LOGW("I2C", "write failed: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}

/**
 * @brief Read a sequence of bytes from a SGP30 sensor registers
 */
esp_err_t i2c_read(uint8_t *buf, size_t len) {
    esp_err_t err = i2c_master_receive(dev_handle, buf, len, 100);
    if (err != ESP_OK) {
        ESP_LOGW("I2C", "read failed: %s", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}