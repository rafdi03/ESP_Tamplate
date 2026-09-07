/*
 * Com.c
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 */

#include "Com.h"
#include "ringbuff_com.h"
#include "IMU_MPU.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "COM_HUB";
static com_tx_handler_t s_tx_handlers[COM_IF_MAX] = {NULL};

esp_err_t com_init(void) {
    ESP_LOGI(TAG, "Central Communication Hub siap (Ultra-Fast 1ms Dispatcher).");
    return ESP_OK;
}

void com_register_tx_handler(com_interface_t iface, com_tx_handler_t handler) {
    if (iface < COM_IF_MAX) {
        s_tx_handlers[iface] = handler;
    }
}

bool com_push_incoming_request(com_interface_t iface, const void *data, size_t len) {
    if (data == NULL || len == 0 || len > sizeof(com_inbound_req_t)) {
        return false;
    }

    com_inbound_req_t req;
    memcpy(&req, data, len);
    req.iface_source = (uint8_t)iface;

    return ringbuf_com_send(&req, sizeof(req), 0);
}

static void com_dispatch_response(const com_inbound_req_t *req) {
    if (req == NULL) return;

    comm_packet_t resp = {
        .preamble = COMM_PACKET_PREAMBLE,
        .msg_type = req->cmd_code | 0x80, 
        .payload_len = 0
    };

    switch (req->cmd_code) {
        case CMD_REQ_PING: {
            const char pong[] = "PONG";
            resp.payload_len = sizeof(pong);
            memcpy(resp.payload, pong, sizeof(pong));
            break;
        }

        case CMD_REQ_IMU: {
            const imu_data_t *imu = imu_mpu_get_data();
            resp.payload_len = sizeof(float);
            memcpy(resp.payload, &imu->accel_z, sizeof(float));
            break;
        }

        case CMD_REQ_ALL_SENSORS: {
            const imu_data_t *imu = imu_mpu_get_data();
            resp.payload_len = sizeof(float);
            memcpy(resp.payload, &imu->accel_z, sizeof(float));
            break;
        }

        default:
            resp.msg_type = 0xFF; 
            break;
    }
    resp.crc16 = comm_crc16(&resp, offsetof(comm_packet_t, crc16));
    com_interface_t src = (com_interface_t)req->iface_source;
    if (src < COM_IF_MAX && s_tx_handlers[src] != NULL) {
        s_tx_handlers[src](&resp, sizeof(resp));
    }
}

void com_update_1ms(void) {
    size_t item_size = 0;
    com_inbound_req_t *req = (com_inbound_req_t *)ringbuf_com_receive(&item_size, 0);

    if (req == NULL) {
        return;
    }
    if (req->preamble == COMM_PACKET_PREAMBLE &&
        comm_verify_crc16(req, offsetof(com_inbound_req_t, crc16), req->crc16)) {
        com_dispatch_response(req);
    } else {
        ESP_LOGW(TAG, "Request dari IF %u korup / CRC Mismatch!", req->iface_source);
    }
    ringbuf_com_free(req);
}
