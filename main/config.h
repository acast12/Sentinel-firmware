#pragma once 

// I2C 
#define I2C_MASTER_SDA_IO   GPIO_NUM_21      // GPIO number used for I2C master data 
#define I2C_MASTER_SCL_IO   GPIO_NUM_22    // GPIO number used for I2C master clock
#define I2C_MASTER_NUM I2C_NUM_0

// SGP30 SENSOR
#define SGP30_SENSOR_ADDR 0x58
// SGP30 supports fast mode on 400 kHz
#define I2C_MASTER_FREQ_HZ  400000
#define I2C_MASTER_TIMEOUT_MS   100

// DHT22 SENSOR
#define DHT22_GPIO_PIN    GPIO_NUM_4
