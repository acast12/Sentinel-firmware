#include "provisioning.h"
#include "secrets.h"
#include "esp_log.h"
#include "nvs.h"
#include "cJSON.h"
#include "driver/uart.h"

static const char *TAG = "PROVISIONING";

esp_err_t provisioning_check() {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("sentinel", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    char tenant_id[37]; // UUIDs => 36 chars + null terminator
    size_t tx_len = sizeof(tenant_id);
    err = nvs_get_str(handle, "tenant_id", tenant_id, &tx_len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Device not provisioned — entering provisioning mode");
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        };
        uart_param_config(UART_NUM_0, &uart_config);
        uart_driver_install(UART_NUM_0, 1024, 0, 0, NULL, 0);

        char buf[256];
        int rx_len = uart_read_bytes(UART_NUM_0, buf, sizeof(buf) - 1, portMAX_DELAY);



        buf[rx_len] = '\0';
        // DEBUG
        if (rx_len <= 0) {
            ESP_LOGE(TAG, "No data received during provisioning window");
            nvs_close(handle);
            return ESP_ERR_TIMEOUT;
        }
        ESP_LOGI(TAG, "Received %d bytes: %s", rx_len, buf);
        cJSON *json = cJSON_Parse(buf);
        if (json == NULL) {
            ESP_LOGE(TAG, "Failed to parse JSON — check payload format");
            nvs_close(handle);
            return ESP_ERR_INVALID_ARG;
        }

        cJSON *tenant_item = cJSON_GetObjectItem(json, "tenant_id");
        cJSON *device_item = cJSON_GetObjectItem(json, "device_id");

        if (!cJSON_IsString(tenant_item) || !cJSON_IsString(device_item)) {
            ESP_LOGE(TAG, "Invalid JSON payload");
            cJSON_Delete(json);
            nvs_close(handle);
            return ESP_ERR_INVALID_ARG;
        }

        nvs_set_str(handle, "tenant_id", tenant_item->valuestring);
        nvs_set_str(handle, "device_id", device_item->valuestring);

        nvs_commit(handle);
        nvs_close(handle);
        cJSON_Delete(json);

        ESP_LOGI(TAG, "Provisioning complete!");
        ESP_LOGI(TAG, "tenant_id: %s", tenant_item->valuestring);
        ESP_LOGI(TAG, "device_id: %s", device_item->valuestring);
        ESP_LOGI(TAG, "Rebooting...");

        esp_restart();
        return ESP_OK;
    }
    else if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS read error: %s", esp_err_to_name(err));
        nvs_close(handle);
    return err;
    }


    nvs_close(handle);
    return ESP_OK;
}

esp_err_t provisioning_get_topic(char *buf, size_t buf_len) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("sentinel", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    char tenant_id[37]; // UUIDs => 36 chars + null terminator
    size_t ten_len = sizeof(tenant_id);
    char device_id[37]; // UUIDs => 36 chars + null terminator
    size_t dev_len = sizeof(device_id);

    err = nvs_get_str(handle, "tenant_id", tenant_id, &ten_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "tenant_id read failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }
    err = nvs_get_str(handle, "device_id", device_id, &dev_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "device_id read failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    snprintf(buf, buf_len, "tenants/%s/devices/%s/telemetry", tenant_id, device_id);

    nvs_close(handle);
    return ESP_OK;
}