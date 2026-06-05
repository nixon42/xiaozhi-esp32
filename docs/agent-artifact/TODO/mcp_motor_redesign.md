# Redesign MCP Motor Control untuk Deskbot Mimo (2-Wheel Differential Drive)

## Masalah Implementasi Sekarang

1. **Tidak ada konsep direksi** — Tool hanya punya `speed` (integer), LLM harus menebak bahwa speed negatif = mundur. Ini membingungkan dan error-prone.
2. **Durasi terlalu pendek** — LLM sering memberikan `duration_ms` yang sangat kecil (misal 100ms), menghasilkan roda cuma berputar setengah kali. Tidak ada minimum enforcement.
3. **Terlalu banyak tool** — 6 tool terpisah (`move_robot`, `move_forward`, `move_backward`, `turn_left`, `turn_right`, `stop_robot`) membuat prompt LLM membengkak dan membingungkan.
4. **Tidak ada action queue** — Perintah langsung dieksekusi tanpa antrian. Jika LLM mengirim 3 perintah berturutan (maju → belok → maju), perintah sebelumnya langsung ditimpa.
5. **Tidak ada status feedback** — LLM tidak tahu apakah robot sedang bergerak atau idle, sehingga bisa mengirim perintah yang saling bertabrakan.

---

## Desain Baru yang Diusulkan

### Prinsip Desain

1. **Satu tool utama, satu stop, satu status** — Minimalkan jumlah tool agar prompt LLM ringan.
2. **Gunakan `action` string** — LLM memilih aksi (`forward`, `backward`, `turn_left`, `turn_right`) sebagai string, bukan angka.
3. **Gunakan `steps` bukan `duration_ms`** — 1 step = gerakan yang meaningful dan konsisten.Step dihitung berdasarkan jumlah "pulsa" PWM yang cukup untuk menghasilkan putaran roda yang terlihat.
4. **Action Queue (FreeRTOS)** — Perintah masuk ke antrian, dieksekusi berurutan oleh background task. LLM bisa mengirim beberapa perintah sekaligus tanpa race condition.
5. **Minimum enforcement** — Step minimum = 1, speed range terbatas ke preset yang aman.

### Tool yang Diusulkan

#### 1. `self.mimo.action` — Tool Gerakan Utama

Satu tool untuk semua gerakan robot.

| Parameter | Tipe | Default | Range | Deskripsi |
|-----------|------|---------|-------|-----------|
| `action` | string | `"forward"` | `forward`, `backward`, `turn_left`, `turn_right` | Arah gerakan |
| `steps` | integer | 3 | 1-20 | Jumlah langkah. 1 step ≈ 200-500ms gerakan fisik |
| `speed` | integer | 2 | 1-3 | Kecepatan: 1=pelan, 2=normal, 3=cepat |

**Mapping speed ke PWM duty (internal):**
- speed 1 → duty 40%  (pelan, presisi tinggi)
- speed 2 → duty 65%  (normal)
- speed 3 → duty 90%  (cepat, kurang presisi)

**Mapping action ke motor (internal):**

| Action | Motor Kiri | Motor Kanan |
|--------|-----------|-------------|
| `forward` | +duty | +duty |
| `backward` | -duty | -duty |
| `turn_left` | -duty | +duty |
| `turn_right` | +duty | -duty |

**Logika 1 Step (internal):**
```
untuk setiap step:
    1. Set motor sesuai action dan speed
    2. Delay STEP_DURATION_MS (misal 300ms)
    3. Stop motor
    4. Delay STEP_GAP_MS (misal 50ms) → beri jeda antar step
```

> [!IMPORTANT]
> `STEP_DURATION_MS` dan `STEP_GAP_MS` harus di-tune pada hardware asli untuk mendapatkan gerakan yang konsisten dan terlihat natural.

#### 2. `self.mimo.stop` — Emergency Stop

Menghentikan semua gerakan dan mengosongkan antrian.

| Parameter | Tipe | Default | Deskripsi |
|-----------|------|---------|-----------|
| _(none)_ | — | — | Tidak ada parameter |

**Logika:**
1. Kill action task (jika ada)
2. Reset antrian
3. Stop kedua motor secara langsung
4. Restart action task

