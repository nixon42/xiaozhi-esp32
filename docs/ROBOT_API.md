# DAPUR.AI SOPHIE - Robot Communication API

Dokumen ini menjelaskan bagaimana Firmware / Core Robot dapat mengirimkan sinyal perubahan *state*, monolog, dan tampilan layar ke UI (Display Engine) hanya menggunakan **HTTP GET Requests** sederhana.

Tidak perlu menggunakan WebSocket atau MQTT. UI akan otomatis tersinkronisasi secara *real-time* begitu HTTP GET request diterima oleh *endpoint* trigger.

## 1. Konfigurasi Endpoint

Display Engine (Web Server) mengekspos endpoint API di port **8080** (default pada Docker).

**Base URL:**
`http://<IP_DISPLAY_ENGINE>:8080/api/trigger`

---

## 2. Mengubah Mode Layar dan Percakapan (Display State)

Setiap kali Robot berubah mode (mendengarkan, berbicara) atau ingin menampilkan halaman informasi (*dashboard*), kirimkan HTTP GET dengan parameter `event`.

### Endpoint
`GET /api/trigger?event=<STATE>`

### Parameter yang Valid:

**Mode Percakapan:**
- `ATTRACT` : Mengembalikan UI ke mode Slideshow Monolog.
- `LISTENING` : Mengubah UI ke animasi gelombang suara biru (Mic / Audio In).
- `THINKING` : Mengubah UI ke animasi *neural network* ungu (Proses LLM).
- `SPEAKING` : Mengubah UI ke animasi lingkaran emas (Audio Out).

**Mode Layar Informasi (Kiosk Dashboards):**
- `SHOW_SOPHIE_INFO` : Menampilkan arsitektur sistem robot SOPHIE (AI + Hardware).
- `SHOW_AURA_INFO` : Menampilkan informasi tentang hasil foto *Aura / Cosmic Origin*.
- `SHOW_DAPURAI_INFO` : Menampilkan profil perusahaan, keahlian, dan prinsip AI Dapur.AI.
- `SHOW_VIR_INFO` : Menampilkan dashboard radar sensor dari *Vortex Intelligence Robot* (VIR).

### Contoh Request (cURL)
```bash
# Robot mulai mendengarkan suara user
curl "http://localhost:8080/api/trigger?event=LISTENING"

# Memunculkan dashboard VIR
curl "http://localhost:8080/api/trigger?event=SHOW_VIR_INFO"
```

---

## 3. Men-trigger Monolog / Audio (Attract Mode)

Saat Robot berada dalam mode *Idle* dan memutar rekaman audio monolog tertentu (misalnya file `T001_id`), Robot harus memberi tahu UI agar menampilkan teks untuk monolog tersebut.

UI sudah dirancang agar:
- Jika diakhiri dengan `_id`, maka teks Bahasa Indonesia akan ditampilkan besar, dan Bahasa Inggris ditampilkan kecil di bawahnya.
- Jika diakhiri dengan `_en`, sebaliknya (Bahasa Inggris besar).

### Endpoint
`GET /api/trigger?event=PLAY_MONOLOGUE&signal=<ID_MONOLOG>`

### Contoh Request (cURL)
```bash
# Memutar T001 (Bahasa Indonesia diutamakan)
curl "http://localhost:8080/api/trigger?event=PLAY_MONOLOGUE&signal=T001_id"

# Memutar T002 (Bahasa Inggris diutamakan)
curl "http://localhost:8080/api/trigger?event=PLAY_MONOLOGUE&signal=T002_en"
```

### Respons (Success)
Jika request berhasil diterima, server akan membalas dengan status 200 OK:
```json
{
  "success": true,
  "type": "PLAY_MONOLOGUE",
  "signal": "T001_id"
}
```

## Referensi Kode Integrasi (Python / ESP32)

### Contoh menggunakan Python (`requests`):
```python
import requests

def update_ui_state(state):
    try:
        requests.get(f"http://127.0.0.1:8080/api/trigger?event={state}", timeout=1)
    except:
        pass # Abaikan error agar tidak memblokir robot

def trigger_monologue(signal):
    try:
        requests.get(f"http://127.0.0.1:8080/api/trigger?event=PLAY_MONOLOGUE&signal={signal}", timeout=1)
    except:
        pass
```

### Contoh menggunakan ESP32 (Arduino C++):
```cpp
#include <HTTPClient.h>

void triggerUI(String event, String signal = "") {
  HTTPClient http;
  String url = "http://192.168.1.X:8080/api/trigger?event=" + event;
  if (signal != "") {
    url += "&signal=" + signal;
  }
  
  http.begin(url);
  int httpCode = http.GET();
  http.end();
}
```
