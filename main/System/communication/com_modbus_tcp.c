/*
 * com_modbus_tcp.c
 *
 *  Created on: 14 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Implementasi Modbus TCP Server terisolasi dengan auto-routing ke Com Hub.
 */

#include "com_modbus_tcp.h"
#include "COM.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "COM_MODBUS_TCP";
static uint8_t s_modbus_slave_id = MODBUS_DEFAULT_SLAVE_ID;
static uint16_t s_modbus_port = MODBUS_TCP_DEFAULT_PORT;
static bool s_modbus_initialized = false;

esp_err_t com_modbus_tcp_init(uint16_t port, uint8_t slave_id) {
    s_modbus_port = (port == 0) ? MODBUS_TCP_DEFAULT_PORT : port;
    s_modbus_slave_id = (slave_id == 0) ? MODBUS_DEFAULT_SLAVE_ID : slave_id;

    // Daftarkan fungsi transmisi Modbus ke Central Com Hub
    com_register_tx_handler(COM_IF_MODBUS, com_modbus_tcp_send_response);

    s_modbus_initialized = true;
    ESP_LOGI(TAG, "Modbus TCP Server siap pada Port %u (Slave Unit ID: %u)", s_modbus_port, s_modbus_slave_id);
    return ESP_OK;
}

esp_err_t com_modbus_tcp_send_response(const void *data, size_t len) {
    if (!s_modbus_initialized || data == NULL || len == 0) {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "[Modbus TX] Mengirim %u bytes frame respons Modbus TCP", (unsigned int)len);
    return ESP_OK;
}

void com_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count) {
    if (slave_id != s_modbus_slave_id && slave_id != 0) {
        return; 
    }

    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .cmd_code = (fc == MODBUS_FC_READ_INPUT_REGS) ? CMD_REQ_IMU : CMD_REQ_ALL_SENSORS,
        .payload_len = 4,
        .iface_source = (uint8_t)COM_IF_MODBUS
    };
    memcpy(&req.payload[0], &reg_addr, 2);
    memcpy(&req.payload[2], &reg_count, 2);
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));

    com_push_incoming_request(COM_IF_MODBUS, &req, sizeof(req));
}
