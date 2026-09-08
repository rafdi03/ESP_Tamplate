/*
 * apps_log.c
 *
 *  Created on: 8 Sept 2026
 *      Author: Rafdi
 */


 #include "apps_log.h"
 #include "esp_log.h"
 #include "esp_partition.h"

 static const char *TAG = "APPS_LOG_BACKEND";
 static const esp_partition_t *s_log_partition = NULL;

 static esp_err_t spi_flash_init(void) {
     s_log_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "log_data");
     
     if (s_log_partition == NULL) {
         ESP_LOGE(TAG, "Partisi 'log_data' tidak ditemukan!");
         return ESP_FAIL;
     }
     
     ESP_LOGI(TAG, "SPI Flash Backend siap. Ukuran: %lu bytes", (unsigned long)s_log_partition->size);
     return ESP_OK;
 }

 static esp_err_t spi_flash_write(uint64_t address, const void *data, size_t length) {
     if (s_log_partition == NULL) return ESP_FAIL;
     
     return esp_partition_write(s_log_partition, (size_t)address, data, length);
 }

 static esp_err_t spi_flash_read(uint64_t address, void *data, size_t length) {
     if (s_log_partition == NULL) return ESP_FAIL;
     
     return esp_partition_read(s_log_partition, (size_t)address, data, length);
 }

 static esp_err_t spi_flash_erase(uint64_t address, size_t length) {
     if (s_log_partition == NULL) return ESP_FAIL;
     
     return esp_partition_erase_range(s_log_partition, (size_t)address, length);
 }

 static esp_err_t spi_flash_sync(void) {
	
     return ESP_OK; 
 }

 static esp_err_t spi_flash_get_info(storage_info_t *info) {
     if (info == NULL || s_log_partition == NULL) return ESP_ERR_INVALID_ARG;
     
     info->total_size = s_log_partition->size;
     info->write_block_size = 256;  
     info->erase_block_size = 4096; 
     info->requires_erase = true;   
     info->type = STORAGE_MEDIA_SPI_FLASH;
     
     return ESP_OK;
 }

 static storage_backend_t s_spi_flash_backend = {
     .init     = spi_flash_init,
     .write    = spi_flash_write,
     .read     = spi_flash_read,
     .erase    = spi_flash_erase,
     .sync     = spi_flash_sync,
     .get_info = spi_flash_get_info
 };

 storage_backend_t* apps_log_get_backend(void) {

     return &s_spi_flash_backend;
 }

