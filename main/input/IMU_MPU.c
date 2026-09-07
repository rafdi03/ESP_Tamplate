/*
 * IMU_MPU.c
 *
 *  Created on: 4 Sept 2026
 *      Author: Rafdi
 */

#include "IMU_MPU.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MPU_PWR_MGMT_1      0x6B
#define MPU_ACCEL_ZOUT_H    0x3F

static const char *TAG = "MPU6050";
static i2c_master_dev_handle_t s_mpu_dev = NULL;
static i2c_master_bus_handle_t s_mpu_bus = NULL;

// Enkapsulasi privat
static imu_data_t s_imu_data = {0};

esp_err_t imu_mpu_init(const imu_mpu_config_t *config) {
    imu_mpu_config_t default_cfg;
    if (config == NULL) {
        default_cfg = IMU_MPU_CONFIG_DEFAULT();
        config = &default_cfg;
    }

    // 1. Inisialisasi Bus I2C
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1, 
        .sda_io_num = config->pins.sda,
        .scl_io_num = config->pins.scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &s_mpu_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Gagal membuat I2C bus untuk MPU6050!");
        return ret;
    }

    // 2. Pasang device MPU6050 ke bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->i2c_addr,
        .scl_speed_hz = 400000, // 400kHz Fast Mode
    };
    ret = i2c_master_bus_add_device(s_mpu_bus, &dev_cfg, &s_mpu_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Gagal menambahkan MPU6050 device ke bus!");
        return ret;
    }

    // 3. Bangunkan MPU dari Sleep (Tulis 0 ke register 0x6B)
    uint8_t wake_cmd[2] = {MPU_PWR_MGMT_1, 0x00};
    ret = i2c_master_transmit(s_mpu_dev, wake_cmd, sizeof(wake_cmd), pdMS_TO_TICKS(50));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "MPU6050 terdeteksi dan aktif.");
    } else {
        ESP_LOGE(TAG, "Gagal komunikasi dengan MPU6050!");
    }
    return ret;
}

esp_err_t imu_mpu_init_pins(bsp_i2c_pins_t pins) {
    imu_mpu_config_t cfg = {
        .pins = pins,
        .i2c_addr = MPU6050_DEFAULT_ADDR,
    };
    return imu_mpu_init(&cfg);
}

void imu_mpu_update(void) {
    if (s_mpu_dev == NULL) return;

    uint8_t reg_addr = MPU_ACCEL_ZOUT_H;
    uint8_t raw_buf[2];

    // Baca 2 byte dari register 0x3F (Z_HIGH) dan 0x40 (Z_LOW)
    esp_err_t ret = i2c_master_transmit_receive(
        s_mpu_dev,
        &reg_addr, 1,
        raw_buf, 2,
        pdMS_TO_TICKS(10)
    );

    if (ret == ESP_OK) {
        s_imu_data.raw_accel_z = (int16_t)((raw_buf[0] << 8) | raw_buf[1]);
        // Konversi ke G (Default range +-2g adalah 16384 LSB/g)
        s_imu_data.accel_z = (float)s_imu_data.raw_accel_z / 16384.0f;
    }
}

const imu_data_t* imu_mpu_get_data(void) {
    return &s_imu_data;
}
