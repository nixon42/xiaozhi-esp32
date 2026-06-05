# Komponen Hardware yang Didukung (Native)

Proyek XiaoZhi ESP32 mendukung berbagai macam komponen hardware secara bawaan (native). Jika Anda membangun board custom, Anda bisa memilih komponen dari daftar ini agar integrasi kodenya lebih mudah.

## 1. Microcontroller (SoC)
Project ini dikembangkan menggunakan ESP-IDF dan mendukung:
- **ESP32** (Standard)
- **ESP32-S3** (Sangat disarankan karena fitur AI & PSRAM)
- **ESP32-C3 / C4 / C5 / C6**
- **ESP32-P4** (High performance, didukung di branch/versi terbaru)
- **ESP32-H2**

## 2. Audio Codec (Suara & Mic)
Mendukung protokol I2S untuk output speaker dan input mikrofon:
- **Everest Semiconductor**: ES8311, ES8388, ES8374, ES8389.
- **Internal DAC/ADC**: ESP32 internal (kualitas rendah).
- **ESP-BOX Codec**: Integrasi native dengan seri ESP-BOX dari Espressif.

## 3. Layar (Display)
Mendukung berbagai driver layar lewat library LVGL dan SPI/RGB/DSI interface:
- **LCD Driver**: 
    - ST7789, ST7701, ST7796, ST7735, ST77916
    - ILI9341, ILI9881C
    - GC9A01 (Layar bulat)
    - SPD2010, NV3023
    - JD9365, EK79007
- **OLED Driver**: 
    - SH1106, SSD1306
- **Interface**: SPI, RGB (Parallel), MIPI DSI, QSPI.

## 4. Touch Controller (Layar Sentuh)
- **FT5x06, GT911, GT1151**
- **CST816S, CST9217**
- **ST7123**

## 5. Konektivitas & Modem
- **Wi-Fi**: Dukungan native semua seri ESP32.
- **Modem 4G/LTE**: 
    - **ML307** (Populer di China/Global)
    - **NT26**
- **Ethernet**: Via SPI atau RNDIS.
- **LoRa**: Didukung pada board tertentu (Lilygo T-Display S3 Pro).

## 6. Sensor & Power Management
- **PMIC (Power Management)**: AXP2101, SY6970.
- **Battery Monitor**: ADC-based estimation.
- **IMU (Motion)**: BMI270.
- **Expander (GPIO Tambahan)**: TCA9554, TCA95xx, CH32V003.

## 7. Peripheral Lainnya
- **LED**: 
    - Single LED (GPIO)
    - Addressable LED Strip (WS2812, SK6812 via `led_strip` component)
- **Input**: 
    - Tactile Buttons
    - Rotary Encoder (Knob)
    - Touch Slider
- **Camera**: OV2640 dan sensor kamera standar lainnya yang didukung `esp32-camera`.

---

> [!TIP]
> Jika Anda menggunakan komponen di luar daftar ini, Anda mungkin perlu menambahkan driver kustom di folder `main/boards/common/` atau menyesuaikan `idf_component.yml`.
