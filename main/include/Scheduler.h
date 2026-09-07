/*
 * Scheduler.h
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_SCHEDULER_H_
#define MAIN_SCHEDULER_H_

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Struct konfigurasi aktivasi Task per Priority
typedef struct {
    bool enable_prio_5_fast;    // Core 1 - Priority 5 (1ms & 5ms)
    bool enable_prio_4_high;    // Core 1 - Priority 4 (10ms, 15ms, 20ms)
    bool enable_prio_3_mid;     // Core 1 - Priority 3 (50ms & 100ms)
    bool enable_prio_2_low;     // Core 0 - Priority 2 (200ms, 300ms, 500ms)
    bool enable_prio_1_bg;      // Core 0 - Priority 1 (1000ms / Heartbeat)
} scheduler_config_t;

// Macro helper: Menyalakan semua priority level (Default)
#define SCHEDULER_CONFIG_ALL_ENABLE() ((scheduler_config_t){\
    .enable_prio_5_fast = true, \
    .enable_prio_4_high = true, \
    .enable_prio_3_mid  = true, \
    .enable_prio_2_low  = true, \
    .enable_prio_1_bg   = true  \
})

/**
 * @brief Inisialisasi dan jalankan scheduler FreeRTOS serta subsistem komunikasi.
 * @param config Pointer ke konfigurasi aktivasi task (jika NULL, otomatis mengaktifkan semua).
 */
void init_scheduler(const scheduler_config_t *config);

#endif /* MAIN_SCHEDULER_H_ */
