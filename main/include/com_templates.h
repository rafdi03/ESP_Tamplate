/*
 * com_templates.h
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Template boilerplate siap pakai untuk 7 protokol komunikasi ESP32
 *             (UART, Modbus, LoRa, WiFi HTTP, MQTT, CAN Bus, BLE).
 */

#ifndef MAIN_INCLUDE_COM_TEMPLATES_H_
#define MAIN_INCLUDE_COM_TEMPLATES_H_

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "main.h"
#include "Com.h"

// =========================================================================
// 1. TEMPLATE UART / RS485
// =========================================================================
esp_err_t com_tmpl_uart_init(bsp_uart_pins_t pins, uint32_t baud_rate);
esp_err_t com_tmpl_uart_send(const void *data, size_t len);
void com_tmpl_uart_on_rx_bytes(const uint8_t *bytes, size_t len);

// =========================================================================
// 2. TEMPLATE MODBUS (RTU / RS485)
// =========================================================================
typedef enum {
    MODBUS_FC_READ_HOLDING_REGS = 0x03,
    MODBUS_FC_READ_INPUT_REGS   = 0x04,
    MODBUS_FC_WRITE_SINGLE_REG  = 0x06,
    MODBUS_FC_WRITE_MULTI_REGS  = 0x10,
} modbus_function_code_t;

esp_err_t com_tmpl_modbus_init(bsp_uart_pins_t pins, uint8_t slave_id, uint32_t baud_rate);
esp_err_t com_tmpl_modbus_send_response(const void *data, size_t len);
void com_tmpl_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count);

// =========================================================================
// 3. TEMPLATE LORA (P2P SX1276 / SX1278)
// =========================================================================
esp_err_t com_tmpl_lora_init(bsp_spi_pins_t pins, long frequency_hz);
esp_err_t com_tmpl_lora_send_packet(const void *data, size_t len);
void com_tmpl_lora_on_rx_packet(const uint8_t *packet, size_t len);

// =========================================================================
// 4. TEMPLATE WIFI & HTTP REST SERVER
// =========================================================================
esp_err_t com_tmpl_wifi_http_init(const char *ssid, const char *pass);
esp_err_t com_tmpl_http_send_response(const void *data, size_t len);
void com_tmpl_http_on_endpoint_request(const char *uri, com_cmd_code_t cmd);

// =========================================================================
// 5. TEMPLATE MQTT BROKER PUB / SUB
// =========================================================================
esp_err_t com_tmpl_mqtt_init(const char *broker_uri, const char *client_id);
esp_err_t com_tmpl_mqtt_publish_response(const void *data, size_t len);
void com_tmpl_mqtt_on_message_received(const char *topic, const uint8_t *payload, size_t len);

// =========================================================================
// 6. TEMPLATE CAN BUS / TWAI (Automotive & Industrial)
// =========================================================================
esp_err_t com_tmpl_can_init(int tx_pin, int rx_pin, uint32_t baud_rate_kbps);
esp_err_t com_tmpl_can_send_frame(const void *data, size_t len);
void com_tmpl_can_on_frame_received(uint32_t can_id, const uint8_t *data, uint8_t dlc);

// =========================================================================
// 7. TEMPLATE BLE (Bluetooth Low Energy GATT)
// =========================================================================
esp_err_t com_tmpl_ble_init(const char *device_name);
esp_err_t com_tmpl_ble_send_notify(const void *data, size_t len);
void com_tmpl_ble_on_characteristic_write(const uint8_t *data, size_t len);

// =========================================================================
// MASTER HELPER: DAFTARKAN SELURUH CALLBACK HANDLER KE COM HUB
// =========================================================================
void com_templates_register_all_handlers(void);

#endif /* MAIN_INCLUDE_COM_TEMPLATES_H_ */
