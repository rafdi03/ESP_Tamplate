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

/* =========================================================================
 * 1. TEMPLATE UART / RS485
 * ========================================================================= */

/**
 * @brief  Inisialisasi peripheral UART/RS485 dengan konfigurasi baud rate dan pin BSP.
 * @param  pins Struktur konfigurasi pin TX dan RX dari main.h (bsp_uart_pins_t).
 * @param  baud_rate Kecepatan transmisi baud rate (contoh: 9600, 115200 bps).
 * @retval ESP_OK jika inisialisasi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_uart_init(bsp_uart_pins_t pins, uint32_t baud_rate);

/**
 * @brief  Mengirimkan byte data balasan melalui antarmuka UART/RS485 (TX Handler).
 * @param  data Pointer ke buffer data yang akan dikirimkan.
 * @param  len Jumlah byte data yang akan ditransmisikan.
 * @retval ESP_OK jika data berhasil dikirim, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_uart_send(const void *data, size_t len);

/**
 * @brief  Callback event penampung data byte masuk dari UART ISR/Driver ke Com Hub.
 * @param  bytes Pointer ke data byte yang diterima dari UART hardware.
 * @param  len Panjang data byte yang diterima.
 * @retval None
 */
void com_tmpl_uart_on_rx_bytes(const uint8_t *bytes, size_t len);

/* =========================================================================
 * 2. TEMPLATE MODBUS (RTU / RS485)
 * ========================================================================= */

/**
 * @brief  Daftar standar Function Code Modbus RTU / TCP.
 */
typedef enum {
    MODBUS_FC_READ_HOLDING_REGS = 0x03, /*!< Function Code 0x03: Read Holding Registers */
    MODBUS_FC_READ_INPUT_REGS   = 0x04, /*!< Function Code 0x04: Read Input Registers */
    MODBUS_FC_WRITE_SINGLE_REG  = 0x06, /*!< Function Code 0x06: Write Single Register */
    MODBUS_FC_WRITE_MULTI_REGS  = 0x10  /*!< Function Code 0x10: Write Multiple Registers */
} modbus_function_code_t;

/**
 * @brief  Inisialisasi stack Modbus RTU Slave pada transceiver RS485.
 * @param  pins Struktur konfigurasi pin hardware UART (bsp_uart_pins_t).
 * @param  slave_id Nomor identitas unik Modbus Slave (1 - 247).
 * @param  baud_rate Kecepatan transmisi serial Modbus (contoh: 9600 bps).
 * @retval ESP_OK jika inisialisasi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_modbus_init(bsp_uart_pins_t pins, uint8_t slave_id, uint32_t baud_rate);

/**
 * @brief  Mengirimkan paket respons frame Modbus ke Master melalui RS485.
 * @param  data Pointer ke buffer frame respons Modbus.
 * @param  len Ukuran frame respons dalam bytes.
 * @retval ESP_OK jika transmisi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_modbus_send_response(const void *data, size_t len);

/**
 * @brief  Callback pemrosesan frame query Modbus dari Master untuk diteruskan ke Com Hub.
 * @param  slave_id ID Slave target pada frame Modbus yang diterima.
 * @param  fc Function code Modbus yang diminta (modbus_function_code_t).
 * @param  reg_addr Alamat register awal (Starting Register Address).
 * @param  reg_count Jumlah register yang diminta (Number of Registers).
 * @retval None
 */
void com_tmpl_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count);

/* =========================================================================
 * 3. TEMPLATE LORA (P2P SX1276 / SX1278)
 * ========================================================================= */

/**
 * @brief  Inisialisasi modul radio LoRa P2P via antarmuka SPI.
 * @param  pins Struktur konfigurasi pin SPI (MOSI, MISO, SCK, CS) dari main.h.
 * @param  frequency_hz Frekuensi operasi RF dalam Hertz (contoh: 915000000 Hz / 433000000 Hz).
 * @retval ESP_OK jika inisialisasi chip radio berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_lora_init(bsp_spi_pins_t pins, long frequency_hz);

/**
 * @brief  Mengirimkan paket data biner melalui gelombang radio LoRa RF (TX Handler).
 * @param  data Pointer ke data paket payload.
 * @param  len Ukuran paket data dalam bytes.
 * @retval ESP_OK jika paket berhasil dikirim ke modul LoRa, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_lora_send_packet(const void *data, size_t len);

/**
 * @brief  Callback event penampung paket radio masuk dari LoRa Receiver ke Com Hub.
 * @param  packet Pointer ke buffer payload yang diterima dari sinyal radio.
 * @param  len Ukuran paket data yang diterima dalam bytes.
 * @retval None
 */