#### 3. `self.mimo.get_status` — Status Robot

Mengembalikan status robot saat ini.

| Parameter | Tipe | Default | Deskripsi |
|-----------|------|---------|-----------|
| _(none)_ | — | — | Tidak ada parameter |

**Return:** `"moving"` atau `"idle"`

---

### Arsitektur Internal

```
┌──────────────┐     ┌──────────────┐     ┌──────────────────┐
│  MCP Tool    │────>│ Action Queue │────>│ Action Task      │
│  Callback    │     │ (FreeRTOS)   │     │ (Background)     │
└──────────────┘     └──────────────┘     │                  │
                                          │  for each step:  │
                                          │   - Set motors   │
                                          │   - Delay step   │
                                          │   - Stop motors  │
                                          │   - Delay gap    │
                                          └──────┬───────────┘
                                                 │
                                          ┌──────▼───────────┐
                                          │  TB6612FNG       │
                                          │  Driver (PWM)    │
                                          └──────────────────┘
```

**Komponen:**
1. **ActionParams struct** — Menyimpan action_type, steps, speed
2. **Action Queue** — `xQueueCreate(10, sizeof(ActionParams))` — buffer 10 perintah
3. **Action Task** — FreeRTOS task yang consume dari queue dan eksekusi step-by-step
4. **is_action_in_progress_** — flag boolean untuk status reporting

---

### Konstanta yang Perlu Di-tune

```cpp
// Durasi satu step (ms) — di-tune berdasarkan hardware
#define STEP_DURATION_MS  300

// Jeda antar step (ms) — mencegah motor overshoot
#define STEP_GAP_MS       50

// PWM duty per speed level (0-100)
#define SPEED_SLOW   40
#define SPEED_NORMAL 65
#define SPEED_FAST   90
```

> [!TIP]
> Mulai dengan nilai di atas, lalu tune `STEP_DURATION_MS` agar 1 step menghasilkan gerakan ~10-15cm pada lantai datar. Ini membuat perintah "maju 3 langkah" mudah diprediksi oleh LLM.

---

### Contoh Penggunaan oleh LLM

**User:** "Robot, maju ke depan 5 langkah"
```json
{"action": "forward", "steps": 5, "speed": 2}
```

**User:** "Belok kiri pelan"
```json
{"action": "turn_left", "steps": 2, "speed": 1}
```

**User:** "Maju, belok kanan, lalu maju lagi"
```
Call 1: {"action": "forward", "steps": 3, "speed": 2}
Call 2: {"action": "turn_right", "steps": 2, "speed": 2}
Call 3: {"action": "forward", "steps": 3, "speed": 2}
→ Semua masuk antrian, dieksekusi berurutan tanpa race condition
```

**User:** "Stop!"
```
Call: self.mimo.stop (no params)
→ Motor langsung berhenti, antrian dikosongkan
```

---

### File yang Akan Diubah

| File | Aksi | Deskripsi |
|------|------|-----------|
| `mcp_mimo.h` | REWRITE | Tambah ActionQueue, ActionTask, ActionParams, status flag |
| `mcp_mimo.cc` | REWRITE | Implementasi ulang: 3 tool, queue, task, step-based logic |
| `tb6612fng.h` | KEEP | Driver low-level tidak berubah |
| `tb6612fng.cc` | KEEP | Driver low-level tidak berubah |
| `deskbot_mimo_board.cc` | MINOR | Sesuaikan inisialisasi jika API berubah |
| `config.h` | KEEP | Pin config tidak berubah |

---

### Checklist Implementasi

- [ ] Definisikan `ActionParams` struct dan enum `ActionType`
- [ ] Buat `ActionTask` static function dengan loop queue receive
- [ ] Implementasi step-based movement di action task
- [ ] Register 3 tool: `self.mimo.action`, `self.mimo.stop`, `self.mimo.get_status`
- [ ] Hapus tool lama (6 tool → 3 tool)
- [ ] Hapus `esp_timer` (diganti dengan FreeRTOS task + delay)
- [ ] Tune `STEP_DURATION_MS` pada hardware asli
- [ ] Test: kirim beberapa perintah berturutan, verifikasi antrian bekerja
- [ ] Test: `stop` di tengah gerakan, verifikasi motor langsung berhenti
