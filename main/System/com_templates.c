/*
 * com_templates.c
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Implementasi template boilerplate modular untuk 7 protokol komunikasi:
 *             1. UART / RS485
 *             2. Modbus RTU / TCP
 *             3. LoRa P2P (SX127x)
 *             4. WiFi HTTP REST Server
 *             5. MQTT Broker Pub/Sub
 *             6. CAN Bus / TWAI Controller
 *             7. BLE GATT Services
 */

#include "com_templates.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "COM_TEMPLATES";

esp_err_t com_tmpl_uart_init(bsp_uart_pins_t pins, uint32_t baud_rate) {
    ESP_LOGI(TAG, "[UART Template] Init TX: %d, RX: %d @ %lu bps", pins.tx, pins.rx, (unsigned long)baud_rate);
    return ESP_OK;
}

esp_err_t com_tmpl_uart_send(const void *data, size_t len) {
    ESP_LOGI(TAG, "[UART TX] Mengirim %u bytes balasan ke UART/RS485", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_uart_on_rx_bytes(const uint8_t *bytes, size_t len) {
    com_push_incoming_request(COM_IF_UART, bytes, len);
}

static uint8_t s_modbus_slave_id = 1;

esp_err_t com_tmpl_modbus_init(bsp_uart_pins_t pins, uint8_t slave_id, uint32_t baud_rate) {
    s_modbus_slave_id = slave_id;
    ESP_LOGI(TAG, "[Modbus Template] Init Slave ID: %u, TX: %d, RX: %d @ %lu bps", 
             slave_id, pins.tx, pins.rx, (unsigned long)baud_rate);
    return ESP_OK;
}

esp_err_t com_tmpl_modbus_send_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[Modbus TX] Mengirim %u bytes frame balasan Modbus", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count) {
    if (slave_id != s_modbus_slave_id && slave_id != 0) {
        return; 
    }

    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_MODBUS,
        .cmd_code = (fc == MODBUS_FC_READ_INPUT_REGS) ? CMD_REQ_IMU : CMD_REQ_ALL_SENSORS,
        .payload_len = 4
    };
    memcpy(&req.payload[0], &reg_addr, 2);
    memcpy(&req.payload[2], &reg_count, 2);
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));

    com_push_incoming_request(COM_IF_MODBUS, &req, sizeof(req));
}

esp_err_t com_tmpl_lora_init(bsp_spi_pins_t pins, long frequency_hz) {
    ESP_LOGI(TAG, "[LoRa Template] Init Frekuensi: %ld Hz, MOSI:%d MISO:%d SCK:%d CS:%d", 
             frequency_hz, pins.mosi, pins.miso, pins.sck, pins.cs);
    return ESP_OK;
}

esp_err_t com_tmpl_lora_send_packet(const void *data, size_t len) {
    ESP_LOGI(TAG, "[LoRa TX] Mengirim %u bytes paket radio RF", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_lora_on_rx_packet(const uint8_t *packet, size_t len) {
    com_push_incoming_request(COM_IF_LORA, packet, len);
}

esp_err_t com_tmpl_wifi_http_init(const char *ssid, const char *pass) {
    ESP_LOGI(TAG, "[WiFi/HTTP Template] Connect ke SSID: %s & start HTTP REST Server", ssid);
    return ESP_OK;
}

esp_err_t com_tmpl_http_send_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[HTTP Server TX] Balas JSON Response (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_http_on_endpoint_request(const char *uri, com_cmd_code_t cmd) {
    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_WIFI_HTTP,
        .cmd_code = (uint8_t)cmd,
        .payload_len = 0
    };
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));
    com_push_incoming_request(COM_IF_WIFI_HTTP, &req, sizeof(req));
}

esp_err_t com_tmpl_mqtt_init(const char *broker_uri, const char *client_id) {
    ESP_LOGI(TAG, "[MQTT Template] Connect ke broker: %s (Client: %s)", broker_uri, client_id);
    return ESP_OK;
}

esp_err_t com_tmpl_mqtt_publish_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[MQTT TX] Publish balasan ke topik 'esp32/response' (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_mqtt_on_message_received(const char *topic, const uint8_t *payload, size_t len) {
    ESP_LOGI(TAG, "[MQTT RX] Pesan masuk di topik: %s", topic);
    com_push_incoming_request(COM_IF_MQTT, payload, len);
}

esp_err_t com_tmpl_can_init(int tx_pin, int rx_pin, uint32_t baud_rate_kbps) {
    ESP_LOGI(TAG, "[CAN Bus Template] Init TWAI TX: %d, RX: %d @ %lu kbps", tx_pin, rx_pin, (unsigned long)baud_rate_kbps);
    return ESP_OK;
}

esp_err_t com_tmpl_can_send_frame(const void *data, size_t len) {
    ESP_LOGI(TAG, "[CAN Bus TX] Transmit CAN Frame (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_can_on_frame_received(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_CAN,
        .cmd_code = (uint8_t)(can_id & 0xFF),
        .payload_len = (dlc > 32) ? 32 : dlc
    };
    if (data && dlc > 0) {
        memcpy(req.payload, data, req.payload_len);
    }
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));

    com_push_incoming_request(COM_IF_CAN, &req, sizeof(req));
}

esp_err_t com_tmpl_ble_init(const char *device_name) {
    ESP_LOGI(TAG, "[BLE Template] Inisialisasi BLE GATT Service (%s)", device_name);
    return ESP_OK;
}

esp_err_t com_tmpl_ble_send_notify(const void *data, size_t len) {
    ESP_LOGI(TAG, "[BLE TX] Kirim GATT Characteristic Notify (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

void com_tmpl_ble_on_characteristic_write(const uint8_t *data, size_t len) {
    com_push_incoming_request(COM_IF_BLE, data, len);
}

void com_templates_register_all_handlers(void) {
    com_register_tx_handler(COM_IF_UART, com_tmpl_uart_send);
    com_register_tx_handler(COM_IF_MODBUS, com_tmpl_modbus_send_response);
    com_register_tx_handler(COM_IF_LORA, com_tmpl_lora_send_packet);
    com_register_tx_handler(COM_IF_WIFI_HTTP, com_tmpl_http_send_response);
    com_register_tx_handler(COM_IF_MQTT, com_tmpl_mqtt_publish_response);
    com_register_tx_handler(COM_IF_CAN, com_tmpl_can_send_frame);
    com_register_tx_handler(COM_IF_BLE, com_tmpl_ble_send_notify);

    ESP_LOGI(TAG, "Seluruh 7 Communication Protocol TX Handlers berhasil didaftarkan ke Com Hub.");
}
