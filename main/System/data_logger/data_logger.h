/*
 * data_logger.h
 *
 *  Created on: 8 Sept 2026
 *      Author: Rafdi
 */

 #ifndef DATA_LOGGER_H
 #define DATA_LOGGER_H

 #include <stdint.h>
 #include <stdbool.h>
 #include <stddef.h>
 #include "esp_err.h"

 typedef enum {
     STORAGE_MEDIA_NONE = 0,
     STORAGE_MEDIA_SPI_FLASH,
     STORAGE_MEDIA_SD_CARD
 } storage_media_t;

 typedef struct {
     uint64_t total_size;
     uint32_t write_block_size;
     uint32_t erase_block_size;
     bool requires_erase;
     storage_media_t type;
 } storage_info_t;

 typedef struct {
     esp_err_t (*init)(void);
     esp_err_t (*write)(uint64_t address, const void *data, size_t length);
     esp_err_t (*read)(uint64_t address, void *data, size_t length);
     esp_err_t (*erase)(uint64_t address, size_t length);
     esp_err_t (*sync)(void);
     esp_err_t (*get_info)(storage_info_t *info);
 } storage_backend_t;

 #pragma pack(push, 1)

 typedef struct {
     uint32_t magic;
     uint32_t sequence;
     uint16_t imu_odr;
     uint16_t imu_lpf;
     float accel_scale;
     uint32_t firmware_version;
     uint32_t crc32;
     uint32_t valid_marker;
 } device_config_t;

 typedef struct {
     uint32_t session_id;
     uint64_t start_address;
     uint64_t end_address;
     uint32_t start_timestamp;
     uint32_t end_timestamp;
     uint8_t status;
     uint32_t crc32;
 } session_header_t;

 typedef struct {
     uint32_t timestamp;
     float accel_z;
 } sensor_payload_t;

 typedef struct {
     uint32_t magic_start;
     uint32_t sequence_num;
     uint16_t payload_count;
 } chunk_header_t;

 #pragma pack(pop)

 esp_err_t data_logger_init(void);
 esp_err_t data_logger_push_sensor(const sensor_payload_t *data);
 esp_err_t data_logger_sync(void);

 #endif

