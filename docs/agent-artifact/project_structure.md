# Project Structure & Responsibilities

This document maps the key directories and files in the `xiaozhi-esp32` project to their specific technical responsibilities.

## 核心 Logical (Core Logic)
| Path | Responsibility |
| :--- | :--- |
| `main/main.cc` | Entry point. Initializes NVS and starts the `Application` singleton. |
| `main/application.cc` | **Brain of the device**. Manages the main event loop, state transitions (Idle, Listening, Speaking), and ties all services together. |
| `main/device_state_machine.cc` | Manages the internal states of the chatbot. |
| `main/mcp_server.cc` | Implements the Model Context Protocol. Handles tool registration and execution from the LLM. |

## 音频 (Audio System)
| Path | Responsibility |
| :--- | :--- |
| `main/audio/audio_service.cc` | High-level audio management. Integrates wake word detection, audio encoding/decoding, and VAD. |
| `main/audio/audio_codec.h` | Abstract interface for audio hardware. |
| `main/audio/codecs/` | Concrete implementations for various audio chips (ES8311, ES8388, etc.). |
| `main/audio/demuxer/` | Handles parsing of incoming audio streams (e.g., OGG/Opus). |

## 显示 (Display System)
| Path | Responsibility |
| :--- | :--- |
| `main/display/display.h` | Abstract interface for all display types. |
| `main/display/oled_display.cc` | Implementation for small OLED screens (often SSD1306/SH1106). |
| `main/display/lcd_display.cc` | Implementation for larger LCD/TFT screens using LVGL. |
| `main/display/lvgl_display/` | UI components, fonts, and emoji rendering logic using the LVGL library. |
| `main/display/emote_display.cc` | Specific logic for displaying "emotions" or AI expressions. |

## 网络与通信 (Network & Protocols)
| Path | Responsibility |
| :--- | :--- |
| `main/protocols/protocol.h` | Base class for communication with the AI server. |
| `main/protocols/websocket_protocol.cc` | Interaction via WebSockets. |
| `main/protocols/mqtt_protocol.cc` | Interaction via MQTT + UDP (alternative for low latency). |
| `main/ota.cc` | Handles firmware updates and activation processes. |

## 硬件抽象 (Hardware Abstraction - Boards)
| Path | Responsibility |
| :--- | :--- |
| `main/board.h` | Singleton interface to access board-specific hardware (Display, Speaker, Camera). |
| `main/boards/common/` | Shared board logic (WiFi, Battery monitoring, Buttons). |
| `main/boards/[board-name]/` | Specific hardware configuration and initialization for nearly 100 different boards. |

## 灯光 (LED Control)
| Path | Responsibility |
| :--- | :--- |
| `main/led/led.h` | Interface for status LEDs. |
| `main/led/circular_strip.cc` | Control for Addressable RGB LED rings (e.g., WS2812). |
| `main/led/gpio_led.cc` | Simple PWM/GPIO control for single-color LEDs. |
