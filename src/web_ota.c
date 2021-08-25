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

#include "web_ota.h"
static const char *TAG = "scan";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

void wifi_init()
{
    // Initialize NVS - needed for WIFI
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    { // if no memory in NVS
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize the underlying TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // WIFI configuration and Init WiFi Alloc resource for WiFi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count)
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE; // list len
    *ap_count = 0;                            // zero the count
    memset(ap_info, 0, sizeof(number));       // zero the list

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true); // Scan all available APs
    // get the data
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(ap_count));

    ESP_ERROR_CHECK(esp_wifi_stop()); // close the wifi
}

void print_ap_info(wifi_ap_record_t *ap_info, uint16_t ap_count)
{
    for (size_t i = 0; i < ap_count; i++)
    {
        if (*((ap_info + i)->ssid) == 0)
        {
            break;
        }
        ESP_LOGI(TAG, "SSID \t\t%s", (ap_info + i)->ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", (ap_info + i)->rssi);
        print_auth_mode((ap_info + i)->authmode);
        ESP_LOGI(TAG, "Channel \t\t%d\n", (ap_info + i)->primary);
    }
}

static int s_retry_num = 0;
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ip = event->ip_info.ip;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool connect_to_sta(credential data)
{
    char *SSID = data.ssid;
    char *pass = data.password;
    s_wifi_event_group = xEventGroupCreate();
    bool toReturn;

    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {

            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    strcpy((char *)wifi_config.sta.ssid, (char *)SSID);
    strcpy((char *)wifi_config.sta.password, (char *)pass);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) //connected to ap
    {
        toReturn = true;
    }
    else if (bits & WIFI_FAIL_BIT) // TODO: start AP server here
    {
        toReturn = false; //Failed to connect to SSID
    }
    else
    {
        toReturn = false; //UNEXPECTED EVENT
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
    return toReturn;
}

esp_ip4_addr_t get_ip()
{
    return ip;
}

void start_ap(wifi_config_t ap_config)
{
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "esp",
            .ssid_len = strlen("esp"),
            .channel = 5,
            .password = "",
            .max_connection = 5,
            .authmode = WIFI_AUTH_OPEN},
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void print_auth_mode(int authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
        break;
    case WIFI_AUTH_WEP:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
        break;
    case WIFI_AUTH_WPA_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
        break;
    case WIFI_AUTH_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
        break;
    case WIFI_AUTH_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
        break;
    default:
        ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
        break;
    }
}

credential *read_wifi_data()
{
    //Opening Non-Volatile Storage (NVS) handle
    nvs_handle_t my_handle;
    esp_err_t err;
    credential *credentialData = malloc(sizeof(credential));

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI("NVS", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        //Reading data from NVS
        size_t required_size;
        nvs_get_str(my_handle, "credentialSsid", NULL, &required_size);
        char *tmpStrSsid = malloc(required_size);
        err = nvs_get_str(my_handle, "credentialSsid", tmpStrSsid, &required_size);

        nvs_get_str(my_handle, "credentialPass", NULL, &required_size);
        char *tmpStrPass = malloc(required_size);
        err = nvs_get_str(my_handle, "credentialPass", tmpStrPass, &required_size);

        switch (err)
        {
        case ESP_OK:
            credentialData->ssid = tmpStrSsid;
            credentialData->password = tmpStrPass;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI("NVS", "The value is not initialized yet!\n");
            free(tmpStrSsid);
            free(tmpStrPass);
            free(tmpStrSsid);
            nvs_close(my_handle);
            return NULL;
            break;
        default:
            ESP_LOGI("NVS", "Error (%s) reading!(%d)\n", esp_err_to_name(err), required_size);
            free(tmpStrSsid);
            free(tmpStrPass);
            free(tmpStrSsid);
            nvs_close(my_handle);
            return NULL;
        }

        // Close
        nvs_close(my_handle);
    }
    return credentialData;
}

void free_credential(credential *data)
{
    free(data->ssid);
    free(data->password);
    free(data);
}

bool write_wifi_data(credential data)
{
    //Opening Non-Volatile Storage (NVS) handle
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI("NVS", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        // Write
        printf("Updating restart counter in NVS ... ");
        err = nvs_set_str(my_handle, "credentialSsid", data.ssid);
        if (err != ESP_OK)
        {
            ESP_LOGI("NVS", "Failed in writing ssid!\n");
            return false;
        }

        err = nvs_set_str(my_handle, "credentialPass", data.password);
        if (err != ESP_OK)
        {
            ESP_LOGI("NVS", "Failed in writing password!\n");
            return false;
        }
        // Commit written value.
        err = nvs_commit(my_handle);
        if (err != ESP_OK)
        {
            ESP_LOGI("NVS", "Failed in commiting!\n");
            return false;
        }
        // Close
        nvs_close(my_handle);
    }
    return true;
}

//need to reset wifi
void restart_module()
{
    esp_restart();
}