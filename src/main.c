#include "web_ota.h"
#include "esp_netif_ip_addr.h"
#include <string.h>

void app_main(void)
{
    wifi_init();
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count;
    wifi_scan(ap_info, &ap_count);
    print_ap_info(ap_info, ap_count);
    credential_t data;
    data.ssid = "Bspot0856_2.4";
    data.password = "7C000856";
    credential_t *read = read_wifi_data();
    bool ap = connect_to_sta(*read);
    if (!ap)
    {
        wifi_config_t ap_config;
        start_ap(ap_config);
    }

    //write_wifi_data(data);
    credential_t *a;
    a = read_wifi_data();
    printf("%s , %s\n", (*a).ssid, (*a).password);
    esp_ip4_addr_t ip = get_ip();
    printf(IPSTR, IP2STR(&ip));
    printf("\n");


    start_webserver();
}

/**
 *  TODO:
 *      *ota
 *      *cleanup #1
 *      * get ap ip
 * 
 * DONE:
 *      *connect to sta with pass and ssid
 *      *join all the init of the wifi
 *      *scanning the wifi
 *      *print the scanned data
 *      *start ap - need to check more
 *      *save the ssid and pass to memory
 *      *restart the module after input
 *      *ip getting for showing
 *      *if connection fail start AP (sta connection returns bool)
 *      *start web interface for inserting ssid and pass
 *      *remove last credential

 */