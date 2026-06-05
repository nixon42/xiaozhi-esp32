# Walkthrough: Deskbot Mimo Custom Driver & Animations

I have successfully ported the custom SSD1306 SPI driver and integrated the high-resolution bitmap animations for the Deskbot Mimo board, bypassing the standard `esp_lcd` and LVGL layers for better performance and compatibility.

## Key Accomplishments

### 1. Custom Driver Integration
- Ported [ssd1306.h](file:///home/nixon/Project/xiaozhi-esp32/main/boards/deskbot-mimo/ssd1306.h) and [ssd1306.c](file:///home/nixon/Project/xiaozhi-esp32/main/boards/deskbot-mimo/ssd1306.c) from the `test_mic` project.
- Updated [deskbot_mimo_board.cc](file:///home/nixon/Project/xiaozhi-esp32/main/boards/deskbot-mimo/deskbot_mimo_board.cc) to initialize the display using this manual SPI driver instead of the `esp_lcd` variant.

### 2. Simplified Display Logic
- Rewrote [MimoEmojiDisplay](file:///home/nixon/Project/xiaozhi-esp32/main/boards/deskbot-mimo/deskbot_mimo_display.cc) to inherit directly from [Display](file:///home/nixon/Project/xiaozhi-esp32/main/display/display.h#28-62), dropping the LVGL status bar and layout overhead.
- Implemented direct frame buffer updates at 15 FPS using `esp_timer` and [ssd1306_refresh](file:///home/nixon/Project/test_mic/main/ssd1306.c#116-125).

## Display Status Events

As requested, here is the list of status strings that `xiaozhi-esp32` normally sends to the display. You can map these in `MimoEmojiDisplay::SetStatus` to trigger custom animations:

| Status Code | Description | Suggested Animation |
| :--- | :--- | :--- |
| `Initializing` | Device is booting up | Logo or Intro animation |
| `WiFi Config` | Entered WiFi setup (SoftAP) | QR Code or networking pulse |
| `Connecting...` | Connecting to WiFi/Server | Progress dots |
| `Listening...` | Hearing wake word / user input | Attentive eyes / Sound wave |
| `Activating...` | Server is processing/thinking | Thinking circles / `buxue` |
| `Speaking...` | Voice response is playing | Mouth moving / Happy eyes |
| `Upgrading...` | OTA firmware update | Download arrow / Percent bar |
| `Fatal Error` | System crash | X eyes / Scared look |

## Emotion Mapping

The standard [SetEmotion](file:///home/nixon/Project/xiaozhi-esp32/main/display/emote_display.cc#137-144) still supports the standard set of keys, which you can continue to expand:
- `happy`
- `sad`
- `angry`
- `scare`
- `neutral` (Default)
- `buxue`
- `sleepy` (Horizontal bars fallback)

## Verification Results

- Verified the build for `deskbot-mimo`.
- Verified that the custom driver initialization sequence matches the one known to work with your hardware.
- Confirmed animations run in a dedicated timer thread at 66ms intervals.
