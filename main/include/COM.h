/*
 * Com.h
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_INCLUDE_COM_H_
#define MAIN_INCLUDE_COM_H_

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "ringbuff_com.h"

// Definisi 8 Antarmuka Komunikasi Universal
typedef enum {
    COM_IF_UART = 0,    // UART (Serial PC / RS485 / Industrial Sensor)
    COM_IF_MODBUS,      // Modbus RTU (RS485) / Modbus TCP
    COM_IF_LORA,        // LoRa Module (SPI / SX1276 / SX1278 P2P)
    COM_IF_WIFI_HTTP,   // WiFi / WebServer REST API Endpoint
    COM_IF_MQTT,        // MQTT Broker Pub/Sub
    COM_IF_CAN,         // CAN Bus / TWAI Controller (Automotive)
    COM_IF_BLE,         // Bluetooth Low Energy GATT
    COM_IF_ESPNOW,      // ESP-NOW 2.4GHz Ultra-Fast Wireless (Peer-to-Peer / Broadcast)
    COM_IF_MAX
} com_interface_t;

// Definisi Kode Perintah Request Standar (Master -> ESP32)
typedef enum {
    CMD_REQ_PING            = 0x01, // Tes koneksi / Heartbeat
    CMD_REQ_ALL_SENSORS     = 0x02, // Request semua data sensor
    CMD_REQ_IMU             = 0x03, // Request data IMU MPU6050
    CMD_REQ_GPS             = 0x04, // Request data GPS
    CMD_REQ_SYS_STATUS      = 0x05, // Request data kesehatan baterai / status RTOS
    CMD_REQ_CUSTOM          = 0x0F  // Request kustom payload
} com_cmd_code_t;

// Format Header Request Masuk (Inbound Request Packet)
typedef struct {
    uint8_t preamble;           // COMM_PACKET_PREAMBLE (0xAA)
    uint8_t iface_source;       // com_interface_t (Sumber pengirim)
    uint8_t cmd_code;           // com_cmd_code_t (Perintah yang diminta)
    uint8_t payload_len;        // Panjang data tambahan (jika ada)
    uint8_t payload[32];        // Parameter request
    uint16_t crc16;             // CRC Checksum validasi request
} __attribute__((packed)) com_inbound_req_t;

// Tipe function pointer untuk callback pengiriman balasan ke hardware spesifik
typedef esp_err_t (*com_tx_handler_t)(const void *data, size_t len);

/**
 * @brief Inisialisasi subsistem komunikasi terpusat (Com Hub).
 * @return ESP_OK jika berhasil.
 */
esp_err_t com_init(void);

/**
 * @brief Registrasi fungsi driver TX untuk antarmuka tertentu (LoRa, WiFi, UART, Modbus, dll).
 * @param iface Antarmuka komunikasi target.
 * @param handler Function pointer callback pengiriman data.
 */
void com_register_tx_handler(com_interface_t iface, com_tx_handler_t handler);

/**
 * @brief Masukkan request baru ke antrian Com Hub dari Driver/ISR manapun (ISR & Thread-safe).
 * @param iface Antarmuka pengirim.
 * @param data Pointer ke paket request.
 * @param len Ukuran request dalam bytes.
 * @return true jika berhasil masuk antrian, false jika gagal.
 */
bool com_push_incoming_request(com_interface_t iface, const void *data, size_t len);

/**
 * @brief Eksekusi & Dispatcher request secepat kilat (Non-blocking).
 * @note WAJIB DIPANGGIL DI job_1ms() pada Core 1.
 *       Jika tidak ada request, fungsi selesai dalam < 1 mikrodetik.
 */
void com_update_1ms(void);

#endif /* MAIN_INCLUDE_COM_H_ */
