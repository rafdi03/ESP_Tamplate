/*
 * task.h
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_INCLUDE_TASK_H_
#define MAIN_INCLUDE_TASK_H_

#pragma once
#include <stdint.h>
#include <stdbool.h>

// Callback function pointer
typedef void (*int_callback_t)(void);

void register_int_callback(int_callback_t cb);
void execute_int_callback(void);

// Inisialisasi awal aplikasi (dijalankan sekali sebelum scheduler loop)
void startup_application(void);

// =========================================================================
// TEMPLATE PRIORITY 5 (Critical Real-Time / Fast Cyclic - Core 1, Prio 5)
// =========================================================================
void job_1ms(void);    // Peruntukan: IMU fast reading, fast sensor update
void job_5ms(void);    // Peruntukan: Fast control loop / PID calculation

// =========================================================================
// TEMPLATE PRIORITY 4 (High Priority / Control - Core 1, Prio 4)
// =========================================================================
void job_10ms(void);   // Peruntukan: Sensor fusion, state estimation
void job_15ms(void);   // Peruntukan: Intermediate signal processing
void job_20ms(void);   // Peruntukan: Actuator / Servo / Motor update

// =========================================================================
// TEMPLATE PRIORITY 3 (Medium Priority / UI & Display - Core 1, Prio 3)
// =========================================================================
void job_50ms(void);   // Peruntukan: Button debouncing, keypad scanning
void job_100ms(void);  // Peruntukan: Display update (LCD / OLED), UI refresh

// =========================================================================
// TEMPLATE PRIORITY 2 (Low Priority / Telemetry & Comms - Core 0, Prio 2)
// =========================================================================
void job_200ms(void);  // Peruntukan: GPS NMEA parsing, LoRa TX/RX, Telemetry
void job_300ms(void);  // Peruntukan: Secondary communication protocol
void job_500ms(void);  // Peruntukan: Environment sensors (Temp/Humidity/BMP280)

// =========================================================================
// TEMPLATE PRIORITY 1 (Lowest / Background Monitor - Core 0, Prio 1)
// =========================================================================
void job_1000ms(void); // Peruntukan: Heartbeat LED, RAM/CPU stats, Watchdog kick

#endif /* MAIN_INCLUDE_TASK_H_ */
