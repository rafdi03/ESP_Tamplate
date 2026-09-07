/*
 * task.c
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#include "task.h"
#include "esp_log.h"
#include <stddef.h>
#include <stdint.h>
#include "LCD.h"
#include "IMU_MPU.h"
#include "esp_timer.h"
#include "esp_intr_alloc.h"
#include "esp_attr.h"

static const char *TAG = "TASK_JOBS";

// Callback function pointer internal
static int_callback_t in_user_cb = NULL;

void register_int_callback(int_callback_t cb) {
    in_user_cb = cb;
}

void execute_int_callback(void) {
    if (in_user_cb != NULL) {
        in_user_cb();
    }
}

void startup_application(void) {

}

void job_1ms(void) {
  
}

void job_5ms(void) {
   
}

void job_10ms(void) {
   
}

void job_15ms(void) {
 
}

void job_20ms(void) {

}

void job_50ms(void) {
    
}

void job_100ms(void) {
   
}

void job_200ms(void) {

}

void job_300ms(void) {
  
}

void job_500ms(void) {
   
}

void job_1000ms(void) {
}

#if 0
static void IRAM_ATTR isr_level_1_3_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;

}

void init_interrupt_level_1_3(gpio_num_t gpio_pin, gpio_int_type_t intr_type) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = intr_type
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    gpio_isr_handler_add(gpio_pin, isr_level_1_3_handler, (void*) gpio_pin);
}
#endif

#if 0
static void IRAM_ATTR isr_level_4_5_handler(void* arg) {

}

void init_interrupt_level_4_5(gpio_num_t gpio_pin) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&io_conf);
    esp_intr_alloc(ETS_GPIO_INTR_SOURCE, ESP_INTR_FLAG_LEVEL5 | ESP_INTR_FLAG_IRAM, 
                   isr_level_4_5_handler, NULL, NULL);
}
#endif
