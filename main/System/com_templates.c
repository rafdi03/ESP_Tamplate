/*
 * com_templates.c
 *
 *  Created on: 7 Sept 2026
 *      Author: Rafdi
 *
 *  Deskripsi: Implementasi template boilerplate modular untuk 7 protokol komunikasi:
 *             1. UART / RS485
 *             2. Modbus RTU / TCP
 *             3. LoRa P2P (SX127x)
 *             4. WiFi HTTP REST Server
 *             5. MQTT Broker Pub/Sub
 *             6. CAN Bus / TWAI Controller
 *             7. BLE GATT Services
 */

#include "com_templates.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/twai.h"
#pragma GCC diagnostic pop
#include "esp_now.h"
#include "esp_mac.h"
#include "ota_update.h"

static const char *TAG = "COM_TEMPLATES";

/* =========================================================================
 * 1. TEMPLATE UART / RS485
 * ========================================================================= */

/**
 * @brief  Inisialisasi peripheral UART/RS485 dengan konfigurasi baud rate dan pin BSP.
 * @param  pins Struktur konfigurasi pin TX dan RX dari main.h (bsp_uart_pins_t).
 * @param  baud_rate Kecepatan transmisi baud rate (contoh: 9600, 115200 bps).
 * @retval ESP_OK jika inisialisasi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_uart_init(bsp_uart_pins_t pins, uint32_t baud_rate) {
    ESP_LOGI(TAG, "[UART Template] Init TX: %d, RX: %d @ %lu bps", pins.tx, pins.rx, (unsigned long)baud_rate);
    return ESP_OK;
}

/**
 * @brief  Mengirimkan byte data balasan melalui antarmuka UART/RS485 (TX Handler).
 * @param  data Pointer ke buffer data yang akan dikirimkan.
 * @param  len Jumlah byte data yang akan ditransmisikan.
 * @retval ESP_OK jika data berhasil dikirim, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_uart_send(const void *data, size_t len) {
    ESP_LOGI(TAG, "[UART TX] Mengirim %u bytes balasan ke UART/RS485", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback event penampung data byte masuk dari UART ISR/Driver ke Com Hub.
 * @param  bytes Pointer ke data byte yang diterima dari UART hardware.
 * @param  len Panjang data byte yang diterima.
 * @retval None
 */
void com_tmpl_uart_on_rx_bytes(const uint8_t *bytes, size_t len) {
    com_push_incoming_request(COM_IF_UART, bytes, len);
}

/* =========================================================================
 * 2. TEMPLATE MODBUS (RTU / RS485)
 * ========================================================================= */

static uint8_t s_modbus_slave_id = 1;

/**
 * @brief  Inisialisasi stack Modbus RTU Slave pada transceiver RS485.
 * @param  pins Struktur konfigurasi pin hardware UART (bsp_uart_pins_t).
 * @param  slave_id Nomor identitas unik Modbus Slave (1 - 247).
 * @param  baud_rate Kecepatan transmisi serial Modbus (contoh: 9600 bps).
 * @retval ESP_OK jika inisialisasi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_modbus_init(bsp_uart_pins_t pins, uint8_t slave_id, uint32_t baud_rate) {
    s_modbus_slave_id = slave_id;
    ESP_LOGI(TAG, "[Modbus Template] Init Slave ID: %u, TX: %d, RX: %d @ %lu bps", 
             slave_id, pins.tx, pins.rx, (unsigned long)baud_rate);
    return ESP_OK;
}

/**
 * @brief  Mengirimkan paket respons frame Modbus ke Master melalui RS485.
 * @param  data Pointer ke buffer frame respons Modbus.
 * @param  len Ukuran frame respons dalam bytes.
 * @retval ESP_OK jika transmisi berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_modbus_send_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[Modbus TX] Mengirim %u bytes frame balasan Modbus", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback pemrosesan frame query Modbus dari Master untuk diteruskan ke Com Hub.
 * @param  slave_id ID Slave target pada frame Modbus yang diterima.
 * @param  fc Function code Modbus yang diminta (modbus_function_code_t).
 * @param  reg_addr Alamat register awal (Starting Register Address).
 * @param  reg_count Jumlah register yang diminta (Number of Registers).
 * @retval None
 */
