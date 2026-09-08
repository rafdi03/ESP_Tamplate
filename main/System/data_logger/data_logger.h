/*
 * data_logger.h
 *
 *  Created on: 8 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Abstraksi sistem data logging, definisi skema metadata parameter,
 *             struct payload pengukuran (device_datalog_t), dan API publik.
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * KONFIGURASI DAN ENUMERASI MEDIA PENYIMPANAN
 * ========================================================================= */

#define NUMBER_OF_LOGDATA       7       /*!< Jumlah parameter channel per sample record */
#define DATALOG_CHUNK_MAGIC     0x55AA55AA  /*!< Magic word penanda awal chunk blok log */
#define DATALOG_CHUNK_END_MAGIC 0xAA55AA55  /*!< Magic word penanda akhir chunk blok log */

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

/**
 * @brief  Kontrak abstraksi antarmuka driver media penyimpanan (Storage Backend)
 */
typedef struct {
    esp_err_t (*init)(void);
    esp_err_t (*write)(uint64_t address, const void *data, size_t length);
    esp_err_t (*read)(uint64_t address, void *data, size_t length);
    esp_err_t (*erase)(uint64_t address, size_t length);
    esp_err_t (*sync)(void);
    esp_err_t (*get_info)(storage_info_t *info);
} storage_backend_t;

/* =========================================================================
 * STRUKTUR DATA LOGGING & METADATA SKEMA
 * ========================================================================= */

#pragma pack(push, 1)

/**
 * @brief  Struktur paket data record sensor (Sample Data Payload)
 * @note   Total 7 parameter (5 metrik float + 1 timestamp float + 1 sample counter uint32_t)
 */
typedef struct {
    float log_param1;   /*!< Parameter 1 */
    float log_param2;   /*!< Parameter 2 */
    float log_param3;   /*!< Parameter 3 */
    float log_param4;   /*!< Parameter 4 */
    float log_param5;   /*!< Parameter 5 */
    float timestamp;    /*!< Waktu realtime hardware timer (detik) */
    uint32_t countTIMER;/*!< Nomor urut sample counter (sample sequence) */
} device_datalog_t;

/**
 * @brief  Snapshot konfigurasi dan kalibrasi perangkat sebelum sesi logging
 */
typedef struct {
    uint32_t magic;             /*!< Magic identifier (0xC0FFEE01) */
    uint32_t sequence;          /*!< Sequence number konfigurasi */
    uint16_t imu_odr;           /*!< Output Data Rate IMU (Hz) */
    uint16_t imu_lpf;           /*!< Low Pass Filter IMU (Hz) */
    float accel_scale;          /*!< Skala sensitivitas accelerometer */
    uint32_t firmware_version;  /*!< Versi firmware aplikasi */
    uint32_t crc32;             /*!< CRC32 checksum validasi */
    uint32_t valid_marker;      /*!< Status validitas (0xA5A55A5A) */
} device_config_t;

/**
 * @brief  Header sesi rekaman data logger
 */
typedef struct {
    uint32_t session_id;        /*!< Identifikasi unik sesi perekaman */
    uint64_t start_address;     /*!< Alamat awal sektor data pada Flash */
    uint64_t end_address;       /*!< Alamat akhir sektor data pada Flash */
    uint32_t start_timestamp;   /*!< Waktu mulai sesi */
    uint32_t end_timestamp;     /*!< Waktu selesai sesi */
    uint8_t status;             /*!< Status sesi (0: Aktif, 1: Ditutup) */
    uint32_t crc32;             /*!< CRC32 checksum header */
} session_header_t;

/**
 * @brief  Header blok sektor penyimpanan (Chunk Block Header)
 */
typedef struct {
    uint32_t magic_start;       /*!< DATALOG_CHUNK_MAGIC (0x55AA55AA) */
    uint32_t sequence_num;      /*!< Sequence number sektor (1, 2, 3, ...) */
    uint16_t record_count;      /*!< Jumlah record device_datalog_t di dalam blok */
    uint16_t record_size;       /*!< Ukuran per record dalam bytes */
    uint32_t timestamp_start;   /*!< Timestamp sample pertama di blok ini */
} chunk_header_t;

/**
 * @brief  Footer penutup blok sektor penyimpanan (Chunk Block Footer)
 */
typedef struct {
    uint32_t crc32;             /*!< Checksum CRC32 dari seluruh isi blok */
    uint32_t magic_end;         /*!< DATALOG_CHUNK_END_MAGIC (0xAA55AA55) */
} chunk_footer_t;

#pragma pack(pop)

/**
 * @brief  Deskriptor Metadata Parameter (Kamus Skema Log)
 */
typedef struct {
    uint8_t index;              /*!< Nomor urut indeks parameter (0 .. NUMBER_OF_LOGDATA-1) */
    const char *name;           /*!< Nama string parameter (Contoh: "bias_dc_acc_x") */
    uint8_t name_length;        /*!< Panjang string nama parameter */
    float *data;                /*!< Pointer ke variabel data aktif */
    const char *unit;           /*!< Satuan pengukuran (Contoh: "m/s2", "rad/s", "deg") */
    uint8_t unit_length;        /*!< Panjang string satuan */
} datalog_param_desc_t;

/* =========================================================================
 * PUBLIC API DATA LOGGER
 * ========================================================================= */

/**
 * @brief  Inisialisasi subsistem Data Logger (Memory, Ping-Pong Buffer, & Storage Backend).
 * @param  None
 * @retval ESP_OK jika inisialisasi berhasil, atau esp_err_t jika gagal.
 */
esp_err_t data_logger_init(void);

/**
 * @brief  Menulis 1 record sample data sensor ke dalam active buffer log.
 * @param  data Pointer ke data struct device_datalog_t.
 * @retval ESP_OK jika berhasil disimpan ke buffer, atau kode error esp_err_t jika gagal.
 */
esp_err_t data_logger_write_record(const device_datalog_t *data);

/**
 * @brief  Fungsi eksekusi logging periodik (dipanggil dari job / task sensor).
 * @param  None
 * @retval None
 */
void Write_Datalog(void);

/**
 * @brief  Menyinkronkan seluruh sisa buffer di RAM ke media penyimpanan fisik.
 * @param  None
 * @retval ESP_OK jika sinkronisasi berhasil.
 */
esp_err_t data_logger_sync(void);

/**
 * @brief  Mengatur status proteksi pembacaan log (mencegah write saat sedang dump/baca log).
 * @param  is_reading Status true jika sedang membaca, false jika normal.
 * @retval None
 */
void data_logger_set_reading_state(bool is_reading);

/**
 * @brief  Mengecek apakah sistem sedang dalam proses pembacaan log.
 * @param  None
 * @retval true jika sedang membaca log, false jika tidak.
 */
bool data_logger_is_reading(void);

/**
 * @brief  Mengekspor string header format CSV berdasarkan tabel kamus parameter.
 * @param  out_buf Buffer penampung string output CSV.
 * @param  max_len Kapasitas buffer output.
 * @retval ESP_OK jika berhasil, atau kode error esp_err_t jika buffer tidak cukup.
 */
esp_err_t data_logger_export_csv_header(char *out_buf, size_t max_len);

/**
 * @brief  Mendapatkan pointer ke tabel skema metadata parameter yang terdaftar.
 * @param  num_entries Pointer untuk menerima jumlah parameter (output).
 * @retval Pointer ke array datalog_param_desc_t.
 */
const datalog_param_desc_t* data_logger_get_schema(size_t *num_entries);

#ifdef __cplusplus
}
#endif

#endif /* DATA_LOGGER_H */


