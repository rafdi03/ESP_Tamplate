/*
 * Scheduler.c
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"
#include "Scheduler.h"
#include "task.h"
#include "ringbuff_com.h"

static const char *TAG = "SCHEDULER";

static void init_task_watchdog(void) {
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 3000,
        .idle_core_mask = (1 << 0) | (1 << 1),
        .trigger_panic = true 
    };

    if (esp_task_wdt_reconfigure(&twdt_config) != ESP_OK) {
        esp_task_wdt_init(&twdt_config);
    }
}

static void vTaskPriority5(void *pvParameters) {
    esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t counter_1ms = 0;

    while (1) {
        job_1ms();
        if (++counter_1ms >= 5) {
            job_5ms();
            counter_1ms = 0;
        }

        esp_task_wdt_reset();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void vTaskPriority4(void *pvParameters) {
	esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t counter_5ms = 0;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(5));
        counter_5ms++;

        if (counter_5ms % 2 == 0) {
            job_10ms();
        }

        if (counter_5ms % 3 == 0) {
            job_15ms();
        }
		 
        if (counter_5ms % 4 == 0) {
            job_20ms();
        }

        if (counter_5ms >= 60) {
            counter_5ms = 0;
        }
		esp_task_wdt_reset();
    }
}

static void vTaskPriority3(void *pvParameters) {
	esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t counter_50ms = 0;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
        counter_50ms++;

        job_50ms();

        if (counter_50ms % 2 == 0) {
            job_100ms();
            counter_50ms = 0;
        }
		esp_task_wdt_reset();
    }
}

static void vTaskPriority2(void *pvParameters) {
	esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t counter_100ms = 0;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
        counter_100ms++;

        if (counter_100ms % 2 == 0) {
            job_200ms();
        }
		 
        if (counter_100ms % 3 == 0) {
            job_300ms();
        }

        if (counter_100ms % 5 == 0) {
            job_500ms();
        }

        if (counter_100ms >= 30) {
            counter_100ms = 0;
        }
		esp_task_wdt_reset();
    }
}

static void vTaskPriority1(void *pvParameters) {
    esp_task_wdt_add(NULL);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        job_1000ms();

        esp_task_wdt_reset();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void init_scheduler(const scheduler_config_t *config) {
    scheduler_config_t cfg;
    if (config == NULL) {
        cfg = SCHEDULER_CONFIG_ALL_ENABLE();
        config = &cfg;
    }
    init_task_watchdog();
    ringbuf_com_init(RINGBUF_COMM_DEFAULT_SIZE);
    startup_application();

    if (config->enable_prio_5_fast) {
        xTaskCreatePinnedToCore(vTaskPriority5, "TaskPrio5", 4096, NULL, 5, NULL, 1);
    }

    if (config->enable_prio_4_high) {
        xTaskCreatePinnedToCore(vTaskPriority4, "TaskPrio4", 4096, NULL, 4, NULL, 1);
    }

    if (config->enable_prio_3_mid) {
        xTaskCreatePinnedToCore(vTaskPriority3, "TaskPrio3", 4096, NULL, 3, NULL, 1);
    }

    if (config->enable_prio_2_low) {
        xTaskCreatePinnedToCore(vTaskPriority2, "TaskPrio2", 3072, NULL, 2, NULL, 0);
    }

    if (config->enable_prio_1_bg) {
        xTaskCreatePinnedToCore(vTaskPriority1, "TaskPrio1", 2048, NULL, 1, NULL, 0);
    }
}