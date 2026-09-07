/*
 * main.h
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_INCLUDE_MAIN_H_
#define MAIN_INCLUDE_MAIN_H_

#pragma once
#include <stdint.h>

// Struct konfigurasi pin BSP (Board Support Package)
typedef struct {
    int sda;
    int scl;
} bsp_i2c_pins_t;

typedef struct {
    int mosi;
    int miso;
    int sck;
    int cs;
} bsp_spi_pins_t;

typedef struct {
    int tx;
    int rx;
} bsp_uart_pins_t;

// Mapping Pin Perangkat Keras
#define I2C_PINS_LCD   ((bsp_i2c_pins_t){ .sda = 26, .scl = 25 })
#define I2C_PINS_IMU   ((bsp_i2c_pins_t){ .sda = 14, .scl = 12 })

#define UART_PINS_GPS  ((bsp_uart_pins_t){ .tx = 17, .rx = 16 })
#define SPI_PINS_LORA  ((bsp_spi_pins_t){ .mosi = 23, .miso = 19, .sck = 18, .cs = 5 })

#endif /* MAIN_INCLUDE_MAIN_H_ */
