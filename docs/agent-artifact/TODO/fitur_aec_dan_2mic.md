# TODO: Implementasi Fitur AEC & 2-Microphone (Barge-in)

Panduan untuk mengaktifkan fitur mutakhir (AEC/Barge-in) agar AI bisa mendengarkan perintah Anda meskipun robot sedang berbicara atau memutar suara keras.

## 1. Kebutuhan Hardware (Wajib)
Fitur ini sangat berat dan membutuhkan spesifikasi tertentu:
- [ ] **Chip**: ESP32-S3 (Wajib punya fitur AI hardware).
- [ ] **PSRAM**: Minimal 2MB/8MB (Wajib, karena algoritma ESP-AFE sangat rakus RAM).
- [ ] **Microphone**: 2x Digital I2S Microphone (INMP441, SPH0645, atau ICS-43434).
- [ ] **Isolasi Akustik**: Microphone harus diletakkan sejauh mungkin dari Speaker dan idealnya dipisahkan oleh segel karet/foam agar suara speaker tidak langsung masuk kembali ke microphone.

## 2. Skema Penempatan (Acoustic Design)
Agar fitur **AEC (Acoustic Echo Cancellation)** bekerja optimal:
- [ ] Letakkan 2 mic dengan jarak sekitar 2-10 cm satu sama lain (untuk pembentukan *beamforming*).
- [ ] Gunakan sirkuit audio yang terpisah antara jalur Speaker (Output) dan Mic (Input).
- [ ] Pastikan driver I2S diatur ke mode **Stereo / 2-Channel** input.

## 3. Konfigurasi Software (Menuconfig)
Anda harus mengaktifkan beberapa parameter di sistem build:
- [ ] Jalankan `idf.py menuconfig`.
- [ ] Masuk ke menu **Xiaozhi Assistant**.
- [ ] Aktifkan: `USE_AUDIO_PROCESSOR` (Enable Audio Noise Reduction).
- [ ] Aktifkan: `USE_DEVICE_AEC` (Enable Device-Side AEC).

## 4. Keuntungan Fitur Ini
- **Barge-in Capable**: Anda bisa menghentikan pembicaraan robot hanya dengan memanggil "Xiao Zhi" (Wake Word) tanpa menekan tombol.
- **Noise Suppression**: Suara bising di latar belakang (kipas angin, tv) akan disaring lebih baik.
- **AEC**: AI akan secara otomatis "mengurangi" suara dirinya sendiri dari rekaman mic, sehingga ia hanya fokus mendengar suara manusia.

---

> [!WARNING]
> Menyalakan fitur AEC tanpa penempatan mic yang benar secara fisik tidak akan memberikan hasil maksimal. Pastikan ada penghalang fisik (shell robot) antara speaker dan mic.
