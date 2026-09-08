/*
 * log_system.h
 *
 *  Created on: 8 Sept 2026
 *      Author: Rafdi
 */

#ifndef MAIN_SYSTEM_DATA_LOGGER_LOG_SYSTEM_H_
#define MAIN_SYSTEM_DATA_LOGGER_LOG_SYSTEM_H_

#include "data_logger.h"
#include "esp_err.h"

esp_err_t log_system_init(void);

esp_err_t log_system_push_data(const sensor_payload_t *data);

#endif /* MAIN_SYSTEM_DATA_LOGGER_LOG_SYSTEM_H_ */
