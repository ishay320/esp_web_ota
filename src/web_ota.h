#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include <string.h>
#include "esp_system.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define DEFAULT_SCAN_LIST_SIZE 15
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

void print_auth_mode(int authmode);
void wifi_init();
static void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count);
void print_ap_info(wifi_ap_record_t *ap_info, uint16_t ap_count);
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);
void wifi_init_sta(char *SSID, char *pass);
bool connect_to_ap(wifi_config_t sta_config);
void start_ap();
