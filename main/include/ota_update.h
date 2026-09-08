/*
 * ota_update.h
 *
 *  Created on: 9 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Modul Over-The-Air (OTA) Firmware Update berbasis Web Server lokal.
 *             Memungkinkan flashing file .bin langsung via browser http://<IP>/update.
 */

#ifndef MAIN_INCLUDE_OTA_UPDATE_H_
#define MAIN_INCLUDE_OTA_UPDATE_H_

#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Inisialisasi Web Server internal untuk halaman OTA Firmware Update.
 * @param  None
 * @retval ESP_OK jika Web Server berhasil dijalankan, atau esp_err_t jika gagal.
 */
esp_err_t ota_update_init(void);

/**
 * @brief  Menghentikan Web Server OTA Firmware Update.
 * @param  None
 * @retval None
 */
void ota_update_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_INCLUDE_OTA_UPDATE_H_ */
