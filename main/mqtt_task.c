#include "mqtt_task.h"
#include "wifi_manager.h"
#include "shared_data.h"
#include "config.h"
#include "secrets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <stdio.h>

const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----";

static const char *TAG = "MQTT";

#define MQTT_CONNECTED_BIT  BIT0
#define MQTT_FAIL_BIT       BIT1

static EventGroupHandle_t   s_mqtt_event_group;
static esp_mqtt_client_handle_t s_client = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static int build_payload(char *buf, size_t buf_len, const processed_reading_t *r);

void mqtt_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");

    for(;;) {
        esp_err_t wifi_err = wifi_manager_init();
        if (wifi_err != ESP_OK) {
            ESP_LOGE(TAG, "WIFI failed -retrying in 10s");
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }
        break;
    }
 
    s_mqtt_event_group = xEventGroupCreate();
 
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri                      = ACTIVE_BROKER_URI,
        #if !USE_LOCAL_BROKER
            .broker.verification.certificate = root_ca,
        #endif
        .credentials.username                    = ACTIVE_USERNAME,
        .credentials.authentication.password     = ACTIVE_PASSWORD,
        .credentials.client_id                   = MQTT_CLIENT_ID,
        .session.keepalive                        = 30,
    };
 
    s_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        vTaskDelete(NULL);
        return;
    }
 
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_client));
 
    // Wait for connected or failed
    EventBits_t bits = xEventGroupWaitBits(
        s_mqtt_event_group,
        MQTT_CONNECTED_BIT | MQTT_FAIL_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(10000)    // 10 second timeout
    );
 
    if (!(bits & MQTT_CONNECTED_BIT)) {
        ESP_LOGE(TAG, "Could not connect to broker");
        esp_mqtt_client_destroy(s_client);
        vTaskDelete(NULL);
        return;
    }
 
    // ── Publish loop ──────────────────────────────────────────────────────────
    processed_reading_t reading;
    char payload[256];
 
    for (;;) {
        // Block until processing task pushes a reading
        if (xQueueReceive(g_processed_queue, &reading, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        ESP_LOGI(TAG, "Queue item receieved");
 
        // Skip publish if broker disconnected
        if (!(xEventGroupGetBits(s_mqtt_event_group) & MQTT_CONNECTED_BIT)) {
            ESP_LOGW(TAG, "Not connected, skipping publish");
            continue;
        }
 
        int len = build_payload(payload, sizeof(payload), &reading);
        if (len < 0 || len >= (int)sizeof(payload)) {
            ESP_LOGE(TAG, "Payload build failed");
            continue;
        }
 
        int msg_id = esp_mqtt_client_publish(
            s_client,
            MQTT_TOPIC,
            payload,
            0,          // length 0 = use strlen
            MQTT_QOS,
            0           // not retained
        );
 
        if (msg_id >= 0) {
            ESP_LOGI(TAG, "Published msg_id=%d: %s", msg_id, payload);
        } else {
            ESP_LOGE(TAG, "Publish failed");
        }
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
 
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to broker");
            xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            xEventGroupClearBits(s_mqtt_event_group, MQTT_FAIL_BIT);
            break;
 
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected from broker");
            xEventGroupClearBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
 
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "Publish acknowledged, msg_id=%d", event->msg_id);
            break;
 
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            if (event->error_handle &&
                event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGE(TAG, "Transport error, socket errno=%d",
                         event->error_handle->esp_transport_sock_errno);
            }
            xEventGroupSetBits(s_mqtt_event_group, MQTT_FAIL_BIT);
            break;
 
        default:
            break;
    }
}

static int build_payload(char *buf, size_t buf_len, const processed_reading_t *r) {
    return snprintf(buf, buf_len,
        "{"
        "\"temp\":%.2f,"
        "\"humidity\":%.2f,"
        "\"eco2\":%.0f,"
        "\"tvoc\":%.0f,"
        "\"alerts\":{"
            "\"temp_high\":%s,"
            "\"temp_low\":%s,"
            "\"humidity_high\":%s,"
            "\"humidity_low\":%s,"
            "\"eco2_warn\":%s,"
            "\"eco2_bad\":%s,"
            "\"tvoc_warn\":%s,"
            "\"tvoc_bad\":%s"
        "},"
        "\"ts\":%llu"
        "}",
        r->temp_smoothed,
        r->humidity_smoothed,
        r->eco2_smoothed,
        r->tvoc_smoothed,
        r->alert_temp_high     ? "true" : "false",
        r->alert_temp_low      ? "true" : "false",
        r->alert_humidity_high ? "true" : "false",
        r->alert_humidity_low  ? "true" : "false",
        r->alert_eco2_warn     ? "true" : "false",
        r->alert_eco2_bad      ? "true" : "false",
        r->alert_tvoc_warn     ? "true" : "false",
        r->alert_tvoc_bad      ? "true" : "false",
        (unsigned long long)r->timestamp_ms
    );
}