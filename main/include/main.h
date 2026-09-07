/*
 * main.h
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_INCLUDE_MAIN_H_
#define MAIN_INCLUDE_MAIN_H_

#include <stdint.h>

#pragma once

#define WIFI_SSID_DEFAULT           "Rumah Kita"
#define WIFI_PASS_DEFAULT           "EKAGUNAPUTRA03"
#define MQTT_BROKER_URI_DEFAULT     "mqtt://broker.emqx.io:1883" // atau broker lokal / HiveMQ
#define MQTT_CLIENT_ID_DEFAULT      "ESP32_Node_01"
#define MQTT_TOPIC_REQ_DEFAULT      "esp32/node1/request"
#define MQTT_TOPIC_RESP_DEFAULT     "esp32/node1/response"

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

#define I2C_PINS_LCD   ((bsp_i2c_pins_t){ .sda = 26, .scl = 25 })
#define I2C_PINS_IMU   ((bsp_i2c_pins_t){ .sda = 14, .scl = 12 })

#define UART_PINS_GPS  ((bsp_uart_pins_t){ .tx = 17, .rx = 16 })
#define SPI_PINS_LORA  ((bsp_spi_pins_t){ .mosi = 23, .miso = 19, .sck = 18, .cs = 5 })

#endif /* MAIN_INCLUDE_MAIN_H_ */