void com_tmpl_modbus_on_rx_frame(uint8_t slave_id, modbus_function_code_t fc, uint16_t reg_addr, uint16_t reg_count) {
    if (slave_id != s_modbus_slave_id && slave_id != 0) {
        return; 
    }

    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_MODBUS,
        .cmd_code = (fc == MODBUS_FC_READ_INPUT_REGS) ? CMD_REQ_IMU : CMD_REQ_ALL_SENSORS,
        .payload_len = 4
    };
    memcpy(&req.payload[0], &reg_addr, 2);
    memcpy(&req.payload[2], &reg_count, 2);
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));

    com_push_incoming_request(COM_IF_MODBUS, &req, sizeof(req));
}

/* =========================================================================
 * 3. TEMPLATE LORA (P2P SX1276 / SX1278)
 * ========================================================================= */

/**
 * @brief  Inisialisasi modul radio LoRa P2P via antarmuka SPI.
 * @param  pins Struktur konfigurasi pin SPI (MOSI, MISO, SCK, CS) dari main.h.
 * @param  frequency_hz Frekuensi operasi RF dalam Hertz (contoh: 915000000 Hz / 433000000 Hz).
 * @retval ESP_OK jika inisialisasi chip radio berhasil, atau kode error esp_err_t jika gagal.
 */
esp_err_t com_tmpl_lora_init(bsp_spi_pins_t pins, long frequency_hz) {
    ESP_LOGI(TAG, "[LoRa Template] Init Frekuensi: %ld Hz, MOSI:%d MISO:%d SCK:%d CS:%d", 
             frequency_hz, pins.mosi, pins.miso, pins.sck, pins.cs);
    return ESP_OK;
}

/**
 * @brief  Mengirimkan paket data biner melalui gelombang radio LoRa RF (TX Handler).
 * @param  data Pointer ke data paket payload.
 * @param  len Ukuran paket data dalam bytes.
 * @retval ESP_OK jika paket berhasil dikirim ke modul LoRa, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_lora_send_packet(const void *data, size_t len) {
    ESP_LOGI(TAG, "[LoRa TX] Mengirim %u bytes paket radio RF", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback event penampung paket radio masuk dari LoRa Receiver ke Com Hub.
 * @param  packet Pointer ke buffer payload yang diterima dari sinyal radio.
 * @param  len Ukuran paket data yang diterima dalam bytes.
 * @retval None
 */
void com_tmpl_lora_on_rx_packet(const uint8_t *packet, size_t len) {
    com_push_incoming_request(COM_IF_LORA, packet, len);
}

/* =========================================================================
 * 4. TEMPLATE WIFI & HTTP REST SERVER
 * ========================================================================= */

static const char *TAG_WIFI = "WIFI_AUTO";

/**
 * @brief  Event handler internal untuk memproses siklus hidup WiFi dan auto-reconnect.
 * @param  arg Pointer argumen opsional yang dilewatkan saat pendaftaran.
 * @param  event_base Tipe base event (WIFI_EVENT / IP_EVENT).
 * @param  event_id ID event spesifik (STA_START, STA_DISCONNECTED, GOT_IP).
 * @param  event_data Pointer data payload event.
 * @retval None
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG_WIFI, "WiFi Driver siap, menghubungkan ke AP...");
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG_WIFI, "Koneksi WiFi terputus (Reason: %d)! Mencoba reconnect...", disconn->reason);
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "==================================================");
        ESP_LOGI(TAG_WIFI, ">>> SUKSES TERHUBUNG KE WIFI! <<<");
        ESP_LOGI(TAG_WIFI, ">>> IP Address : " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG_WIFI, ">>> Netmask    : " IPSTR, IP2STR(&event->ip_info.netmask));
        ESP_LOGI(TAG_WIFI, ">>> Gateway    : " IPSTR, IP2STR(&event->ip_info.gw));
        ESP_LOGI(TAG_WIFI, "==================================================");

        // Otomatis aktifkan Web Server OTA Firmware Update
        ota_update_init();
        ESP_LOGI(TAG_WIFI, ">>> OTA Web UI Aktif: http://" IPSTR "/update <<<", IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG_WIFI, "==================================================");
    }
}

/**
 * @brief  Inisialisasi stack WiFi Station (STA Mode) dan Web Server REST API.
 * @param  ssid Nama Access Point WiFi (SSID).
 * @param  pass Kata sandi jaringan WiFi.
 * @retval ESP_OK jika WiFi stack dan HTTP server berhasil dijalankan, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_wifi_http_init(const char *ssid, const char *pass) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    return ESP_OK;
}

/**
 * @brief  Mengirimkan respons HTTP JSON/Binary dari REST Server ke client web.
 * @param  data Pointer ke payload respons HTTP.
 * @param  len Panjang payload dalam bytes.
 * @retval ESP_OK jika pengiriman respons berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_http_send_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[HTTP Server TX] Balas JSON Response (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback penangan permintaan HTTP GET/POST pada endpoint REST API tertentu.
 * @param  uri String jalur endpoint URI yang diakses client (contoh: "/api/v1/sensors").
 * @param  cmd Kode perintah Com Hub yang dipetakan (com_cmd_code_t).
 * @retval None
 */
