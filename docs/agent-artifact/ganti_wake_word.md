# Panduan Mengganti Wake Word (Kata Pemanggil)

Secara default, robot ini menggunakan kata panggilan **"Xiaozhi"**. Jika kamu ingin menggantinya, ada dua cara utama tergantung pada metode deteksi yang digunakan.

## Metode 1: Menggunakan Custom Wake Word (Cara Termudah via Firmware)
Metode ini menggunakan fitur "Custom Wake Word" dari Espressif yang memungkinkan kamu menentukan kata panggilan sendiri melalui teks (phonetic).

### Langkah-langkah:
1.  Jalankan `idf.py menuconfig`.
2.  Masuk ke menu **Xiaozhi Assistant** -> **Wake Word Type**.
3.  Pilih **Use Custom Wake Word**.
4.  Isi konfigurasi berikut:
    *   **Custom Wake Word**: Masukkan kata panggil dalam format pinyin atau phonetic (contoh: `ni hao xiao ya`).
    *   **Custom Wake Word Display**: Masukkan teks yang akan muncul di layar saat terpanggil (contoh: `Halo Robot`).
5.  Simpan, Build, dan Flash ulang:
    ```bash
    idf.py build flash
    ```

---

## Metode 2: Menggunakan AFE Wake Word (Default)
Metode ini lebih akurat karena menggunakan model AI yang sudah dilatih, namun lebih sulit untuk dikustomisasi sendiri oleh pengguna biasa.

*   **Cara Kerja**: Model suara disimpan di dalam file `srmodels.bin` yang ada di dalam partisi `assets`.
*   **Cara Mengganti**: Kamu harus memiliki file `srmodels.bin` baru yang berisi model wake word yang diinginkan, kemudian mem-flash ulang partisi `assets`.

---

## Tips Memilih Kata Panggilan
Agar robot mudah mengenali panggilanmu, pilihlah kata yang:
-   **Panjang (3-4 suku kata)**: Contoh "Halo Robot" lebih baik daripada "Hai".
-   **Vokal Jelas**: Hindari kata-kata yang terlalu pendek atau mirip dengan percakapan sehari-hari.
-   **Unik**: Agar tidak sering terjadi *false trigger* (robot tiba-tiba menyala saat kamu sedang mengobrol biasa).

---

> [!IMPORTANT]
> Jika kamu mengganti wake word di sisi robot, pastikan server AI kamu (jika menggunakan server kustom) juga dikonfigurasi untuk mengenali kata panggil tersebut agar percakapan tetap nyambung.
