# TODO: Konfigurasi Hubungan ke Server Lokal

Langkah-langkah untuk menghubungkan perangkat ESP32 ke komputer yang menjalankan [xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server).

## 1. Persiapan Server (Sisi Komputer)
- [ ] Pastikan server lokal sudah berjalan (cek log terminal server).
- [ ] Dapatkan **Alamat IP Lokal** komputer Anda (misal: `192.168.1.10` di Windows atau `ifconfig` di Linux).
- [ ] Pastikan **Port 8000** (atau port yang Anda gunakan) tidak diblokir oleh Firewall komputer.

## 2. Setting Alamat di ESP32 (Sisi Perangkat)
Ada dua cara utama untuk mengarahkan ESP32 ke server lokal Anda:

### Cara A: Melalui Web Portal WiFi (Tanpa Flash Ulang)
1.  Nyalakan ESP32 Anda.
2.  Jika belum terhubung WiFi, cari WiFi AP bernama `XiaoZhi-XXXX` menggunakan HP.
3.  Buka browser ke alamat `192.168.4.1`.
4.  Isi kolom **WebSocket URL** dengan format:
    `ws://[IP_KOMPUTER]:8000/ws`
5.  Isi kolom **Token** sesuai dengan yang tertulis di `config.yaml` server Anda.
6.  Klik **Save** dan perangkat akan restart.

### Cara B: Melalui Menuconfig (Permanen)
1.  Jalankan `idf.py menuconfig`.
2.  Masuk ke **Xiaozhi Assistant** -> **Default OTA URL**.
3.  Ubah ke: `http://[IP_KOMPUTER]:8000/ota/`.
4.  *Catatan:* Server lokal harus mendukung endpoint OTA agar cara ini berhasil sepenuhnya untuk pembaruan fitur.

## 3. Verifikasi Koneksi
- [ ] Saat ESP32 menyala, perhatikan layar. Label **"Connecting..."** seharusnya berubah dengan cepat menjadi **"Idle"** (atau muncul ikon ekspresi).
- [ ] Periksa log di terminal komputer server. Pastikan muncul baris:
  `New connection from [IP_ESP32]`
- [ ] Coba panggil kata aktivasi (Wake Word) dan lihat apakah server merespons.

## 4. Troubleshooting Server Lokal
- **Handshake Error**: Cek apakah versi protokol (V2/V3) di firmware dan server sudah sama.
- **Connection Timed Out**: Coba `ping [IP_KOMPUTER]` dari perangkat lain di WiFi yang sama untuk memastikan IP tersebut bisa dijangkau.
- **Audio Lag**: Gunakan router WiFi 5GHz jika memungkinkan (untuk ESP32 yang mendukung) dan pastikan komputer server menggunakan koneksi kabel LAN agar latensi minimal.
