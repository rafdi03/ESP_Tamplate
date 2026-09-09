#ifndef MAIN_INCLUDE_IMU_MPU_H_
#define MAIN_INCLUDE_IMU_MPU_H_

#pragma once
#include <stdint.h>
#include "esp_err.h"
#include "main.h"

// Tipe data struct data IMU
typedef struct {
    float accel_z;
    int16_t raw_accel_z;
} imu_data_t;

#define MPU6050_DEFAULT_ADDR    0x68

// Struct konfigurasi modular untuk MPU6050
typedef struct {
    bsp_i2c_pins_t pins;
    uint8_t i2c_addr;
} imu_mpu_config_t;

// Macro helper inisialisasi default menggunakan pin dari main.h
#define IMU_MPU_CONFIG_DEFAULT() ((imu_mpu_config_t){ .pins = I2C_PINS_IMU, .i2c_addr = MPU6050_DEFAULT_ADDR })

// API Driver IMU
esp_err_t imu_mpu_init(const imu_mpu_config_t *config);
esp_err_t imu_mpu_init_pins(bsp_i2c_pins_t pins);
void imu_mpu_update(void);
const imu_data_t* imu_mpu_get_data(void);

#endif /* MAIN_INCLUDE_IMU_MPU_H_ */