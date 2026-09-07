/*
 * main.c
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#include "Scheduler.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void) {
    init_scheduler(NULL);

	// Priority
    /*
    scheduler_config_t rtos_cfg = {
        .enable_prio_5_fast = true,  // Aktifkan 1ms & 5ms (Core 1)
        .enable_prio_4_high = true,  // Aktifkan 10ms - 20ms (Core 1)
        .enable_prio_3_mid  = true,  // Aktifkan 50ms & 100ms (Core 1)
        .enable_prio_2_low  = false, // Matikan 200ms - 500ms (Core 0)
        .enable_prio_1_bg   = true   // Aktifkan 1000ms Heartbeat (Core 0)
    };
    init_scheduler(&rtos_cfg);
    */
}
 
 


