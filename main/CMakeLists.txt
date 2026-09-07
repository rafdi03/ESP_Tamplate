idf_component_register(SRCS "System/main.c" "System/Scheduler.c" "System/task.c" "input/LCD.c" "input/IMU_MPU.c"
                       PRIV_REQUIRES spi_flash esp_driver_i2c esp_timer
                       INCLUDE_DIRS "include")
