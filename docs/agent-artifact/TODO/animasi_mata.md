# TODO: Implementasi Animasi Mata Robot pada Custom Board

Daftar tugas untuk mengaktifkan fitur animasi mata (GIF) pada hardware custom yang tidak menggunakan profil standar Otto/Electron.

## 1. Konfigurasi Sistem Build (CMake)
- [ ] Buka `main/CMakeLists.txt`.
- [ ] Tambahkan konfigurasi untuk board custom baru (misal: `CONFIG_BOARD_TYPE_MY_CUSTOM`).
- [ ] Atur variabel `set(DEFAULT_EMOJI_COLLECTION otto-gif)` di dalam blok board tersebut.

## 2. Implementasi Hardware Abstraction Layer (HAL)
- [ ] Buat folder board baru di `main/boards/my-custom-board/`.
- [ ] Implementasikan file `.cc` dan `.h` untuk board tersebut.
- [ ] Pada saat inisialisasi display, gunakan class `OttoEmojiDisplay` alih-alih `LcdDisplay` atau `SpiLcdDisplay`.
    - *Contoh:* `display_ = new OttoEmojiDisplay(panel_io, panel, width, height, ...);`

## 3. Persiapan Partisi & Assets
- [ ] Pastikan file `partitions.csv` memiliki partisi bernama `assets`.
- [ ] Siapkan file `assets.bin` yang berisi asset GIF mata robot (dapat diambil dari repository original XiaoZhi).
- [ ] Flash firmware sekaligus partisi assets menggunakan perintah: 
  `idf.py flash` dan `esptool.py write_flash 0x[alamat_partisi] assets.bin`.

## 4. Optimasi Memory & Hardware
- [ ] Pastikan board custom menggunakan modul ESP32 dengan **PSRAM** (minimal 2MB/8MB disarankan).
- [ ] Verifikasi resolusi layar di kode cocok dengan fisik layar agar animasi GIF tidak terpotong atau berantakan.

## 5. Pengujian Interaksi
- [ ] Hubungkan ke server AI.
- [ ] Kirimkan voice command yang memicu emosi (misal: "Halo, senang bertemu kamu").
- [ ] Verifikasi apakah mata robot berubah ekspresi sesuai perintah server.