void com_tmpl_http_on_endpoint_request(const char *uri, com_cmd_code_t cmd) {
    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_WIFI_HTTP,
        .cmd_code = (uint8_t)cmd,
        .payload_len = 0
    };
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));
    com_push_incoming_request(COM_IF_WIFI_HTTP, &req, sizeof(req));
}

/* =========================================================================
 * 5. TEMPLATE MQTT BROKER PUB / SUB
 * ========================================================================= */

/**
 * @brief  Inisialisasi client MQTT dan menghubungkan ke broker IoT secara asinkron.
 * @param  broker_uri Alamat URI broker MQTT (contoh: "mqtt://broker.hivemq.com:1883").
 * @param  client_id Nama identitas unik client perangkat pada broker MQTT.
 * @retval ESP_OK jika client MQTT berhasil dibuat dan dimulai, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_mqtt_init(const char *broker_uri, const char *client_id) {
    ESP_LOGI(TAG, "[MQTT Template] Connect ke broker: %s (Client: %s)", broker_uri, client_id);
    return ESP_OK;
}

/**
 * @brief  Mempublikasikan (Publish) data balasan ke topik response MQTT broker.
 * @param  data Pointer ke payload data yang akan dipublikasikan.
 * @param  len Ukuran data payload dalam bytes.
 * @retval ESP_OK jika publish berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_mqtt_publish_response(const void *data, size_t len) {
    ESP_LOGI(TAG, "[MQTT TX] Publish balasan ke topik 'esp32/response' (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback penampung pesan request masuk dari subscription topik MQTT.
 * @param  topic Nama topik MQTT tempat pesan diterima.
 * @param  payload Pointer ke isi data pesan payload yang diterima.
 * @param  len Panjang data payload dalam bytes.
 * @retval None
 */
void com_tmpl_mqtt_on_message_received(const char *topic, const uint8_t *payload, size_t len) {
    ESP_LOGI(TAG, "[MQTT RX] Pesan masuk di topik: %s", topic);
    com_push_incoming_request(COM_IF_MQTT, payload, len);
}

/* =========================================================================
 * 6. TEMPLATE CAN BUS / TWAI (Automotive & Industrial)
 * ========================================================================= */

