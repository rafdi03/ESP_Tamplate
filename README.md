# ESP32 Real-Time Firmware Framework

> **A production-ready, modular ESP32 firmware framework** built on top of ESP-IDF and FreeRTOS, featuring a hierarchical real-time scheduler, a unified multi-protocol communication hub (8 interfaces), over-the-air (OTA) firmware updates, and a non-blocking dual-buffer data logging engine — all designed for industrial IoT, robotics, and embedded systems development.

---

## Table of Contents

1. [Overview](#overview)
2. [Key Features](#key-features)
3. [Hardware & Prerequisites](#hardware--prerequisites)
4. [Project Structure](#project-structure)
5. [Architecture Overview](#architecture-overview)
6. [System Components](#system-components)
   - [Scheduler](#1-scheduler-freertos-task-engine)
   - [Task Job Slots](#2-task-job-slots)
   - [Communication Hub](#3-communication-hub-com-hub)
   - [Ring Buffer Layer](#4-ring-buffer-communication-layer)
   - [Protocol Templates](#5-communication-protocol-templates)
   - [OTA Updater](#6-ota-firmware-updater-web-based)
   - [Data Logger](#7-data-logger-system)
   - [Input Drivers](#8-input-drivers-sensors--display)
7. [Pin Map (BSP)](#pin-map-bsp)
8. [Flash Partition Table](#flash-partition-table)
9. [Getting Started](#getting-started)
10. [Configuration Reference](#configuration-reference)
11. [Communication Protocols](#communication-protocols)
12. [OTA Update Workflow](#ota-update-workflow)
13. [Data Logger Guide](#data-logger-guide)
14. [Testing](#testing)
15. [Extending the Framework](#extending-the-framework)
16. [File Reference](#file-reference)

---

## Overview

This project is a **fully-featured ESP32 embedded firmware framework** that goes far beyond a minimal "hello world". It provides a carefully architected foundation for real-time embedded applications that require:

- **Deterministic timing** across multiple periodic task slots (1 ms to 1000 ms)
- **Unified multi-protocol communication** dispatched from a single central hub
- **ISR-safe, zero-copy data transfer** via FreeRTOS Ring Buffers
- **Browser-based OTA firmware updates** triggered automatically after Wi-Fi connects
- **Non-blocking high-speed data logging** to SPI Flash using a Ping-Pong double-buffer architecture
- **CRC-verified packet integrity** on every message exchange

The framework is structured so that a developer only needs to fill in the `job_Xms()` functions in `task.c` and optionally initialise any of the 8 ready-made communication protocol templates.

---

## Key Features

| Feature | Details |
|---|---|
| **Real-Time Scheduler** | 5 FreeRTOS priority levels, dual-core pinned, 1 ms resolution |
| **Task Watchdog** | 3-second WDT with `trigger_panic = true` on both cores |
| **Communication Hub** | 8 protocol interfaces unified under one dispatcher |
| **Ring Buffer IPC** | FreeRTOS `RINGBUF_TYPE_NOSPLIT`, ISR-safe, 2 KB default |
| **CRC Validation** | CRC-16 (CCITT-LE) for packets, CRC-32 (IEEE 802.3-LE) for log sectors |
| **OTA Update** | Browser drag-and-drop `.bin` upload, live progress bar, auto-reboot |
| **Data Logger** | Ping-Pong 4 KB double-buffer, background Core-0 writer task |
| **Boot Recovery** | Scans Flash sectors for last valid CRC-verified chunk on every boot |
| **IMU Driver** | MPU6050 via I2C, 400 kHz Fast Mode, encapsulated read/get API |
| **LCD Driver** | HD44780-compatible LCD via PCF8574 I2C expander, 4-bit mode |
| **Wi-Fi Auto-Reconnect** | STA mode with automatic reconnect on disconnect event |
| **MQTT (ready)** | Template stub ready to connect to any broker |
| **CAN / TWAI** | Full TWAI driver init, non-blocking `can_rx_poll()` at 1 ms |
| **ESP-NOW** | Two-way P2P + broadcast with auto peer-registration on first RX |
| **LoRa (ready)** | SPI-based SX127x template stub |
| **BLE GATT (ready)** | BLE advertising + GATT characteristic template stub |
| **Modbus RTU (ready)** | Slave frame parser + register-read dispatcher |

---

## Hardware & Prerequisites

### Target MCU

- **ESP32** (Dual-Core Xtensa LX6, 240 MHz)
- Real-time tasks pinned to **Core 1**, Background tasks pinned to **Core 0**

### Required Software

| Tool | Version |
|---|---|
| ESP-IDF | >= 5.x (CMake-based) |
| Python | >= 3.8 (for `idf.py` and tests) |
| CMake | >= 3.22 |
| Ninja | Any modern version |

### Optional Hardware

| Module | Interface | Purpose |
|---|---|---|
| MPU6050 IMU | I2C (SDA: GPIO14, SCL: GPIO12) | Accelerometer/Gyroscope |
| HD44780 LCD (via PCF8574) | I2C (SDA: GPIO26, SCL: GPIO25) | Display |
| SX1276/SX1278 LoRa | SPI (MOSI:23, MISO:19, SCK:18, CS:5) | Long-range RF |
| Any CAN transceiver | TWAI (TX: GPIO21, RX: GPIO22) | CAN Bus |
| GPS module | UART (TX: GPIO17, RX: GPIO16) | GPS data |

---

## Project Structure

```
hello_world/
├── CMakeLists.txt              # Root CMake: IDF project entry point
├── partitions.csv              # Custom 4MB flash partition table (OTA + Data)
├── sdkconfig.defaults          # Default SDK config overrides
├── pytest_hello_world.py       # pytest-embedded integration tests
│
└── main/
    ├── CMakeLists.txt          # Component registration: all sources + dependencies
    │
    ├── include/                # Public header files (all modules)
    │   ├── main.h              # BSP pin definitions, Wi-Fi/MQTT defaults
    │   ├── Scheduler.h         # scheduler_config_t, init_scheduler()
    │   ├── task.h              # All job_Xms() prototypes
    │   ├── COM.h               # Com Hub: com_interface_t, com_cmd_code_t, API
    │   ├── com_templates.h     # 8-protocol template function declarations
    │   ├── ringbuff_com.h      # Ring buffer API + CRC utilities
    │   ├── IMU_MPU.h           # MPU6050 driver API
    │   └── LCD.h               # LCD driver API
    │
    ├── System/                 # Core system modules
    │   ├── main.c              # app_main(): calls init_scheduler()
    │   ├── Scheduler.c         # FreeRTOS task creation + WDT setup
    │   ├── task.c              # User job slot implementations
    │   ├── COM.c               # Com Hub dispatcher logic
    │   ├── com_templates.c     # 8-protocol template implementations
    │   ├── ringbuff_com.c      # Ring buffer + CRC implementation
    │   ├── ota_update.c        # HTTP web server OTA flash handler
    │   │
    │   └── data_logger/        # Data logging subsystem
    │       ├── data_logger.c   # Public logger API + parameter schema
    │       ├── data_logger.h   # Data structures + API declarations
    │       ├── log_system.c    # Ping-Pong engine + boot recovery + writer task
    │       ├── log_system.h    # Log system API
    │       ├── app_log.c       # Storage backend abstraction (SPI Flash / SD)
    │       └── app_log.h       # Backend registration API
    │
    └── input/                  # Peripheral input drivers
        ├── IMU_MPU.c           # MPU6050 I2C driver
        └── LCD.c               # HD44780 LCD I2C driver
```

---

## Architecture Overview

```
+---------------------------------------------------------------------+
|                         app_main()                                  |
|                    +-- init_scheduler(NULL)                         |
+----------------------------+----------------------------------------+
                             |
              +--------------v--------------+
              |        Scheduler.c          |
              |  +---------------------+    |
              |  |  Task Watchdog (3s) |    |
              |  +---------------------+    |
              |  +---------------------+    |
              |  | startup_application |    |  <- com_init()
              |  +---------------------+    |  <- com_templates_register_all_handlers()
              |  +---------------------+    |  <- com_tmpl_wifi_http_init()
              |  |  ringbuf_com_init() |    |
              |  +---------------------+    |
              +--------------+--------------+
                             | Creates 5 FreeRTOS Tasks
        +--------------------+--------------------+
        |                    |                    |
   Core 1 (Prio 5)      Core 1 (Prio 4)      Core 1 (Prio 3)
  +-------------+      +-------------+      +-------------+
  | vTaskPrio5  |      | vTaskPrio4  |      | vTaskPrio3  |
  |  job_1ms()  |      |  job_10ms() |      |  job_50ms() |
  |  job_5ms()  |      |  job_15ms() |      | job_100ms() |
  +-------------+      |  job_20ms() |      +-------------+
                       +-------------+

   Core 0 (Prio 2)      Core 0 (Prio 1)
  +-------------+      +-------------+
  | vTaskPrio2  |      | vTaskPrio1  |
  | job_200ms() |      |job_1000ms() |
  | job_300ms() |      +-------------+
  | job_500ms() |
  +-------------+

  +======================================+
  |     COM HUB (com_update_1ms)         |
  |  +------+  +--------+  +-------+    |
  |  | UART |  |  LoRa  |  |  CAN  |    |
  |  +------+  +--------+  +-------+    |
  |  +------+  +--------+  +-------+    |
  |  | MQTT |  |  WiFi  |  |  BLE  |    |
  |  +------+  +--------+  +-------+    |
  |  +------+  +--------+               |
  |  |Modbus|  |ESP-NOW |               |
  |  +------+  +--------+               |
  |       v  RingBuffer (2KB)  ^        |
  |  Dispatcher -> com_dispatch_response |
  +======================================+
```

---

## System Components

### 1. Scheduler (FreeRTOS Task Engine)

**Files:** `Scheduler.c`, `Scheduler.h`

The scheduler is the heart of the firmware. It creates up to **5 pinned FreeRTOS tasks**, each with a specific priority level and CPU core affinity.

#### Task Priority Map

| Task | Core | Priority | Period | Job Functions |
|---|---|---|---|---|
| `vTaskPriority5` | Core 1 | 5 (highest) | 1 ms base | `job_1ms()`, `job_5ms()` |
| `vTaskPriority4` | Core 1 | 4 | 5 ms base | `job_10ms()`, `job_15ms()`, `job_20ms()` |
| `vTaskPriority3` | Core 1 | 3 | 50 ms base | `job_50ms()`, `job_100ms()` |
| `vTaskPriority2` | Core 0 | 2 | 100 ms base | `job_200ms()`, `job_300ms()`, `job_500ms()` |
| `vTaskPriority1` | Core 0 | 1 (lowest) | 1000 ms | `job_1000ms()` |

> **Design Rationale:** Core 1 handles all time-critical and sensor/control tasks. Core 0 handles slower telemetry, communication, and background work.

#### Task Watchdog

- **Timeout:** 3000 ms
- **Monitored cores:** Both Core 0 and Core 1
- **Panic on timeout:** `true` — triggers a full system panic + stack trace

Every task calls `esp_task_wdt_reset()` at the end of each loop iteration.

#### Scheduler Configuration

```c
typedef struct {
    bool enable_prio_5_fast;  // Core 1 - 1ms & 5ms (fast real-time)
    bool enable_prio_4_high;  // Core 1 - 10ms, 15ms, 20ms (control)
    bool enable_prio_3_mid;   // Core 1 - 50ms & 100ms (UI/display)
    bool enable_prio_2_low;   // Core 0 - 200ms, 300ms, 500ms (telemetry)
    bool enable_prio_1_bg;    // Core 0 - 1000ms (heartbeat/monitoring)
} scheduler_config_t;
```

**Default — enable all priority groups:**

```c
init_scheduler(NULL);  // NULL uses SCHEDULER_CONFIG_ALL_ENABLE() macro
```

**Custom — selectively disable groups to save CPU:**

```c
scheduler_config_t cfg = {
    .enable_prio_5_fast = true,
    .enable_prio_4_high = true,
    .enable_prio_3_mid  = true,
    .enable_prio_2_low  = false,  // Disable 200-500 ms tasks
    .enable_prio_1_bg   = true
};
init_scheduler(&cfg);
```

---

### 2. Task Job Slots

**Files:** `task.c`, `task.h`

`task.c` is **the main user-facing file** where you implement your application logic.

#### Currently Active Job Implementations

| Function | Period | Active Logic |
|---|---|---|
| `job_1ms()` | 1 ms | `com_update_1ms()` — dispatch Com Hub; `can_rx_poll()` — poll CAN TWAI |
| `job_5ms()` | 5 ms | *(empty — reserved for PID / fast control loops)* |
| `job_10ms()` | 10 ms | *(empty — reserved for sensor fusion)* |
| `job_15ms()` | 15 ms | *(empty — reserved for intermediate signal processing)* |
| `job_20ms()` | 20 ms | *(empty — reserved for actuator / servo update)* |
| `job_50ms()` | 50 ms | *(empty — reserved for button debounce)* |
| `job_100ms()` | 100 ms | *(empty — reserved for LCD / OLED update)* |
| `job_200ms()` | 200 ms | *(empty — reserved for GPS / LoRa TX-RX)* |
| `job_300ms()` | 300 ms | *(empty — reserved for secondary comms)* |
| `job_500ms()` | 500 ms | *(empty — reserved for temperature/humidity sensors)* |
| `job_1000ms()` | 1000 ms | `ringbuf_com_print_stats()` — print ring buffer health |

#### Application Startup

`startup_application()` runs **once** before the scheduler loop:

```c
void startup_application(void) {
    com_init();
    com_templates_register_all_handlers();
    com_tmpl_wifi_http_init(WIFI_SSID_DEFAULT, WIFI_PASS_DEFAULT);
    // com_tmpl_mqtt_init(...);   // Optional MQTT
    // data_logger_init();        // Optional data logger
}
```

#### Interrupt Callback System

```c
typedef void (*int_callback_t)(void);
void register_int_callback(int_callback_t cb);
void execute_int_callback(void);
```

---

### 3. Communication Hub (Com Hub)

**Files:** `COM.c`, `COM.h`

A **single-dispatcher, multi-interface request router**. Any incoming data from any of the 8 interfaces is pushed into a shared Ring Buffer. The dispatcher (`com_update_1ms()`) pulls requests from the buffer every 1 ms and routes the response back to the originating interface.

#### Supported Interfaces (`com_interface_t`)

| Enum | Index | Protocol | Medium |
|---|---|---|---|
| `COM_IF_UART` | 0 | UART / RS485 | Wired Serial |
| `COM_IF_MODBUS` | 1 | Modbus RTU / TCP | RS485 / Ethernet |
| `COM_IF_LORA` | 2 | LoRa P2P (SX127x) | RF 433/868/915 MHz |
| `COM_IF_WIFI_HTTP` | 3 | WiFi HTTP REST | 2.4 GHz Wi-Fi |
| `COM_IF_MQTT` | 4 | MQTT Pub/Sub | TCP/IP |
| `COM_IF_CAN` | 5 | CAN Bus / TWAI | Differential Pair |
| `COM_IF_BLE` | 6 | Bluetooth LE GATT | 2.4 GHz BLE |
| `COM_IF_ESPNOW` | 7 | ESP-NOW | 2.4 GHz proprietary |

#### Hub API

```c
esp_err_t com_init(void);
void com_register_tx_handler(com_interface_t iface, com_tx_handler_t handler);
bool com_push_incoming_request(com_interface_t iface, const void *data, size_t len);
void com_update_1ms(void);  // MUST be called in job_1ms()
```

If the buffer is empty, `com_update_1ms()` returns in **< 1 microsecond** (no blocking).

---

### 4. Ring Buffer Communication Layer

**Files:** `ringbuff_com.c`, `ringbuff_com.h`

Wraps FreeRTOS `esp_ringbuf` (`RINGBUF_TYPE_NOSPLIT`) into a statistics-aware API acting as the **thread-safe IPC queue** between producer drivers and the Com Hub consumer.

#### Core API

```c
esp_err_t ringbuf_com_init(size_t buffer_size);   // 0 = use default 2048 bytes
bool ringbuf_com_send(const void *data, size_t len, uint32_t wait_ms);
bool ringbuf_com_send_from_isr(const void *data, size_t len, BaseType_t *pxHigherPriorityTaskWoken);
void* ringbuf_com_receive(size_t *item_size, uint32_t wait_ms);
void ringbuf_com_free(void *item);
ringbuf_com_stats_t ringbuf_com_get_stats(void);
void ringbuf_com_print_stats(void);
```

#### Statistics Structure

```c
typedef struct {
    uint32_t total_sent;
    uint32_t total_received;
    uint32_t dropped_packets;
    size_t   free_bytes;
} ringbuf_com_stats_t;
```

#### CRC Utilities (hardware-accelerated via `esp_rom_crc`)

```c
uint16_t comm_crc16(const void *data, size_t len);
bool     comm_verify_crc16(const void *data, size_t len, uint16_t expected_crc);
uint32_t comm_crc32(const void *data, size_t len);
bool     comm_verify_crc32(const void *data, size_t len, uint32_t expected_crc);
```

---

### 5. Communication Protocol Templates

**Files:** `com_templates.c`, `com_templates.h`

8 ready-to-use protocol template stubs, each following the **init / send / on_rx** pattern.

#### Template 1: UART / RS485

```c
esp_err_t com_tmpl_uart_init(bsp_uart_pins_t pins, uint32_t baud_rate);
esp_err_t com_tmpl_uart_send(const void *data, size_t len);
void      com_tmpl_uart_on_rx_bytes(const uint8_t *bytes, size_t len);
```

#### Template 2: Modbus RTU (RS485)

Supports function codes: `FC_READ_HOLDING_REGS (0x03)`, `FC_READ_INPUT_REGS (0x04)`, `FC_WRITE_SINGLE_REG (0x06)`, `FC_WRITE_MULTI_REGS (0x10)`.

```c
esp_err_t com_tmpl_modbus_init(bsp_uart_pins_t pins, uint8_t slave_id, uint32_t baud_rate);
esp_err_t com_tmpl_modbus_send_response(const void *data, size_t len);
void      com_tmpl_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc,
                                       uint16_t reg_addr, uint16_t reg_count);
```

#### Template 3: LoRa P2P (SX1276 / SX1278)

```c
esp_err_t com_tmpl_lora_init(bsp_spi_pins_t pins, long frequency_hz);
esp_err_t com_tmpl_lora_send_packet(const void *data, size_t len);
void      com_tmpl_lora_on_rx_packet(const uint8_t *packet, size_t len);
```

#### Template 4: Wi-Fi HTTP REST Server

Initialises ESP32 in STA mode. Upon getting an IP, OTA web server starts at `http://<IP>/update`.

```c
esp_err_t com_tmpl_wifi_http_init(const char *ssid, const char *pass);
esp_err_t com_tmpl_http_send_response(const void *data, size_t len);
void      com_tmpl_http_on_endpoint_request(const char *uri, com_cmd_code_t cmd);
```

#### Template 5: MQTT Pub/Sub

```c
esp_err_t com_tmpl_mqtt_init(const char *broker_uri, const char *client_id);
esp_err_t com_tmpl_mqtt_publish_response(const void *data, size_t len);
void      com_tmpl_mqtt_on_message_received(const char *topic, const uint8_t *payload, size_t len);
```

#### Template 6: CAN Bus / TWAI

Supports 125, 250, 500, and 1000 kbps. Non-blocking `twai_receive()` polling via `can_rx_poll()`.

```c
esp_err_t com_tmpl_can_init(int tx_pin, int rx_pin, uint32_t baud_rate_kbps);
esp_err_t com_tmpl_can_send_frame(const void *data, size_t len);
void      com_tmpl_can_on_frame_received(uint32_t can_id, const uint8_t *data, uint8_t dlc);
void      can_rx_poll(void);
```

#### Template 7: Bluetooth Low Energy (GATT)

```c
esp_err_t com_tmpl_ble_init(const char *device_name);
esp_err_t com_tmpl_ble_send_notify(const void *data, size_t len);
void      com_tmpl_ble_on_characteristic_write(const uint8_t *data, size_t len);
```

#### Template 8: ESP-NOW (Two-Way P2P + Broadcast)

Auto-registers new peer MACs on first receive, enabling bidirectional auto-reply.

```c
esp_err_t com_tmpl_espnow_init(const uint8_t *peer_mac, uint8_t channel);
esp_err_t com_tmpl_espnow_add_peer(const uint8_t *peer_mac, uint8_t channel, bool encrypt);
esp_err_t com_tmpl_espnow_send(const void *data, size_t len);  // Max 250 bytes
void      com_tmpl_espnow_on_recv(const uint8_t *src_mac, const uint8_t *data, int len);
```

#### Registering All TX Handlers

```c
void com_templates_register_all_handlers(void);
```

---

### 6. OTA Firmware Updater (Web-based)

**Files:** `ota_update.c`, `ota_update.h`

The OTA HTTP web server starts **automatically** when Wi-Fi connects (triggered by `IP_EVENT_STA_GOT_IP`).

#### HTTP Endpoints

| Method | URI | Description |
|---|---|---|
| `GET` | `/` | Redirects to `/update` |
| `GET` | `/update` | Serves the OTA web UI page |
| `POST` | `/update` | Streams binary firmware chunks to OTA flash partition |

#### Upload & Flash Flow

1. Open `http://<IP>/update` in a browser
2. Select a `hello_world.bin` file using the file picker
3. Click "Upload & Flash Firmware" — firmware is streamed in 1 KB chunks
4. First chunk validation: first byte must be `0xE9` (ESP32 binary magic byte)
5. On success: `esp_ota_end()` -> `esp_ota_set_boot_partition()` -> delayed reboot (1.5 s)
6. Browser auto-refreshes after 6 seconds

#### OTA Partition Layout

```
ota_0   @ 0x010000  (1536 KB)  <- currently running firmware
ota_1   @ 0x190000  (1536 KB)  <- next firmware (OTA target)
```

#### Web UI Features

- Modern dark UI with glassmorphism card
- Live upload progress bar with percentage
- Filename and file size display
- Error feedback on invalid file or network failure
- Auto-reload after successful flash

---

### 7. Data Logger System

**Files:** `data_logger.c`, `data_logger.h`, `log_system.c`, `log_system.h`, `app_log.c`

A high-performance, non-blocking sensor data logging engine for continuous recording to SPI Flash or SD Card.

#### Architecture: Ping-Pong Double Buffer

```
Core 1 (Producer)              Core 0 (Consumer)
-----------------              -----------------
  Write_Datalog()   -->   [Active Buffer A]  --> (swap when full)
                          [Flush  Buffer B]  --> log_writer_task()
                                                        |
                                                        v
                                                   SPI Flash (4 KB sector write)
```

- **Active buffer** (4 KB, RAM): receives new records continuously
- **Flush buffer** (4 KB, RAM): holds the filled sector waiting to be written
- Buffer swap is atomic; Task Notification sent to `log_writer_task` on Core 0
- Core 1 is **never blocked** by Flash write latency

#### Chunk Format (Per 4 KB Sector)

```
[chunk_header_t]   <- magic, sequence, record count, record size, timestamp
[record 0]         <- device_datalog_t (packed struct, float fields)
[record 1]
...
[record N]
[padding: 0xFF]    <- Fill to sector boundary
[chunk_footer_t]   <- CRC-32 over entire sector minus footer, end magic
```

#### Boot Recovery

`log_system_boot_recovery()` on every boot:
- Scans all sectors for valid magic_start and magic_end markers
- Verifies CRC-32 integrity of each sector
- Finds the sector with the highest valid sequence number
- Positions write head after the last valid sector
- Corrupted sectors (from power loss) are silently skipped with a warning log

#### Data Schema (7 Parameters)

| Index | Name | Unit | Source Field |
|---|---|---|---|
| 0 | `bias_dc_acc_x` | m/s2 | `log_param1` |
| 1 | `rms_noise_acc_x` | m/s2 | `log_param2` |
| 2 | `vrw_acc_x` | m/s | `log_param3` |
| 3 | `bias_dc_gyro_x` | rad/s | `log_param4` |
| 4 | `rms_noise_gyro_x` | rad/s | `log_param5` |
| 5 | `timestamp` | s | `timestamp` (from `esp_timer_get_time()`) |
| 6 | `countTIMER` | count | `countTIMER` (runtime loop counter) |

#### Public Data Logger API

```c
esp_err_t data_logger_init(void);
esp_err_t data_logger_write_record(const device_datalog_t *data);
void Write_Datalog(void);           // Convenience wrapper with auto timestamp
esp_err_t data_logger_sync(void);   // Force-flush remaining buffer to storage
void data_logger_set_reading_state(bool is_reading);
bool data_logger_is_reading(void);
esp_err_t data_logger_export_csv_header(char *out_buf, size_t max_len);
const datalog_param_desc_t* data_logger_get_schema(size_t *num_entries);
```

---

### 8. Input Drivers (Sensors & Display)

#### IMU: MPU6050

**Files:** `IMU_MPU.c`, `IMU_MPU.h`

Driver for the **InvenSense MPU6050** 6-axis IMU over I2C.

```c
esp_err_t imu_mpu_init(const imu_mpu_config_t *config);  // NULL uses defaults
esp_err_t imu_mpu_init_pins(bsp_i2c_pins_t pins);
void imu_mpu_update(void);
const imu_data_t* imu_mpu_get_data(void);
```

Data structure:

```c
typedef struct {
    int16_t raw_accel_z;  // Raw ADC value from MPU register 0x3F
    float   accel_z;      // Converted to G-force (/ 16384.0 for +/-2g range)
} imu_data_t;
```

- Clock speed: 400 kHz (I2C Fast Mode)
- Default I2C address: `0x68`
- Wakeup: writes `0x00` to `PWR_MGMT_1` register (`0x6B`)

#### Display: LCD (HD44780 via PCF8574 I2C Expander)

**Files:** `LCD.c`, `LCD.h`

Driver for HD44780-compatible LCD (2x16) via PCF8574 I2C port expander, 4-bit mode.

```c
esp_err_t lcd_init(const lcd_config_t *config);   // NULL uses defaults
esp_err_t lcd_init_pins(bsp_i2c_pins_t pins);
void lcd_clear(void);
void lcd_put_cur(int row, int col);
void lcd_send_string(const char *str);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
```

---

## Pin Map (BSP)

All hardware pin assignments are centralised in `main.h`:

```c
typedef struct { int sda; int scl; }                   bsp_i2c_pins_t;
typedef struct { int mosi; int miso; int sck; int cs; } bsp_spi_pins_t;
typedef struct { int tx; int rx; }                     bsp_uart_pins_t;
typedef struct { int tx; int rx; }                     bsp_can_pins_t;
```

| Interface | Macro | GPIO Pins |
|---|---|---|
| I2C — LCD | `I2C_PINS_LCD` | SDA: 26, SCL: 25 |
| I2C — IMU | `I2C_PINS_IMU` | SDA: 14, SCL: 12 |
| UART — GPS | `UART_PINS_GPS` | TX: 17, RX: 16 |
| SPI — LoRa | `SPI_PINS_LORA` | MOSI: 23, MISO: 19, SCK: 18, CS: 5 |
| TWAI — CAN | `CAN_PINS_DEFAULT` | TX: 21, RX: 22 |

---

## Flash Partition Table

Custom partition layout in `partitions.csv` for a **4 MB Flash** chip:

| Name | Type | SubType | Offset | Size | Purpose |
|---|---|---|---|---|---|
| `nvs` | data | nvs | `0x9000` | 16 KB | NVS (Wi-Fi credentials, config) |
| `otadata` | data | ota | `0xD000` | 8 KB | OTA boot slot selector |
| `phy_init` | data | phy | `0xF000` | 4 KB | RF PHY calibration data |
| `ota_0` | app | ota_0 | `0x10000` | 1536 KB | Primary firmware slot |
| `ota_1` | app | ota_1 | `0x190000` | 1536 KB | Secondary OTA firmware slot |
| `log_data` | data | `0x99` | `0x310000` | 896 KB | Sensor data log storage |

> **Note:** The `log_data` partition uses custom subtype `0x99` and is directly accessed by the data logger engine at raw Flash addresses.

---

## Getting Started

### Prerequisites

Install the [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/) toolchain and configure `IDF_PATH`.

### Build the Project

```bash
cd hello_world
idf.py menuconfig   # Optional: adjust settings
idf.py build
```

### Flash to Device

```bash
idf.py -p COM<N> flash
```

Replace `COM<N>` with your serial port (`COM3` on Windows, `/dev/ttyUSB0` on Linux).

### Monitor Serial Output

```bash
idf.py -p COM<N> monitor
```

Expected output on boot:

```
I (xxx) RINGBUF_COM: RingBuffer Komunikasi aktif (2048 bytes, No-Split Mode).
I (xxx) COM_HUB: Central Communication Hub siap (Ultra-Fast 1ms Dispatcher).
I (xxx) WIFI_AUTO: WiFi Driver siap, menghubungkan ke AP...
I (xxx) WIFI_AUTO: >>> SUKSES TERHUBUNG KE WIFI! <<<
I (xxx) WIFI_AUTO: >>> IP Address : 192.168.x.x
I (xxx) OTA_UPDATE: HTTP Server OTA Update aktif. Akses melalui: http://192.168.x.x/update
I (xxx) RINGBUF_COM: [STATISTIK] Sent: 0 | Recv: 0 | Dropped: 0 | Free: 2048 bytes
```

### Flash + Monitor

```bash
idf.py -p COM<N> flash monitor
```

---

## Configuration Reference

### Wi-Fi & MQTT Defaults

In `main.h`:

```c
#define WIFI_SSID_DEFAULT        "YourNetworkSSID"
#define WIFI_PASS_DEFAULT        "YourPassword"
#define MQTT_BROKER_URI_DEFAULT  "mqtt://broker.emqx.io:1883"
#define MQTT_CLIENT_ID_DEFAULT   "ESP32_Node_01"
#define MQTT_TOPIC_REQ_DEFAULT   "esp32/node1/request"
#define MQTT_TOPIC_RESP_DEFAULT  "esp32/node1/response"
```

### SDK Configuration

`sdkconfig.defaults` pre-configures:

```
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
CONFIG_ESPTOOLPY_FLASHSIZE="4MB"
CONFIG_HTTPD_MAX_REQ_HDR_LEN=1024
```

### Scheduler Configuration

```c
scheduler_config_t cfg = {
    .enable_prio_5_fast = true,   // Mandatory for Com Hub (job_1ms)
    .enable_prio_4_high = true,
    .enable_prio_3_mid  = true,
    .enable_prio_2_low  = false,  // Disable 200-500ms tasks
    .enable_prio_1_bg   = true
};
init_scheduler(&cfg);
```

---

## Communication Protocols

### Packet Format

Inbound request (`com_inbound_req_t`):

```c
typedef struct {
    uint8_t  preamble;       // Always 0xAA
    uint8_t  iface_source;   // Originating com_interface_t
    uint8_t  cmd_code;       // com_cmd_code_t (what is being requested)
    uint8_t  payload_len;    // Length of optional extra payload (0-32)
    uint8_t  payload[32];    // Optional request parameters
    uint16_t crc16;          // CRC-16 over struct excluding crc16 field
} __attribute__((packed)) com_inbound_req_t;
```

Outbound response (`comm_packet_t`):

```c
typedef struct {
    uint8_t  preamble;    // 0xAA
    uint8_t  msg_type;    // cmd_code | 0x80 (response flag)
    uint8_t  payload_len;
    uint8_t  payload[64]; // Response data (up to 64 bytes)
    uint16_t crc16;
} __attribute__((packed)) comm_packet_t;
```

### Command Codes

| Code | Enum | Description | Response Payload |
|---|---|---|---|
| `0x01` | `CMD_REQ_PING` | Connectivity heartbeat ping | "PONG" (5 bytes) |
| `0x02` | `CMD_REQ_ALL_SENSORS` | Request all sensor data | IMU accel_z (4 bytes float) |
| `0x03` | `CMD_REQ_IMU` | Request IMU data only | accel_z (4 bytes float) |
| `0x04` | `CMD_REQ_GPS` | Request GPS NMEA data | *(not yet implemented)* |
| `0x05` | `CMD_REQ_SYS_STATUS` | System health / battery status | *(not yet implemented)* |
| `0x0F` | `CMD_REQ_CUSTOM` | Custom user-defined payload | *(user-defined)* |

### Adding a New Protocol

1. Define `_init()`, `_send()`, and `_on_rx_*()` following the template pattern
2. Add new enum to `com_interface_t` in `COM.h` (before `COM_IF_MAX`)
3. Register TX handler in `com_templates_register_all_handlers()`:
   ```c
   com_register_tx_handler(COM_IF_MY_PROTO, my_proto_send);
   ```
4. Push incoming data to the hub from your RX callback:
   ```c
   com_push_incoming_request(COM_IF_MY_PROTO, data, len);
   ```
5. Add a case to `com_dispatch_response()` in `COM.c` for new command codes

---

## OTA Update Workflow

1. Build: `idf.py build`
2. Locate binary: `build/hello_world.bin`
3. Boot ESP32 and connect to Wi-Fi. Check serial monitor for IP address
4. Open `http://<ESP32_IP>/update` in browser
5. Select `hello_world.bin` and click "Upload & Flash Firmware"
6. Watch progress bar; wait for success message and 5-second reboot countdown
7. ESP32 boots from newly flashed partition

> **Rollback:** Call `esp_ota_mark_app_valid_cancel_rollback()` in your validated startup code to confirm new firmware is good. Without this, the bootloader can roll back on next boot failure.

---

## Data Logger Guide

### Enabling & Configuring

1. Uncomment `data_logger_init()` in `startup_application()` inside `task.c`

2. Assign your sensor data in `Write_Datalog()` in `data_logger.c`:

```c
void Write_Datalog(void) {
    if (s_is_reading_log) return;
    s_runtime_counter++;

    const imu_data_t *imu = imu_mpu_get_data();
    Device_Datalog_buffer.log_param1 = imu->accel_z;
    Device_Datalog_buffer.countTIMER = s_runtime_counter;
    Device_Datalog_buffer.timestamp  = (float)esp_timer_get_time() / 1000000.0f;

    data_logger_write_record(&Device_Datalog_buffer);
}
```

3. Call `Write_Datalog()` from any periodic slot:

```c
void job_100ms(void) {
    imu_mpu_update();
    Write_Datalog();
}
```

4. Export CSV header for post-processing:

```c
char header[256];
data_logger_export_csv_header(header, sizeof(header));
// Output: "bias_dc_acc_x [m/s2],rms_noise_acc_x [m/s2],...,countTIMER [count]"
```

### Adding New Log Parameters

1. Increase `NUMBER_OF_LOGDATA` in `data_logger.h`
2. Add a new `float` field to `device_datalog_t`
3. Add a new entry to the `Logger_data[]` schema array in `data_logger.c`

### Reading Back Data

Use `data_logger_set_reading_state(true)` to pause writes, then read sectors from the `log_data` partition using `esp_partition_read()`.

---

## Testing

The project includes pytest-embedded integration tests for CI/CD.

**File:** `pytest_hello_world.py`

```python
@pytest.mark.generic
def test_hello_world(dut: IdfDut):
    dut.expect('Hello world!')

@pytest.mark.host_test
def test_hello_world_linux(dut: IdfDut):
    dut.expect('Hello world!')

@pytest.mark.qemu
def test_hello_world_host(app: QemuApp, dut: QemuDut):
    sha256_reported = dut.expect(r'ELF file SHA256:\s+([a-f0-9]+)').group(1)
    verify_elf_sha256_embedding(app, sha256_reported)
```

Run tests:

```bash
pip install pytest pytest-embedded pytest-embedded-idf pytest-embedded-qemu
pytest pytest_hello_world.py -v
```

---

## Extending the Framework

### Adding a New Sensor

1. Create `main/input/MySensor.c` and `main/include/MySensor.h`
2. Follow the IMU pattern: private handle, `init()`, `update()`, `get_data()`
3. Register in `main/CMakeLists.txt`:
   ```cmake
   idf_component_register(SRCS "..." "input/MySensor.c" ...)
   ```
4. Call `my_sensor_update()` in the appropriate `job_Xms()` slot

### Adding a New Command Code

1. Add enum value to `com_cmd_code_t` in `COM.h`
2. Add case to `com_dispatch_response()` in `COM.c`:
   ```c
   case CMD_REQ_MY_COMMAND: {
       resp.payload_len = sizeof(my_data);
       memcpy(resp.payload, &my_data, sizeof(my_data));
       break;
   }
   ```

### Changing Ring Buffer Size

```c
ringbuf_com_init(4096);  // Use 4 KB instead of the default 2 KB
```

---

## File Reference

| File | Role |
|---|---|
| `main.h` | BSP pin definitions, Wi-Fi/MQTT defaults, I2C/SPI/UART/CAN pin structs |
| `main.c` | Entry point — calls `init_scheduler(NULL)` |
| `Scheduler.h` | `scheduler_config_t` struct, `SCHEDULER_CONFIG_ALL_ENABLE()` macro |
| `Scheduler.c` | Creates 5 FreeRTOS tasks, sets up WDT, calls `startup_application()` |
| `task.h` | All `job_Xms()` prototypes + interrupt callback API |
| `task.c` | **User-editable**: implement your periodic application logic here |
| `COM.h` | Com Hub enums, packet structures, API declarations |
| `COM.c` | Dispatcher logic, `com_update_1ms()`, `com_dispatch_response()` |
| `com_templates.h` | 8-protocol template API declarations |
| `com_templates.c` | Full implementations of all 8 communication protocol templates |
| `ringbuff_com.h` | Ring buffer API + CRC utility declarations |
| `ringbuff_com.c` | FreeRTOS ring buffer wrapper + CRC-16/CRC-32 hardware functions |
| `ota_update.h` | OTA API declarations |
| `ota_update.c` | HTTP web server + OTA flash stream handler + embedded web UI |
| `data_logger.h` | Data structures, schema API, public logger API |
| `data_logger.c` | Parameter schema dictionary, `Write_Datalog()`, CSV header exporter |
| `log_system.h` | Log engine API |
| `log_system.c` | Ping-Pong engine, boot recovery scanner, background writer task |
| `app_log.h` | Storage backend abstraction interface |
| `app_log.c` | SPI Flash / SD Card backend implementation |
| `IMU_MPU.h` | MPU6050 API + `imu_data_t` struct |
| `IMU_MPU.c` | MPU6050 I2C driver (init, update, get_data) |
| `LCD.h` | LCD API + `lcd_config_t` struct |
| `LCD.c` | HD44780 LCD driver via PCF8574 I2C expander |
| `partitions.csv` | 4 MB custom partition table (NVS, OTA x2, log_data) |
| `sdkconfig.defaults` | Pre-configured SDK build defaults |
| `CMakeLists.txt` | Root project build file |
| `main/CMakeLists.txt` | Component sources registration + ESP-IDF dependencies |
| `pytest_hello_world.py` | pytest-embedded CI/CD integration tests |

---

## Author

**Rafdi** — Created: September 2026

---

*This framework is designed as a reusable embedded systems foundation. Fork it, strip what you don't need, and fill in the `job_Xms()` slots to build your application.*