void com_tmpl_lora_on_rx_packet(const uint8_t *packet, size_t len);

/* =========================================================================
 * 4. TEMPLATE WIFI & HTTP REST SERVER
 * ========================================================================= */

/**
 * @brief  Inisialisasi stack WiFi Station (STA Mode) dan Web Server REST API.
 * @param  ssid Nama Access Point WiFi (SSID).
 * @param  pass Kata sandi jaringan WiFi.
 * @retval ESP_OK jika WiFi stack dan HTTP server berhasil dijalankan, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_wifi_http_init(const char *ssid, const char *pass);

/**
 * @brief  Mengirimkan respons HTTP JSON/Binary dari REST Server ke client web.
 * @param  data Pointer ke payload respons HTTP.
 * @param  len Panjang payload dalam bytes.
 * @retval ESP_OK jika pengiriman respons berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_http_send_response(const void *data, size_t len);

/**
 * @brief  Callback penangan permintaan HTTP GET/POST pada endpoint REST API tertentu.
 * @param  uri String jalur endpoint URI yang diakses client (contoh: "/api/v1/sensors").
 * @param  cmd Kode perintah Com Hub yang dipetakan (com_cmd_code_t).
 * @retval None
 */
void com_tmpl_http_on_endpoint_request(const char *uri, com_cmd_code_t cmd);

/* =========================================================================
 * 5. TEMPLATE MQTT BROKER PUB / SUB
 * ========================================================================= */

/**
 * @brief  Inisialisasi client MQTT dan menghubungkan ke broker IoT secara asinkron.
 * @param  broker_uri Alamat URI broker MQTT (contoh: "mqtt://broker.hivemq.com:1883").
 * @param  client_id Nama identitas unik client perangkat pada broker MQTT.
 * @retval ESP_OK jika client MQTT berhasil dibuat dan dimulai, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_mqtt_init(const char *broker_uri, const char *client_id);

/**
 * @brief  Mempublikasikan (Publish) data balasan ke topik response MQTT broker.
 * @param  data Pointer ke payload data yang akan dipublikasikan.
 * @param  len Ukuran data payload dalam bytes.
 * @retval ESP_OK jika publish berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_mqtt_publish_response(const void *data, size_t len);

/**
 * @brief  Callback penampung pesan request masuk dari subscription topik MQTT.
 * @param  topic Nama topik MQTT tempat pesan diterima.
 * @param  payload Pointer ke isi data pesan payload yang diterima.
 * @param  len Panjang data payload dalam bytes.
 * @retval None
 */
void com_tmpl_mqtt_on_message_received(const char *topic, const uint8_t *payload, size_t len);

/* =========================================================================
 * 6. TEMPLATE CAN BUS / TWAI (Automotive & Industrial)
 * ========================================================================= */

/**
 * @brief  Inisialisasi kontroler CAN Bus / TWAI hardware ESP32.
 * @param  tx_pin Nomor pin GPIO untuk jalur Transmit (CAN TX).
 * @param  rx_pin Nomor pin GPIO untuk jalur Receive (CAN RX).
 * @param  baud_rate_kbps Kecepatan bus CAN dalam kbps (contoh: 125, 250, 500, 1000 kbps).
 * @retval ESP_OK jika inisialisasi driver TWAI berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_can_init(int tx_pin, int rx_pin, uint32_t baud_rate_kbps);

/**
 * @brief  Mengirimkan frame paket CAN Bus ke jaringan otomotif/industri.
 * @param  data Pointer ke isi payload frame data CAN (maksimal 8 bytes).
 * @param  len Panjang payload frame data dalam bytes.
 * @retval ESP_OK jika transmisi frame berhasil di-enqueue, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_can_send_frame(const void *data, size_t len);

/**
 * @brief  Callback penerimaan frame CAN Bus dari kontroler TWAI ke Com Hub.
 * @param  can_id Identifier unik frame CAN (Standard 11-bit atau Extended 29-bit).
 * @param  data Pointer ke isi array payload frame data yang diterima.
 * @param  dlc Data Length Code (jumlah byte payload yang diterima, 0 - 8 bytes).
 * @retval None
 */