/**
 * @brief  Inisialisasi kontroler CAN Bus / TWAI hardware ESP32.
 * @param  tx_pin Nomor pin GPIO untuk jalur Transmit (CAN TX).
 * @param  rx_pin Nomor pin GPIO untuk jalur Receive (CAN RX).
 * @param  baud_rate_kbps Kecepatan bus CAN dalam kbps (contoh: 125, 250, 500, 1000 kbps).
 * @retval ESP_OK jika inisialisasi driver TWAI berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_can_init(int tx_pin, int rx_pin, uint32_t baud_rate_kbps) {
	twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
	        (gpio_num_t)tx_pin, 
	        (gpio_num_t)rx_pin, 
	        TWAI_MODE_NORMAL
	    );
	    twai_timing_config_t t_config;
	    switch (baud_rate_kbps) {
	        case 125:  t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_125KBITS(); break;
	        case 250:  t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_250KBITS(); break;
	        case 1000: t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_1MBITS(); break;
	        case 500:
	        default:   t_config = (twai_timing_config_t)TWAI_TIMING_CONFIG_500KBITS(); break;
	    }
	    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
	    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
	    if (ret != ESP_OK) {
	        ESP_LOGE(TAG, "Gagal menginstall CAN/TWAI driver!");
	        return ret;
	    }
	    ret = twai_start();
	    if (ret == ESP_OK) {
	        ESP_LOGI(TAG, "CAN Bus (TWAI) Aktif @ %lu kbps pada TX:%d, RX:%d", (unsigned long)baud_rate_kbps, tx_pin, rx_pin);
	    }
	    return ret;
	
}

/**
 * @brief  Mengirimkan frame paket CAN Bus ke jaringan otomotif/industri.
 * @param  data Pointer ke isi payload frame data CAN (maksimal 8 bytes).
 * @param  len Panjang payload frame data dalam bytes.
 * @retval ESP_OK jika transmisi frame berhasil di-enqueue, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_can_send_frame(const void *data, size_t len) {
	if (data == NULL || len == 0) return ESP_ERR_INVALID_ARG;
	    twai_message_t tx_msg = {
	        .identifier = 0x123,           
	        .extd = 0,                 
	        .data_length_code = (len > 8) ? 8 : len,
	    };
	    memcpy(tx_msg.data, data, tx_msg.data_length_code);
	    return twai_transmit(&tx_msg, pdMS_TO_TICKS(10));
}

/**
 * @brief  Callback penerimaan frame CAN Bus dari kontroler TWAI ke Com Hub.
 * @param  can_id Identifier unik frame CAN (Standard 11-bit atau Extended 29-bit).
 * @param  data Pointer ke isi array payload frame data yang diterima.
 * @param  dlc Data Length Code (jumlah byte payload yang diterima, 0 - 8 bytes).
 * @retval None
 */
void com_tmpl_can_on_frame_received(uint32_t can_id, const uint8_t *data, uint8_t dlc) {
    com_inbound_req_t req = {
        .preamble = COMM_PACKET_PREAMBLE,
        .iface_source = (uint8_t)COM_IF_CAN,
        .cmd_code = (uint8_t)(can_id & 0xFF),
        .payload_len = (dlc > 32) ? 32 : dlc
    };
    if (data && dlc > 0) {
        memcpy(req.payload, data, req.payload_len);
    }
    req.crc16 = comm_crc16(&req, offsetof(com_inbound_req_t, crc16));

    com_push_incoming_request(COM_IF_CAN, &req, sizeof(req));
}

/**
 * @brief  Melakukan polling non-blocking frame masuk dari antrean hardware CAN Bus / TWAI.
 * @param  None
 * @retval None
 */
void can_rx_poll(void) {
    twai_message_t rx_msg;
    while (twai_receive(&rx_msg, 0) == ESP_OK) {
        com_tmpl_can_on_frame_received(rx_msg.identifier, rx_msg.data, rx_msg.data_length_code);
    }
}

/* =========================================================================
 * 7. TEMPLATE BLE (Bluetooth Low Energy GATT)
 * ========================================================================= */

/**
 * @brief  Inisialisasi stack Bluetooth Low Energy (BLE) sebagai GATT Server.
 * @param  device_name Nama perangkat BLE yang dipancarkan saat advertising.
 * @retval ESP_OK jika inisialisasi stack BLE dan service berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_ble_init(const char *device_name) {
    ESP_LOGI(TAG, "[BLE Template] Inisialisasi BLE GATT Service (%s)", device_name);
    return ESP_OK;
}

/**
 * @brief  Mengirimkan notifikasi data (GATT Characteristic Notification) ke client BLE yang terhubung.
 * @param  data Pointer ke buffer data yang akan dinotifikasi.
 * @param  len Panjang data yang akan dikirim dalam bytes.
 * @retval ESP_OK jika notifikasi berhasil dikirim, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_ble_send_notify(const void *data, size_t len) {
    ESP_LOGI(TAG, "[BLE TX] Kirim GATT Characteristic Notify (%u bytes)", (unsigned int)len);
    return ESP_OK;
}

/**
 * @brief  Callback event penampung penulisan data (Write Characteristic) dari client smartphone ke Com Hub.
 * @param  data Pointer ke data byte yang ditulis oleh client BLE.
 * @param  len Panjang data byte yang ditulis.
 * @retval None
 */
void com_tmpl_ble_on_characteristic_write(const uint8_t *data, size_t len) {
    com_push_incoming_request(COM_IF_BLE, data, len);
}

