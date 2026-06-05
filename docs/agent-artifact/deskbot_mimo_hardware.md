# Dokumentasi Hardware: Deskbot Mimo (SSD1306 SPI)

Dokumen ini menjelaskan konfigurasi pin dan modul yang digunakan untuk profil board custom `deskbot_mimo` pada proyek XiaoZhi ESP32.

## 1. Spesifikasi Utama
- **Microcontroller**: ESP32-S3-WROOM-1 (Type-C)
- **Memory**: Flash 16MB, PSRAM 8MB
- **Display**: 0.96" OLED SSD1306 SPI
- **Audio Interface**: I2S Duplex
- **Fitur Khusus**: Mendukung animasi mata robot (`otto-gif`)

## 2. Tabel Pemetaan Pin (Pinout)

### A. Display (SSD1306 SPI)
| Fungsi SPI | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **SDA/MOSI** | GPIO 16 | Data Serial |
| **SCK/SCLK** | GPIO 15 | Clock Serial |
| **CS** | GPIO 5 | Chip Select |
| **DC** | GPIO 6 | Data / Command |
| **RST** | GPIO 7 | Reset Hardware |

### B. Audio (I2S Interface)
Digunakan untuk modul spesifik:
- **Mikrofon**: INMP441 (Omnidirectional MEMS)
- **Amplifier**: MAX98357 (3W Class D)

| Fungsi I2S | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **BCLK** | GPIO 40 | Bit Clock |
| **WS / LRCK** | GPIO 41 | Word Select / Left-Right Clock |
| **DIN** | GPIO 38 | Data In (INMP441 SD) |
| **DOUT** | GPIO 18 | Data Out (MAX98357 DIN) |



### C. Input & Kontrol
| Fungsi | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **Boot Button** | GPIO 0 | Tombol kontrol utama / Flash |

### D. Motor Driver (TB6612FNG)
Digunakan untuk menggerakkan 2 roda (Differential Drive).

| Fungsi | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **PWMA** | GPIO 1 | PWM Motor A (Speed) |
| **AIN1** | GPIO 3 | Direction A1 |
| **AIN2** | GPIO 4 | Direction A2 |
| **PWMB** | GPIO 8 | PWM Motor B (Speed) |
| **BIN1** | GPIO 9 | Direction B1 |
| **BIN2** | GPIO 10 | Direction B2 |
| **STBY** | GPIO 11 | Standby (Active High) |
| **LED Status** | GPIO 48 | WS2812 Built-in LED |

### E. Motor Driver (DRV8833) - *Alternative*
DRV8833 hanya membutuhkan 4 pin untuk mengontrol 2 motor.

| Fungsi | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **AIN1** | GPIO 1 | PWM Input A1 |
| **AIN2** | GPIO 3 | PWM Input A2 |
| **BIN1** | GPIO 8 | PWM Input B1 |
| **BIN2** | GPIO 9 | PWM Input B2 |
> [!NOTE]
> Pin AIN1/AIN2 dan BIN1/BIN2 dipetakan ulang dari konfigurasi standar TB6612 jika memilih tipe driver DRV8833 di `config.h`.

### F. Sensor VL53L0X
Sensor jarak Time-of-Flight (ToF) untuk deteksi objek.

| Fungsi I2C | Pin ESP32-S3 | Keterangan |
| :--- | :---: | :--- |
| **SDA** | GPIO 17 | Serial Data |
| **SCL** | GPIO 46 | Serial Clock |

> [!NOTE]
> Sensor ini menggunakan alamat I2C default `0x29`. Pastikan modul ditenagai dengan 3.3V.

## 3. Modul yang Disarankan
- **OLED**: SSD1306 128x64 dengan interface 7-pin SPI.
- **Audio Output**: MAX98357A I2S Amplifier.
- **Audio Input**: INMP441 I2S Microphone.
- **Motor Driver**: TB6612FNG atau DRV8833 Dual DC Motor Driver.

## 4. Konfigurasi Motor (TB6612FNG)
Deskbot Mimo menggunakan sistem **2-wheel differential drive** dengan 1 caster wheel di bagian belakang/depan.

### Wiring Power
- **VM**: Sambungkan ke sumber daya motor (5V - 12V).
- **VCC**: Sambungkan ke 3.3V dari ESP32.
- **GND**: Sambungkan ke Ground (pastikan Common Ground).

### Tabel Logika Kontrol
| AIN1 / BIN1 | AIN2 / BIN2 | PWM | Status |
| :---: | :---: | :---: | :--- |
| High | Low | High | Maju (Forward) |
| Low | High | High | Mundur (Backward) |
| Low | Low | Any | Berhenti (Short Brake) |
| Any | Any | Low | Berhenti (Stop) |

> [!IMPORTANT]
> Pin **STBY** harus diberi tegangan **High (3.3V)** agar driver aktif. Dalam profil ini, STBY dikontrol lewat GPIO 11.

## 5. Cara Menggunakan Profil Ini

### Konfigurasi Build
1. Jalankan `idf.py menuconfig`.
2. Masuk ke menu **Xiaozhi Assistant**.
3. Pada bagian **Board Type**, pilih `Deskbot Mimo (SSD1306 SPI)`.
4. Jika menggunakan driver DRV8833, ubah `#define DESKBOT_MOTOR_TYPE` di `main/boards/deskbot-mimo/config.h` menjadi `MOTOR_TYPE_DRV8833`.
5. Simpan dan keluar (`S` lalu `Esc`).

### Persiapan & Flash Animasi (GIF)
Profil ini dikonfigurasi untuk menggunakan koleksi aset `otto-gif`. File GIF akan dikemas secara otomatis oleh sistem build ke dalam `assets.bin`.

#### Tahapan Flash Asset:
1.  **Konfigurasi**: Jalankan `idf.py menuconfig`, masuk ke **Xiaozhi Assistant** -> **Flash Assets** -> pilih **Flash Default Assets**.
2.  **Flash Otomatis**: Hubungkan ESP32-S3 dan jalankan:
    ```bash
    idf.py build flash
    ```
    Sistem akan otomatis mendeteksi partisi `assets` dan mengisi file GIF ke alamat flash yang sesuai (biasanya `0x800000`).
3.  **Update Asset Saja**: Jika hanya ingin memperbarui file GIF tanpa flash firmware:
    ```bash
    esptool.py write_flash 0x800000 build/assets.bin
    ```

> [!TIP]
> Gunakan GIF beresolusi maksimal **128x64** agar pas dengan layar OLED Mimo.

### Kompilasi & Monitoring
```bash
idf.py monitor
```
Pastikan pada log monitor muncul keterangan `[Assets] Found assets partition`.

---
*Dokumen ini dibuat secara otomatis untuk board `deskbot_mimo`.*