void com_tmpl_can_on_frame_received(uint32_t can_id, const uint8_t *data, uint8_t dlc);
void can_rx_poll(void);

/* =========================================================================
 * 7. TEMPLATE BLE (Bluetooth Low Energy GATT)
 * ========================================================================= */

/**
 * @brief  Inisialisasi stack Bluetooth Low Energy (BLE) sebagai GATT Server.
 * @param  device_name Nama perangkat BLE yang dipancarkan saat advertising.
 * @retval ESP_OK jika inisialisasi stack BLE dan service berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_ble_init(const char *device_name);

/**
 * @brief  Mengirimkan notifikasi data (GATT Characteristic Notification) ke client BLE yang terhubung.
 * @param  data Pointer ke buffer data yang akan dinotifikasi.
 * @param  len Panjang data yang akan dikirim dalam bytes.
 * @retval ESP_OK jika notifikasi berhasil dikirim, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_ble_send_notify(const void *data, size_t len);

/**
 * @brief  Callback event penampung penulisan data (Write Characteristic) dari client smartphone ke Com Hub.
 * @param  data Pointer ke data byte yang ditulis oleh client BLE.
 * @param  len Panjang data byte yang ditulis.
 * @retval None
 */
void com_tmpl_ble_on_characteristic_write(const uint8_t *data, size_t len);

/* =========================================================================
 * 8. TEMPLATE ESP-NOW (Two-Way Peer-to-Peer & Broadcast 2.4GHz)
 * ========================================================================= */

/**
 * @brief  Inisialisasi stack ESP-NOW 2-Way communication (Master/Slave).
 * @param  peer_mac Alamat 6-byte MAC target peer (atau NULL untuk broadcast FF:FF:FF:FF:FF:FF).
 * @param  channel Channel Wi-Fi operasi (1 - 13, default 1).
 * @retval ESP_OK jika inisialisasi ESP-NOW berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_init(const uint8_t *peer_mac, uint8_t channel);

/**
 * @brief  Menambahkan peer baru ke daftar komunikasi ESP-NOW secara dinamis.
 * @param  peer_mac Alamat 6-byte MAC target.
 * @param  channel Channel Wi-Fi target.
 * @param  encrypt Status enkripsi paket (true / false).
 * @retval ESP_OK jika peer berhasil ditambahkan, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_add_peer(const uint8_t *peer_mac, uint8_t channel, bool encrypt);

/**
 * @brief  Mengirimkan paket data biner melalui gelombang radio ESP-NOW (TX Handler).
 * @param  data Pointer ke buffer data yang akan dikirimkan (maksimal 250 bytes).
 * @param  len Panjang data yang akan dikirim dalam bytes.
 * @retval ESP_OK jika paket berhasil dikirim ke antrean radio, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_send(const void *data, size_t len);

/**
 * @brief  Callback penerimaan data masuk ESP-NOW dari perangkat peer/master lain ke Com Hub.
 * @param  src_mac Pointer ke alamat MAC pengirim paket (6 bytes).
 * @param  data Pointer ke payload data yang diterima.
 * @param  len Panjang data payload dalam bytes.
 * @retval None
 */
void com_tmpl_espnow_on_recv(const uint8_t *src_mac, const uint8_t *data, int len);

/* =========================================================================
 * MASTER REGISTRATION HELPER
 * ========================================================================= */

/**
 * @brief  Mendaftarkan seluruh callback pengiriman data (TX Handlers) protokol ke Com Hub.
 * @param  None
 * @retval None
 */
void com_templates_register_all_handlers(void);

#endif /* MAIN_INCLUDE_COM_TEMPLATES_H_ */