/* =========================================================================
 * 8. TEMPLATE ESP-NOW (Two-Way Peer-to-Peer & Broadcast 2.4GHz)
 * ========================================================================= */

static const uint8_t s_espnow_broadcast_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t s_espnow_target_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static bool s_espnow_initialized = false;

/**
 * @brief  Callback status pengiriman paket ESP-NOW (TX Callback).
 * @param  tx_info Informasi pengiriman paket (MAC tujuan, dsb).
 * @param  status Status pengiriman (ESP_NOW_SEND_SUCCESS / ESP_NOW_SEND_FAIL).
 * @retval None
 */
static void espnow_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        if (tx_info && tx_info->des_addr) {
            ESP_LOGD(TAG, "[ESP-NOW TX] Paket berhasil terkirim ke " MACSTR, MAC2STR(tx_info->des_addr));
        } else {
            ESP_LOGD(TAG, "[ESP-NOW TX] Paket berhasil terkirim.");
        }
    } else {
        if (tx_info && tx_info->des_addr) {
            ESP_LOGW(TAG, "[ESP-NOW TX] Gagal mengirim paket ke " MACSTR, MAC2STR(tx_info->des_addr));
        } else {
            ESP_LOGW(TAG, "[ESP-NOW TX] Gagal mengirim paket.");
        }
    }
}

/**
 * @brief  Callback penerimaan data paket ESP-NOW dari driver hardware (RX Callback).
 * @param  recv_info Informasi pengirim paket (MAC, RSSI, dsb).
 * @param  data Pointer data payload yang diterima.
 * @param  len Panjang payload dalam bytes.
 * @retval None
 */
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (recv_info == NULL || data == NULL || len <= 0) {
        return;
    }

    // Simpan MAC pengirim terakhir sebagai default target reply (Two-Way auto-reply)
    memcpy(s_espnow_target_mac, recv_info->src_addr, ESP_NOW_ETH_ALEN);

    com_tmpl_espnow_on_recv(recv_info->src_addr, data, len);
}

/**
 * @brief  Inisialisasi stack ESP-NOW 2-Way communication (Master/Slave).
 * @param  peer_mac Alamat 6-byte MAC target peer (atau NULL untuk broadcast FF:FF:FF:FF:FF:FF).
 * @param  channel Channel Wi-Fi operasi (1 - 13, default 1).
 * @retval ESP_OK jika inisialisasi ESP-NOW berhasil, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_init(const uint8_t *peer_mac, uint8_t channel) {
    if (s_espnow_initialized) {
        ESP_LOGW(TAG, "[ESP-NOW] Sudah diinisialisasi sebelumnya.");
        return ESP_OK;
    }

    // Inisialisasi NVS jika belum
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Inisialisasi Netif & Event loop jika belum aktif
    esp_netif_init();
    esp_event_loop_create_default();

    // Inisialisasi Wi-Fi STA mode jika belum aktif
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // Inisialisasi protokol ESP-NOW
    ret = esp_now_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ESP-NOW] Gagal inisialisasi ESP-NOW! Error: %s", esp_err_to_name(ret));
        return ret;
    }

    // Daftarkan callbacks
    esp_now_register_send_cb(espnow_send_cb);
    esp_now_register_recv_cb(espnow_recv_cb);

    // Tentukan target MAC (Peer spesifik atau Broadcast)
    const uint8_t *target = (peer_mac != NULL) ? peer_mac : s_espnow_broadcast_mac;
    memcpy(s_espnow_target_mac, target, ESP_NOW_ETH_ALEN);

    // Daftarkan target peer
    ret = com_tmpl_espnow_add_peer(target, channel, false);
    if (ret != ESP_OK && ret != ESP_ERR_ESPNOW_EXIST) {
        ESP_LOGE(TAG, "[ESP-NOW] Gagal mendaftarkan peer default!");
        return ret;
    }

    s_espnow_initialized = true;
    ESP_LOGI(TAG, "[ESP-NOW] Inisialisasi sukses! Target MAC: " MACSTR " | Channel: %u",
             MAC2STR(s_espnow_target_mac), (channel == 0) ? 1 : channel);

    return ESP_OK;
}

/**
 * @brief  Menambahkan peer baru ke daftar komunikasi ESP-NOW secara dinamis.
 * @param  peer_mac Alamat 6-byte MAC target.
 * @param  channel Channel Wi-Fi target.
 * @param  encrypt Status enkripsi paket (true / false).
 * @retval ESP_OK jika peer berhasil ditambahkan, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_add_peer(const uint8_t *peer_mac, uint8_t channel, bool encrypt) {
    if (peer_mac == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (esp_now_is_peer_exist(peer_mac)) {
        return ESP_OK;
    }

    esp_now_peer_info_t peer_info = {0};
    memcpy(peer_info.peer_addr, peer_mac, ESP_NOW_ETH_ALEN);
    peer_info.channel = (channel == 0) ? 1 : channel;
    peer_info.ifidx = WIFI_IF_STA;
    peer_info.encrypt = encrypt;

    esp_err_t ret = esp_now_add_peer(&peer_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ESP-NOW] Gagal menambahkan peer " MACSTR " (Error: %s)",
                 MAC2STR(peer_mac), esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "[ESP-NOW] Peer " MACSTR " berhasil didaftarkan (Channel %u)",
             MAC2STR(peer_mac), peer_info.channel);
    return ESP_OK;
}

/**
 * @brief  Mengirimkan paket data biner melalui gelombang radio ESP-NOW (TX Handler).
 * @param  data Pointer ke buffer data yang akan dikirimkan (maksimal 250 bytes).
 * @param  len Panjang data yang akan dikirim dalam bytes.
 * @retval ESP_OK jika paket berhasil dikirim ke antrean radio, atau esp_err_t jika gagal.
 */
