# Daftar Modul & Pinout Mimo Deskbot

Berikut adalah daftar modul hardware yang digunakan beserta pemetaan pin pada ESP32-S3-WROOM-1:

## 1. ESP32-S3-WROOM-1 (DevKit)
*   **Flash**: 16MB
*   **PSRAM**: 8MB (Octal SPI)
*   **Port**: USB Type-C

## 2. Mikrofon: INMP441 (I2S)
Modul mikrofon digital untuk menangkap suara user.
| Pin INMP441 | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| VDD | 3.3V (Wajib) | Power |
| GND | GND | Ground |
| **WS** | **GPIO 42** | I2S Word Select |
| **SCK** | **GPIO 41** | I2S Serial Clock |
| **SD** | **GPIO 2** | I2S Serial Data Out |
| L/R | GND | Left Channel |

**Catatan**: Karena hanya menggunakan **1 mic**, maka pin **L/R** wajib dihubungkan ke **GND** agar data suara masuk melalui channel kiri (Left Channel).

## 3. Amplifier: MAX98357 (I2S)
Modul DAC & Amplifier untuk output suara robot.
| Pin MAX98357 | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| VIN | 5V (Disarankan) | Power |
| GND | GND | Ground |
| **LRC** | **GPIO 42** | I2S Word Select |
| **BCLK** | **GPIO 41** | I2S Serial Clock |
| **DIN** | **GPIO 18** | I2S Serial Data In |
| **SD** | **3.3V / VCC** | Always On / Mono Mix |
| GAIN | NC / Floating | Gain 9dB (Fixed) |

**Catatan I2S Duplex & Barge-in**: 
Pin **BCLK (41)** dan **WS/LRC (42)** memang **sengaja digabung** (dishare) antara Mic dan Amp. Ini disebut mode **I2S Duplex**. 
*   **Kenapa digabung?** Agar input (suara user) dan output (suara robot) berjalan sinkron pada clock yang sama.
*   **Interupsi (Barge-in)**: Dengan clock yang sinkron, fitur **AEC (Acoustic Echo Cancellation)** pada ESP32-S3 bisa bekerja maksimal untuk menghilangkan suara robot sendiri dari input mic. Hasilnya, robot tetap bisa "mendengar" kata panggilanmu meskipun dia sedang berbicara keras.

## 4. Layar: OLED 0.96" SSD1306 (SPI 7-Pin)
Layar untuk menampilkan wajah dan status robot.
| Pin OLED | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| GND | GND | Ground |
| VCC | 3.3V | Power |
| **SCK (D0/CLK)** | **GPIO 15** | SPI Clock |
| **SDA (D1/MOSI)**| **GPIO 16** | SPI Data |
| **RES (RST)**| **GPIO 7** | Reset |
| **DC** | **GPIO 6** | Data/Command |
| **CS** | **GPIO 5** | Chip Select |

## 5. Motor Driver: TB6612FNG (Dual DC Motor)
Modul driver motor untuk menggerakkan roda robot.
| Pin TB6612FNG | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| **PWMA** | **GPIO 1** | PWM Motor A (Kecepatan) |
| **AIN1** | **GPIO 3** | Input A1 (Arah) |
| **AIN2** | **GPIO 4** | Input A2 (Arah) |
| **PWMB** | **GPIO 8** | PWM Motor B (Kecepatan) |
| **BIN1** | **GPIO 9** | Input B1 (Arah) |
| **BIN2** | **GPIO 10** | Input B2 (Arah) |
| **STBY** | **GPIO 11** | Standby (Active High) |
| VM | 5V - 12V | Power Motor |
| VCC | 3.3V | Power Logik |
| GND | GND | Ground |

## 6. Tombol Kontrol (Internal)
| Komponen | Pin ESP32-S3 | Fungsi |
| :--- | :--- | :--- |
| Button | **GPIO 0** | Hubungkan ke **GND** (Active Low) |

---
*Catatan: Pastikan menggunakan kabel jumper yang pendek dan berkualitas untuk jalur I2S & SPI guna menghindari interferensi suara/gambar.*