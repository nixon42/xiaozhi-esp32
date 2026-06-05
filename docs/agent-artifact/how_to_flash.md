# Panduan Setup Board Deskbot Mimo

Agar ESP32-S3 kamu menggunakan konfigurasi `deskbot_mimo` yang telah kita buat, ikuti langkah-langkah berikut:

## 1. Memilih Profile di Menuconfig
1. Jalankan perintah berikut di terminal:
   ```bash
   idf.py menuconfig
   ```
2. Navigasi ke menu: **Xiaozhi Assistant** -> **Board Type**.
3. Cari dan pilih: `Deskbot Mimo (SSD1306 SPI)`.
4. Simpan konfigurasi (tekan `S`) lalu keluar (`Esc`).

## 2. Build dan Flash
Setelah board dipilih, lakukan build ulang agar firmware menggunakan pinout yang benar:
```bash
# Bersihkan build lama (jika perlu)
idf.py fullclean

# Build dan Flash
idf.py build flash monitor
```

## 3. Cek Spesifikasi (Optional)
Karena kamu menggunakan **Flash 16MB** dan **PSRAM 8MB**, pastikan setting berikut juga benar di `menuconfig`:
- **Serial Flasher Config** -> **Flash size** -> `16 MB`
- **Component Config** -> **ESP32S3-specific** -> **Support for external, SPI-connected RAM** -> `Enabled`

---
> [!NOTE]
> Jika butuh bantuan untuk memvalidasi isi [sdkconfig](file:///home/nixon/Project/xiaozhi-esp32/scripts/build_default_assets.py#456-480) agar benar-benar optimal untuk 16MB Flash, beri tahu saya ya!
