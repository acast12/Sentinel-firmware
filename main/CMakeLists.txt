idf_component_register(SRCS "i2c_driver.c" "main.c" "sgp30.c" "dht22.c" "sensor_task.c"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_i2c esp_driver_gpio
                       )
