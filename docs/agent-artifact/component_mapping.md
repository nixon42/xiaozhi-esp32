# Quick Component Mapping

Berikut adalah referensi cepat file mana yang bertanggung jawab untuk komponen hardware tertentu:

### 1. Layar (Displays)
- **OLED (Small)**: `main/display/oled_display.cc`
- **LCD/TFT (Large)**: `main/display/lcd_display.cc`
- **UI Logic (LVGL)**: `main/display/lvgl_display/lvgl_display.cc`
- **Expression/Emotion**: `main/display/emote_display.cc`

### 2. Audio (Speaker & Mic)
- **Speaker Output**: `main/audio/audio_codec.h` (Interface) dan `main/audio/codecs/` (Implementasi hardware seperti ES8311).
- **Microphone Input**: Juga di `main/audio/codecs/` dan dikelola oleh `main/audio/audio_service.cc`.
- **Audio Processing (AEC/NS)**: `main/audio/processors/`.
- **Wake Word Detection**: `main/audio/audio_service.cc` (menggunakan library ESP-SR).

### 3. Konektivitas & Sistem
- **WiFi**: `main/boards/common/wifi_board.cc`
- **4G/LTE (ML307)**: `main/boards/common/ml307_board.cc`
- **Bluetooth/Protocol**: `main/protocols/`
- **Tombol (Buttons)**: `main/boards/common/button.cc`
- **Lampu (LED)**: `main/led/` (circular_strip.cc untuk RGB, gpio_led.cc untuk LED biasa).

### 4. Logic Utama
- **Status Perangkat**: `main/device_state_machine.cc`
- **Daftar Tool AI (MCP)**: `main/mcp_server.cc`
- **Konfigurasi Board**: `main/boards/[nama-board]/[nama-board].cc`
