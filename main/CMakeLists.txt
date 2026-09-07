idf_component_register(SRCS "System/main.c" "System/Scheduler.c" "System/task.c" "input/LCD.c" "input/IMU_MPU.c" "System/ringbuff_com.c" "System/Com.c" "System/com_templates.c"
                       PRIV_REQUIRES spi_flash esp_driver_i2c esp_driver_gpio esp_timer esp_ringbuf esp_wifi nvs_flash esp_netif esp_event
                       INCLUDE_DIRS "include")