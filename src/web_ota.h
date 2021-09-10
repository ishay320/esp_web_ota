#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_server.h"

#define DEFAULT_SCAN_LIST_SIZE 15
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
esp_ip4_addr_t ip;

/**
 * @brief struct that holds the credential to the wifi network
 * 
 */
typedef struct credential
{
    char *ssid;
    char *password;
} credential_t;

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

bool connect_to_sta(credential_t data);

/**
 * @brief Get the ip object
 * 
 * in order to print use:  
 *  esp_ip4_addr_t ip = get_ip();
 *  printf(IPSTR, IP2STR(&ip));
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

/**
 * @brief read the ssid and pasword from NVS
 * !dont forget to free all after!
 * @return credential_t* that hold the ssid and pass
 * @return NULL if failed
 */
credential_t *read_wifi_data();

/**
 * @brief free the credential object from memory
 * 
 * @param data is the object to free
 */
void free_credential(credential_t *data);

/**
 * @brief write the ssid and password to NVS
 * 
 * @param data 
 * @return true if secceded
 * @return false if failed
 */
bool write_wifi_data(credential_t data);

void restart_module();

httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);
char *get_data_ptr(char *data, const int dataPos, int *keySize, int *valueSize);
