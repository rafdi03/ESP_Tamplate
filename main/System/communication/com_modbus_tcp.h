/*
 * com_modbus_tcp.h
 *
 *  Created on: 14 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Modul komunikasi Modbus TCP Server (Port 502) & Modbus RTU.
 */

#ifndef MAIN_COMMUNICATION_COM_MODBUS_TCP_H_
#define MAIN_COMMUNICATION_COM_MODBUS_TCP_H_

#pragma once
#include "esp_err.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MODBUS_TCP_DEFAULT_PORT     502
#define MODBUS_DEFAULT_SLAVE_ID     1

typedef enum {
    MODBUS_FC_READ_COILS            = 0x01,
    MODBUS_FC_READ_DISCRETE_INPUTS  = 0x02,
    MODBUS_FC_READ_HOLDING_REGS     = 0x03,
    MODBUS_FC_READ_INPUT_REGS       = 0x04,
    MODBUS_FC_WRITE_SINGLE_COIL     = 0x05,
    MODBUS_FC_WRITE_SINGLE_REG      = 0x06,
    MODBUS_FC_WRITE_MULTI_REGS      = 0x10
} modbus_function_code_t;

/**
 * @brief Inisialisasi Modbus TCP Server pada port tertentu.
 *        Otomatis mendaftarkan TX handler ke Com Hub.
 * @param port Port TCP listening (default 502).
 * @param slave_id Unit Identifier Modbus.
 * @return ESP_OK jika server socket berhasil dibuat.
 */
esp_err_t com_modbus_tcp_init(uint16_t port, uint8_t slave_id);

/**
 * @brief Mengirimkan frame respons Modbus TCP kembali ke Master client.
 * @param data Buffer frame respons.
 * @param len Panjang data frame dalam bytes.
 * @return ESP_OK jika pengiriman sukses.
 */
esp_err_t com_modbus_tcp_send_response(const void *data, size_t len);

/**
 * @brief Handler pemroses frame request Modbus yang masuk untuk diteruskan ke Com Hub.
 * @param slave_id Unit ID target.
 * @param fc Function Code.
 * @param reg_addr Starting address register.
 * @param reg_count Number of registers.
 */
void com_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_COMMUNICATION_COM_MODBUS_TCP_H_ */