esp_err_t com_tmpl_espnow_send(const void *data, size_t len) {
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_espnow_initialized) {
        ESP_LOGW(TAG, "[ESP-NOW TX] ESP-NOW belum diinisialisasi!");
        return ESP_ERR_INVALID_STATE;
    }

    size_t send_len = (len > ESP_NOW_MAX_DATA_LEN) ? ESP_NOW_MAX_DATA_LEN : len;
    esp_err_t ret = esp_now_send(s_espnow_target_mac, (const uint8_t *)data, send_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "[ESP-NOW TX] Gagal kirim %u bytes ke " MACSTR " (Error: %s)",
                 (unsigned int)send_len, MAC2STR(s_espnow_target_mac), esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

/**
 * @brief  Callback penerimaan data masuk ESP-NOW dari perangkat peer/master lain ke Com Hub.
 * @param  src_mac Pointer ke alamat MAC pengirim paket (6 bytes).
 * @param  data Pointer ke payload data yang diterima.
 * @param  len Panjang data payload dalam bytes.
 * @retval None
 */
void com_tmpl_espnow_on_recv(const uint8_t *src_mac, const uint8_t *data, int len) {
    if (data == NULL || len <= 0) return;

    // Pastikan MAC pengirim terdaftar sebagai peer agar ESP32 bisa membalas otomatis (Two-Way)
    if (src_mac != NULL && !esp_now_is_peer_exist(src_mac)) {
        com_tmpl_espnow_add_peer(src_mac, 1, false);
    }

    com_push_incoming_request(COM_IF_ESPNOW, data, (size_t)len);
}

/* =========================================================================
 * MASTER REGISTRATION HELPER
 * ========================================================================= */

/**
 * @brief  Mendaftarkan seluruh callback pengiriman data (TX Handlers) dari 8 protokol ke Com Hub.
 * @param  None
 * @retval None
 */
void com_templates_register_all_handlers(void) {
    com_register_tx_handler(COM_IF_UART, com_tmpl_uart_send);
    com_register_tx_handler(COM_IF_MODBUS, com_tmpl_modbus_send_response);
    com_register_tx_handler(COM_IF_LORA, com_tmpl_lora_send_packet);
    com_register_tx_handler(COM_IF_WIFI_HTTP, com_tmpl_http_send_response);
    com_register_tx_handler(COM_IF_MQTT, com_tmpl_mqtt_publish_response);
    com_register_tx_handler(COM_IF_CAN, com_tmpl_can_send_frame);
    com_register_tx_handler(COM_IF_BLE, com_tmpl_ble_send_notify);
    com_register_tx_handler(COM_IF_ESPNOW, com_tmpl_espnow_send);
}
