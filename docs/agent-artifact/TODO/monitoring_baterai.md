# TODO: Monitoring Baterai (TP4056 + Resistor Divider)

Panduan teknis untuk mengaktifkan indikator baterai pada board custom menggunakan charger TP4056 dan pembagi tegangan (voltage divider).

## 1. Skema Rangkaian Hardware
Karena pin ADC ESP32 hanya aman hingga 3.3V, sedangkan baterai Li-ion bisa mencapai 4.2V, kita butuh dua resistor untuk menurunkan tegangan.

```text
BAT+ (4.2V) ---- [ R1: 100kΩ ] ----+---- [ R2: 100kΩ ] ---- GND
                                   |
                                   V
                          Ke PIN ADC ESP32 
                          (Misal: GPIO 4)
```

**Opsional (Status Charging):**
Hubungkan pin **CHRG** pada modul TP4056 ke salah satu GPIO ESP32 (misal GPIO 5) dengan bantuan resistor pull-up 10kΩ ke 3.3V. Ini berguna agar ikon baterai di layar berubah menjadi ikon petir saat sedang dicas.

## 2. Pendaftaran di Kode Board (C++)
Buka file board custom Anda (misal `boards/my-board/my_board.cc`) dan inisialisasi class `AdcBatteryMonitor`.

```cpp
#include "adc_battery_monitor.h"

// Di dalam constructor board:
battery_monitor_ = new AdcBatteryMonitor(
    ADC_UNIT_1, 
    ADC_CHANNEL_3,  // Pastikan sesuai dengan pin fisik (GPIO 4 = Channel 3 di ESP32-S3)
    100000.0f,      // R1: 100k Ohm
    100000.0f,      // R2: 100k Ohm
    GPIO_NUM_5      // Pin monitoring status charging (opsional)
);
```

## 3. Konfigurasi Kconfig
Pastikan fitur monitoring baterai diaktifkan melalui menuconfig:
- Jalankan `idf.py menuconfig`.
- Masuk ke **Component config** -> **ADC Battery Estimation**.
- Aktifkan fitur tersebut dan sesuaikan range tegangannya (biasanya 3.3V - 4.2V).

## 4. Keuntungan Setup Ini
- **Icon Baterai Akurat**: Layar akan menampilkan 0-100% berdasarkan kurva tegangan baterai Li-ion.
- **Auto Deep Sleep**: Project XiaoZhi secara otomatis akan masuk ke mode hemat daya jika tegangan baterai terdeteksi terlalu rendah (< 3.4V).
- **Status Charging**: Muncul animasi kilat pada ikon baterai saat TP4056 sedang mengisi daya.
