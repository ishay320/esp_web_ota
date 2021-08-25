#include "web_ota.h"

void app_main(void)
{
    wifi_init();
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count;
    wifi_scan(ap_info, &ap_count);
    print_ap_info(ap_info, ap_count);

    connect_to_sta("Bspot0856_2.4", "7C000856");

    //TODO: replace with reading from mem
    // wifi_config_t sta_config = {
    //     .sta = {.ssid = "Bspot0856_2.4", .password = "7C000856"},
    // };
    //connect_to_ap(sta_config);
    //start_ap();
    // ESP_LOGI(TAG, "*********** connected ********************");
}

/**
 *  TODO:
 *      *if connection fail start AP
 *      *start web interface for inserting ssid and pass
 *      *ota
 *      *save the ssid and pass to memory
 * 
 * DONE:
 *      *connect to sta with pass and ssid
 *      *join all the init of the wifi
 *      *scanning the wifi (have bugs- work only when single)
 *      *print the scanned data
 *      *start ap - need to check more
 * 
 */