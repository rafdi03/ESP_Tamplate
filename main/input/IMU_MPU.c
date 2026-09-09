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
#define MPU_ACCEL_XOUT_H    0x3B

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

    // 2. Cek deteksi MPU6050 (Coba address 0x68 default, jika gagal coba 0x69)
    uint8_t try_addrs[2] = {config->i2c_addr, (config->i2c_addr == 0x68) ? 0x69 : 0x68};
    bool detected = false;
    uint8_t active_addr = 0;

    for (int i = 0; i < 2; i++) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = try_addrs[i],
            .scl_speed_hz = 100000, // 100kHz Standard Mode (Sangat stabil pada kabel jumper/breadboard)
        };
        
        if (s_mpu_dev != NULL) {
            i2c_master_bus_rm_device(s_mpu_dev);
            s_mpu_dev = NULL;
        }

        ret = i2c_master_bus_add_device(s_mpu_bus, &dev_cfg, &s_mpu_dev);
        if (ret != ESP_OK) continue;

        // 3. Bangunkan MPU dari Sleep (Tulis 0x00 ke register 0x6B)
        uint8_t wake_cmd[2] = {MPU_PWR_MGMT_1, 0x00};
        ret = i2c_master_transmit(s_mpu_dev, wake_cmd, sizeof(wake_cmd), pdMS_TO_TICKS(50));
        if (ret == ESP_OK) {
            detected = true;
            active_addr = try_addrs[i];
            break;
        }
    }

    if (detected) {
        ESP_LOGI(TAG, ">>> SUKSES: MPU6050 terdeteksi dan aktif pada SDA:%d, SCL:%d @ Addr 0x%02X <<<", 
                 config->pins.sda, config->pins.scl, active_addr);
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, ">>> GAGAL: MPU6050 TIDAK MERESPONS pada SDA:%d, SCL:%d! <<<", config->pins.sda, config->pins.scl);
        ESP_LOGW(TAG, "Petunjuk: Periksa kabel VCC (3.3V/5V), GND, SDA, SCL, atau ganti pin I2C.");
        if (s_mpu_dev != NULL) {
            i2c_master_bus_rm_device(s_mpu_dev);
            s_mpu_dev = NULL;
        }
        if (s_mpu_bus != NULL) {
            i2c_del_master_bus(s_mpu_bus);
            s_mpu_bus = NULL;
        }
        return ESP_ERR_NOT_FOUND;
    }
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

    uint8_t reg_addr = MPU_ACCEL_XOUT_H;
    uint8_t raw_buf[14];

    // Baca 14 byte sekaligus: Accel XYZ (6B) + Temp (2B) + Gyro XYZ (6B)
    esp_err_t ret = i2c_master_transmit_receive(
        s_mpu_dev,
        &reg_addr, 1,
        raw_buf, sizeof(raw_buf),
        pdMS_TO_TICKS(10)
    );

    if (ret == ESP_OK) {
        s_imu_data.raw_accel_x = (int16_t)((raw_buf[0] << 8) | raw_buf[1]);
        s_imu_data.raw_accel_y = (int16_t)((raw_buf[2] << 8) | raw_buf[3]);
        s_imu_data.raw_accel_z = (int16_t)((raw_buf[4] << 8) | raw_buf[5]);
        s_imu_data.raw_temp    = (int16_t)((raw_buf[6] << 8) | raw_buf[7]);
        s_imu_data.raw_gyro_x  = (int16_t)((raw_buf[8] << 8) | raw_buf[9]);
        s_imu_data.raw_gyro_y  = (int16_t)((raw_buf[10] << 8) | raw_buf[11]);
        s_imu_data.raw_gyro_z  = (int16_t)((raw_buf[12] << 8) | raw_buf[13]);

        // Konversi ke satuan fisik standar:
        // Accel +-2g = 16384 LSB/g
        s_imu_data.accel_x = (float)s_imu_data.raw_accel_x / 16384.0f;
        s_imu_data.accel_y = (float)s_imu_data.raw_accel_y / 16384.0f;
        s_imu_data.accel_z = (float)s_imu_data.raw_accel_z / 16384.0f;

        // Suhu Formula: (TEMP_OUT / 340) + 36.53 °C
        s_imu_data.temp_c  = ((float)s_imu_data.raw_temp / 340.0f) + 36.53f;

        // Gyro +-250 dps = 131 LSB/(deg/s)
        s_imu_data.gyro_x  = (float)s_imu_data.raw_gyro_x / 131.0f;
        s_imu_data.gyro_y  = (float)s_imu_data.raw_gyro_y / 131.0f;
        s_imu_data.gyro_z  = (float)s_imu_data.raw_gyro_z / 131.0f;
    }
}

const imu_data_t* imu_mpu_get_data(void) {
    return &s_imu_data;
}
