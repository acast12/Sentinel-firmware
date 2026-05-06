#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_sntp.h"
#include "esp_netif_sntp.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "config.h"

static EventGroupHandle_t wifi_event_group;

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1
static int retry_count = 0;

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data);
static void sync_time(void);

esp_err_t wifi_manager_init(void) {
    wifi_event_group =xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("WIFI", "Connected");
        sync_time();
        return ESP_OK;
    }
    ESP_LOGE("WIFI", "Failed to connect");
    return ESP_FAIL;
    
}

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t event_id, void *event_data) {
    if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // connect
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // retry or fail
        if (retry_count < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            retry_count++;
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // set connected bit
        retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void sync_time(void) {
    setenv("TZ", "PST8PDT,M3.2.0,M11.1.0", 1);
    tzset();
    ESP_LOGI("SNTP", "TZ set to: %s", getenv("TZ"));

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);
    
    // 10 seconds for time sync
    int retry = 0;
    while (esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < 10) {
        ESP_LOGI("SNTP", "Waiting for time sync... (%d/10)", retry + 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retry++;
    }
    
    time_t now;
    struct tm timeinfo;
    char buf[32];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    ESP_LOGI("SNTP", "Local time: %s", buf);
}