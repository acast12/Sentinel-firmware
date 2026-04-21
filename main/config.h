#pragma once 

#include "secrets.h"

// I2C 
#define I2C_MASTER_SDA_IO   GPIO_NUM_21     // GPIO number used for I2C master data 
#define I2C_MASTER_SCL_IO   GPIO_NUM_22    // GPIO number used for I2C master clock
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  400000      // SGP30 supports fast mode on 400 kHz
#define I2C_MASTER_TIMEOUT_MS   100

// SGP30 SENSOR
#define SGP30_SENSOR_ADDR 0x58

// DHT22 SENSOR
#define DHT22_GPIO_PIN    GPIO_NUM_4

// ALERT VALUES
#define ALERT_TEMP_HIGH_C       35.0f
#define ALERT_TEMP_LOW_C        10.0f
#define ALERT_HUMIDITY_HIGH     60.0f
#define ALERT_HUMIDITY_LOW     30.0f
#define ALERT_ECO2_WARN_PPM     1000.0f
#define ALERT_ECO2_BAD_PPM      2000.0f
#define ALERT_TVOC_WARN_PPB     220.0f
#define ALERT_TVOC_BAD_PPB      660.0f

// MQTT
#define MQTT_TOPIC              "sentinel/readings"
#define MQTT_CLIENT_ID          "sentinel-node-01"
#define MQTT_QOS                1

// WIFI
#define WIFI_MAXIMUM_RETRY      5