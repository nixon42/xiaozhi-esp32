# Implementasi Custom Wake Word (Multinet Model)

Berikut adalah detail implementasi fitur "Multinet model (Custom Wake Word)" di proyek ini:

## 1. Lokasi Implementasi
Fitur ini diimplementasikan di:
- [custom_wake_word.cc](file:///home/nixon/Project/xiaozhi-esp32/main/audio/wake_words/custom_wake_word.cc)
- [custom_wake_word.h](file:///home/nixon/Project/xiaozhi-esp32/main/audio/wake_words/custom_wake_word.h)

## 2. Model yang Digunakan
Fitur ini menggunakan model **MultiNet (esp-mn)** dari framework **ESP-SR** milik Espressif. 
- **Bukan** menggunakan `microwakeword`.
- MultiNet aslinya didesain untuk *offline command recognition* (pengenalan perintah suara offline), namun di sini dimanfaatkan sebagai *wake word engine* kustom.

## 3. Alur Perbedaan dibanding WakeNet (Default ESP-SR)
Secara teknis, keduanya adalah bagian dari `esp-sr`, tapi memiliki perbedaan alur deteksi:

| Fitur | WakeNet (Default) | MultiNet (Custom Wake Word) |
| :--- | :--- | :--- |
| **Model** | Pre-trained CNN (Fixed keyword) | MultiNet Command Recognition Engine |
| **Kustomisasi** | Harus dilatih ulang oleh Espressif/User (sulit) | Bisa langsung diset via teks pinyin/phonetic |
| **Akurasi** | Sangat tinggi, rendah false trigger | Tergantung kemiripan fonetik, bisa terjadi false trigger |
| **Resource** | Lebih ringan, dioptimalkan untuk continuous detection | Sedikit lebih berat karena engine command recognition aktif |

### Alur Kerja:
1.  **Audio Input**: Mic -> AudioFrontEnd (AFE) -> PCM.
2.  **Detection**: Engine MultiNet akan mencocokkan PCM audio dengan daftar perintah (commands) yang didaftarkan.
3.  **Command Dynamic**: Pada `custom_wake_word.cc`, kode mendaftarkan `CONFIG_CUSTOM_WAKE_WORD` sebagai perintah suara via `esp_mn_commands_add`.
4.  **Trigger**: Jika `mn_state == ESP_MN_STATE_DETECTED`, perangkat akan dianggap "bangun" (wake).

### Kelebihan:
Kamu bisa mengganti kata panggil sesukamu hanya dengan mengubah konfigurasi di `menuconfig` (format pinyin) tanpa perlu file model `.bin` yang spesifik untuk kata tersebut.

---

## Contoh: Implementasi "Hai, Mimo" (Bahasa Inggris)

Jika Anda ingin menggunakan kata panggil bahasa Inggris seperti **"Hai, Mimo"**, ikuti langkah-langkah detail berikut:

### Langkah 1: Aktivasi Model Inggris di Menuconfig
Karena secara default proyek ini menggunakan model Mandarin, Anda harus mengaktifkan model Inggris di ESP-SR:
1. Jalankan `idf.py menuconfig`.
2. Masuk ke menu **Component config** -> **ESP Speech Recognition**.
3. Cari bagian **Select MultiNet model**.
4. Aktifkan **English (English)**.
5. (Opsional) Nonaktifkan **Chinese (Mandarin)** untuk menghemat memori flash jika tidak digunakan.

### Langkah 2: Konfigurasi Custom Wake Word
Setelah model Inggris diaktifkan, atur kata panggilnya:
1. Kembali ke menu utama, masuk ke **Xiaozhi Assistant**.
2. Masuk ke **Wake Word Implementation Type** -> Pilih **Multinet model (Custom Wake Word)**.
3. Atur konfigurasi berikut:
   - **Custom Wake Word**: `hai mi mo`
   - **Custom Wake Word Display**: `Hai, Mimo`
   - **Custom Wake Word Threshold (%)**: `20` (Jika sulit terpanggil, coba turunkan ke `15` atau `10`).

### 5. Menggunakan Multiple Wake Words (Banyak Kata Panggil)
Sekarang Anda bisa menggunakan lebih dari satu kata panggil sekaligus (misal: "Hai Mimo", "Mimo", dan "Hello Mimo").

**Cara Setting di menuconfig:**
1. Masuk ke menu **Xiaozhi Assistant** -> **Custom Wake Word**.
2. Di bagian **Custom Wake Word**, ketikkan beberapa frasa dipisahkan tanda koma:
   `hi mi mo, mimo, hello mimo`
3. Di bagian **Custom Wake Word Display**, ketikkan tampilan teksnya (urutan harus sama):
   `Hai Mimo, Mimo, Hello Mimo`
4. **Build dan Flash** ulang. Sistem akan otomatis mendaftarkan semua kata tersebut ke dalam engine MultiNet.

---

### Tips Optimasi & Troubleshooting
- **Threshold**: Jika sensitivitas kurang, turunkan `Custom Wake Word Threshold (%)` di menuconfig (coba 5-10%).
- **Model**: Sangat disarankan menggunakan `mn6_en` atau `mn7_en` untuk akurasi terbaik.
- **Ejaan**: Gunakan ejaan fonetik Inggris yang sederhana (misal pakai `hi` bukan `hai` untuk model English).
- **Log**: Cek serial monitor untuk melihat apakah perintah sudah terdaftar:
  `I (xxxx) CustomWakeWord: Command: hi mi mo, Text: Hai Mimo, Action: wake` 

### Langkah 3: Build & Flash
Simpan konfigurasi dan flash ulang perangkat:
```bash
idf.py build flash
```

> [!TIP]
> Meskipun menggunakan model Inggris, input pada **Custom Wake Word** tetap menggunakan spasi antar kata. Engine MultiNet akan mencocokkan fonetik terdekat dari daftar model yang tersedia di build.
