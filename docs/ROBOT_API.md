# DAPUR.AI SOFIA - Robot Communication API

Dokumen ini menjelaskan bagaimana Firmware / Core Robot dapat mengirimkan sinyal perubahan *state* dan monolog ke UI (Display Engine) hanya menggunakan **HTTP GET Requests** sederhana.

Tidak perlu menggunakan WebSocket atau MQTT. UI akan otomatis tersinkronisasi secara *real-time* begitu HTTP GET request diterima oleh *endpoint* trigger.

## 1. Konfigurasi Endpoint

Display Engine (Web Server) mengekspos endpoint API di port **8080** (default pada Docker).

**Base URL:**
`http://<IP_DISPLAY_ENGINE>:8080/api/trigger`

---

## 2. Mengubah Mode Percakapan (Conversation State)

Setiap kali Robot mulai mendengarkan (*Listening*), berpikir (*Thinking*), atau berbicara (*Speaking*), kirimkan HTTP GET dengan parameter `event`.

### Endpoint
`GET /api/trigger?event=<STATE>`

### Parameter yang Valid:
- `ATTRACT` : Mengembalikan UI ke mode Slideshow Monolog.
- `LISTENING` : Mengubah UI ke animasi gelombang suara biru (Mic / Audio In).
- `THINKING` : Mengubah UI ke animasi *neural network* ungu (Proses LLM).
- `SPEAKING` : Mengubah UI ke animasi lingkaran emas (Audio Out).

### Contoh Request (cURL)
```bash
# Robot mulai mendengarkan suara user
curl "http://localhost:8080/api/trigger?event=LISTENING"

# Robot sedang memproses teks dari LLM
curl "http://localhost:8080/api/trigger?event=THINKING"

# Robot sedang mengeluarkan suara balasan
curl "http://localhost:8080/api/trigger?event=SPEAKING"
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
