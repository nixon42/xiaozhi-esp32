# TODO: Implementasi OLED SSD1306 untuk Mata Robot

Panduan teknis bagi yang ingin menggunakan layar OLED 0.96"/1.3" monokrom sebagai display emosi/mata robot.

## 1. Persiapan Aset (Monokrom 1-Bit)
Jangan menggunakan file GIF berwarna asli karena hasilnya akan berantakan di layar OLED.
- [ ] Unduh aset GIF mata dari repository `txp666/otto-emoji-gif-component`.
- [ ] Konversi manual setiap frame GIF ke format **1-bit (Black & White)**.
- [ ] Gunakan teknik **Fixed Threshold** (bukan Dithering) agar garis mata terlihat solid dan tidak berbintik.
- [ ] Pastikan resolusi akhir adalah **128x64** (untuk SSD1306). 

## 2. Interface Komunikasi: Prioritaskan SPI
Untuk mendapatkan animasi mata yang mulus pada OLED, gunakan model **4-wire SPI** alih-alih I2C.
- **Refresh Rate**: SPI dapat mencapai **30-60 FPS**, jauh lebih tinggi dibanding I2C yang hanya ~10 FPS.
- **Pinout Standar**:
  - **SCLK**: Clock
  - **MOSI**: Data
  - **CS**: Chip Select
  - **DC**: Data/Command
  - **RST**: Reset

## 2. Konfigurasi Driver & LVGL
Untuk efisiensi, driver harus diatur agar tidak memproses data warna yang sia-sia:
- [ ] **Menuconfig**: Pastikan tipe board diatur ke `Custom Board` atau profil `OLED-based` (seperti EchoEar).
- [ ] **Color Depth**: Di konfigurasi LVGL, gunakan `LV_COLOR_DEPTH 1` atau pastikan buffer dirender sebagai format **Monokrom**.
- [ ] **esp_lcd_ssd1306**: Pastikan project menggunakan driver `esp_lcd_ssd1306` dengan konfigurasi **SPI Panel** untuk pengiriman data bitmask yang sangat cepat.

## 3. Modifikasi Kode Inisialisasi
Di file board custom Anda, gunakan class `OledDisplay` yang sudah dioptimasi untuk monochrome:

```cpp
// Contoh (main/boards/custom-board/custom_board.cc)
display_ = new OledDisplay(i2c_bus, width, height, flip_screen);
```

## 4. Keuntungan & Tantangan
- **Keuntungan**: Harga paling murah, sangat tipis dan ringan untuk robot kecil, konsumsi daya sangat rendah.
- **Tantangan**: Refresh rate terbatas (terutama via I2C), hanya satu warna, dan ukuran area mata terbatas.

---

> [!TIP]
> Karena Anda menggunakan model **SPI**, Anda bisa menaikkan SPI clock hingga **10MHz**. Ini akan membuat pemindahan data gambar ke OLED hampir instan, memberikan kesan mata robot yang sangat responsif dan tajam.
