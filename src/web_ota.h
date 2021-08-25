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
static esp_ip4_addr_t ip;

// TODO: 1.get ap from mem
//       2.setup ap if fail
//       3.print ip

/**
 * @brief init the tcp/ip stack 
 * and free and init the NVS partition
 * 
 */
void wifi_init();

/**
 * @brief scans wifi
 * set: wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
 * 
 * @param ap_info 
 * @param ap_count 
 */
void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count);

/**
 * @brief print the list of ap
 * 
 * @param ap_info 
 * @param ap_count 
 */
void print_ap_info(wifi_ap_record_t *ap_info, uint16_t ap_count);

// in use in connect_to_sta
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

bool connect_to_sta(char *SSID, char *pass);

/**
 * @brief Get the ip object
 * 
 * in order to print use printf("IPSTR", IP2STR(ip));
 * @return esp_ip4_addr_t 
 */
esp_ip4_addr_t get_ip();

/**
 * @brief create open softAP
 * TODO: get ssid and pass from outside
 */
void start_ap(wifi_config_t ap_config);

/**
 * @brief print the given auth mode
 * 
 * @param authmode 
 */
void print_auth_mode(int authmode);
