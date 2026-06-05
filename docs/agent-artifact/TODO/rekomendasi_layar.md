# TODO: Rekomendasi Layar High Refresh Rate (~30 FPS)

Agar animasi mata robot terlihat mulus (smooth) seperti pada video demonya, Anda memerlukan layar dengan driver yang mendukung kecepatan clock tinggi dan optimasi DMA.

## 1. Rekomendasi Modul Layar

| Tipe Layar | Driver | Resolusi | Interface | Keunggulan |
|------------|--------|----------|-----------|------------|
| **1.28" Round** | **GC9A01** | 240x240 | SPI | Bentuk bulat, sangat cocok untuk mata robot tunggal/pusat. |
| **1.54" Square**| **ST7789** | 240x240 | SPI | Ukuran standar Otto/Electron, refresh rate sangat stabil. |
| **2.1" Round** | **ST7701S** | 480x480 | **RGB/SPI** | Sangat mulus, tapi membutuhkan banyak GPIO (S3 only). |
| **0.96" OLED** | **SSD1306** | 128x64 | I2C/SPI | Murah, tapi refresh rate rendah (tidak disarankan untuk GIF). |

## 2. Tips Mencapai 30 FPS pada ESP32-S3

Untuk mencapai performa maksimal (~30 FPS) pada board custom Anda:

- [ ] **Gunakan SPI Clock Tinggi**: Set SPI clock ke **40MHz** atau **80MHz** di konfigurasi driver. ST7789 dan GC9A01 biasanya stabil di 80MHz dengan jalur kabel pendek.
- [ ] **Aktifkan DMA (Direct Memory Access)**: Memastikan proses pengiriman data gambar ke layar tidak membebani CPU utama.
- [ ] **Gunakan PSRAM**: Animasi GIF didekompresi ke RAM. Tanpa PSRAM, animasi akan sering patah-patah (stuttering).
- [ ] **Konfigurasi LVGL**: 
    - Set `LV_DISP_DEF_REFR_PERIOD` ke `33` (asumsi 1000ms / 30fps).
    - Gunakan **Double Buffering** (dua buffer layar) sebesar minimal 1/10 resolusi layar.

## 3. Contoh Pinout Ideal (ESP32-S3)
Gunakan pin High Speed untuk SPI (biasanya pin GPIO 1-21 pada S3):
- **MOSI**: GPIO 11
- **SCLK**: GPIO 12
- **CS**: GPIO 10
- **DC**: GPIO 13
- **RST**: GPIO 14
- **BL (Backlight)**: GPIO 48 (PWM)

## 4. Tempat Membeli (Keywords)
Cari di marketplace dengan kata kunci:
1. "1.28 inch Round LCD GC9A01 SPI"
2. "1.54 inch LCD ST7789 SPI 240x240"
3. "ESP32-S3 Touch LCD Parallel" (Jika ingin layar lebih besar yang sangat mulus).

---

> [!IMPORTANT]
> Untuk proyek robot ini, **ST7789 1.54 inch** adalah pilihan paling aman karena didukung secara native oleh hampir semua profil board robot dalam project ini.
