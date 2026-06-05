# Daftar Tool MCP (Model Context Protocol)

Project ini mengimplementasikan berbagai tool MCP yang memungkinkan LLM (AI) untuk berinteraksi langsung dengan hardware ESP32. Tool ini dibagi menjadi beberapa kategori:

## 1. Sistem & Status (Common Tools)
Tool ini tersedia secara umum untuk memberikan informasi dasar tentang perangkat.

| Nama Tool | Deskripsi | Parameter |
| :--- | :--- | :--- |
| `self.get_device_status` | Memberikan info real-time perangkat (speaker, layar, baterai, network). | - |
| `self.audio_speaker.set_volume` | Mengatur volume speaker (0-100). | `volume` (int) |
| `self.screen.set_brightness` | Mengatur kecerahan layar (0-100). | `brightness` (int) |
| `self.screen.set_theme` | Mengatur tema visual (light/dark). | `theme` (string) |

## 2. Media & Multimedia
Tool untuk interaksi visual dan pengambilan data sensor.

| Nama Tool | Deskripsi | Parameter |
| :--- | :--- | :--- |
| `self.camera.take_photo` | Mengambil foto dan meminta AI menjelaskan isinya. | `question` (string) |

## 3. Internal & Pemeliharaan (User Only Tools)
Tool ini biasanya digunakan untuk debugging atau tugas administratif perangkat.

| Nama Tool | Deskripsi | Parameter |
| :--- | :--- | :--- |
| `self.get_system_info` | Mendapatkan informasi sistem mendalam. | - |
| `self.reboot` | Melakukan reboot pada perangkat. | - |
| `self.upgrade_firmware` | Mengunduh dan menginstal firmware dari URL. | `url` (string) |
| `self.screen.get_info` | Info dimensi layar dan mode warna. | - |
| `self.screen.snapshot` | Mengambil snapshot layar dan mengunggah ke URL. | `url`, `quality` |
| `self.screen.preview_image` | Menampilkan gambar dari URL ke layar. | `url` (string) |
| `self.assets.set_download_url` | Mengatur URL unduhan untuk aset (font/emoji). | `url` (string) |

## 4. Board-Specific (Custom Hardware)
Tool yang hanya tersedia jika firmware dikompilasi untuk board tertentu (seperti Robot).

### Umum (Beberapa Board)
- `self.system.reconfigure_wifi`: Menghentikan percakapan dan masuk ke mode konfigurasi WiFi.

### Robot "Otto" (`otto_controller.cc`)
- `self.otto.action`: Menjalankan gerakan (walk, turn, jump, swing, sit, dll).
- `self.otto.servo_sequences`: Pemrograman gerakan kustom via JSON.
- `self.otto.stop`: Berhenti mendadak dan kembali ke posisi Home.
- `self.otto.set_trim` / `get_trims`: Kalibrasi posisi servo.
- `self.otto.get_status`: Mengecek apakah robot sedang bergerak atau diam.

### Robot "Electron" (`electron_bot_controller.cc`)
- `self.electron.hand_action`: Kontrol tangan (angkat, turun, lambaian, tepuk).
- `self.electron.body_turn`: Putar badan (kiri, kanan, tengah).
- `self.electron.head_move`: Gerakan kepala (tengok atas, bawah, mengangguk).
- `self.electron.stop` / `get_status`: Kontrol status gerakan.
- `self.electron.set_trim` / `get_trims`: Kalibrasi servo Electron Bot.
- `self.battery.get_level`: Info baterai khusus robot.

> [!NOTE] 
> Ketersediaan tool di atas tergantung pada konfigurasi hardware yang dipilih saat proses kompilasi (`BOARD_TYPE`).
