# 🎵 S32K Media Player

A **C++17 desktop music player** with an embedded hardware control interface via **NXP S32K144 microcontroller**. Built with clean **MVC architecture**, **SOLID principles**, and full **thread-safety** using atomic operations.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?logo=cplusplus)
![SDL2](https://img.shields.io/badge/SDL2-Audio%20%7C%20Render-green?logo=sdl)
![ImGui](https://img.shields.io/badge/Dear%20ImGui-UI-orange)
![GTest](https://img.shields.io/badge/Google%20Test-Unit%20Tests-red?logo=google)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey?logo=linux)

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| 🎶 **Audio Playback** | Play, pause, stop, next, previous with SDL2_mixer |
| 📁 **File Management** | Load music from directories and USB storage devices |
| 🔀 **Queue Control** | Shuffle, repeat (none/one/all), history navigation |
| 🎨 **Cover Art** | Extract and display album artwork via TagLib |
| 📋 **Smart Playlists** | Auto-generated playlists by artist and album |
| 🔊 **Volume Control** | Software volume + hardware potentiometer via ADC |
| 🔌 **Hardware Integration** | NXP S32K144 board control via UART serial port |
| 🖥️ **Modern GUI** | Dear ImGui with SDL2 renderer backend |
| 🧪 **Unit Testing** | 23 test suites with Google Test & Google Mock |

---

## 🏗️ Architecture

The application follows a strict **Model-View-Controller (MVC)** pattern with **SOLID principles** applied throughout:

```
┌─────────────────────────────────────────────────────────┐
│  VIEW LAYER (ImGui + SDL2)                              │
│  ┌──────────┐ ┌────────────┐ ┌──────────────┐           │
│  │PlayerBar │ │MainContent │ │RightSidebar  │           │
│  └──────────┘ └────────────┘ └──────────────┘           │
├─────────────────────────────────────────────────────────┤
│  CONTROLLER LAYER                                       │
│  ┌──────────────┐ ┌─────────────┐ ┌──────────────────┐  │
│  │AppController │ │AudioPlayer  │ │SerialManager     │  │
│  │  ├Playback   │ │  ├Lifecycle │ │  ├Connection     │  │
│  │  ├Playlist   │ │  ├Loader    │ │  └IO             │  │
│  │  ├Volume     │ │  ├Playback  │ ├──────────────────┤  │
│  │  ├History    │ │  └Volume    │ │StorageManager    │  │
│  │  └Board      │ └─────────────┘ └──────────────────┘  │
├─────────────────────────────────────────────────────────┤
│  MODEL LAYER (Thread-safe with std::atomic)             │
│  ┌──────────────────┐ ┌─────────────────┐               │
│  │ PlayerState      │ │ MediaFile       │               │
│  │  ├PlaybackState  │ │  ├MediaFileInfo │               │
│  │  ├VolumeState    │ │  ├MediaMetadata │               │
│  │  ├TrackPosition  │ │  └CoverArt      │               │
│  │  └PlaylistNav    │ └─────────────────┘               │
│  └──────────────────┘                                   │
└─────────────────────────────────────────────────────────┘
```

---

## 📂 Project Structure

```
.
├── src/
│   ├── main.cpp                  # Application entry point
│   ├── model/                    # Data models (thread-safe)
│   │   ├── mediafile/            # MediaFile components (SRP)
│   │   └── playerstate/          # PlayerState components (SRP)
│   ├── controller/               # Business logic
│   │   ├── appcontroller/        # App controller sub-components
│   │   ├── audioplayer/          # Audio player sub-components
│   │   └── serialmanager/        # Serial communication
│   ├── view/                     # UI layer
│   │   ├── imguiview/            # ImGui widget components
│   │   └── imgui/                # Dear ImGui library
│   └── utils/                    # Utilities (Buffer, Logger, ThreadSafeQueue)
├── test/                         # 23 unit test suites (GTest + GMock)
│   └── mocks/                    # Mock classes for dependency injection
├── uml/                          # UML diagrams (PlantUML)
│   ├── high level design/        # Component, use case, sequence diagrams
│   └── lowlevel/                 # Class, sequence, use case diagrams
├── document/                     # SRS documentation (LaTeX)
└── makefile                      # Build system
```

---

## 🛠️ Tech Stack

| Component | Technology |
|-----------|-----------|
| Language | C++17 |
| Build | GNU Make (parallel build) |
| GUI | Dear ImGui + SDL2 Renderer |
| Audio | SDL2_mixer |
| Metadata | TagLib |
| Image | SDL2_image |
| Serial | POSIX termios (Linux) |
| Testing | Google Test + Google Mock |
| Coverage | LCOV + Genhtml |
| Diagrams | PlantUML |
| Documentation | LaTeX |

---

## 🚀 Getting Started

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install -y \
    build-essential \
    libsdl2-dev \
    libsdl2-mixer-dev \
    libsdl2-image-dev \
    libtag1-dev \
    libgtest-dev \
    libgmock-dev \
    lcov
```

### Build & Run

```bash
# Build the application
make app

# Run the application
make run_app

# The app loads music from ./mMusic/ directory by default
# Place your MP3 files there before running
```

### Unit Testing

```bash
# Build and run all tests
make run

# Generate HTML coverage report
make coverage-report
# Report available at: build/coverage/index.html
```

---

## 🔌 Hardware Integration (Optional)

The application supports the **NXP S32K144 evaluation board** connected via USB-UART for physical playback control:

| Board Input | Function |
|-------------|----------|
| Push buttons | Play/Pause, Next, Previous |
| Potentiometer (ADC) | Volume control |

Connect the board to a serial port (e.g., `/dev/ttyUSB0`) and use the **Connect** panel in the application's right sidebar.

---

## 📐 Design Documents

- **SRS**: See [`document/`](document/) for Software Requirements Specification (LaTeX)
- **UML High-Level**: [`uml/high level design/`](uml/high%20level%20design/) — Component, Use Case, Sequence diagrams
- **UML Low-Level**: [`uml/lowlevel/`](uml/lowlevel/) — Class diagrams, detailed Use Cases, Sequence diagrams
- **Architecture**: All diagrams available as `.svg` files for easy viewing

---

## 🧪 Test Coverage

23 unit test suites covering all layers:

| Layer | Test Suites |
|-------|-------------|
| **Model** | MediaFile, PlayerState, CoverArt, VolumeState, TrackPosition |
| **Controller** | AppController (3 suites), AudioPlayer, PlaybackController (2), PlaylistManager (2), SerialManager, SerialIO, BoardCommunicator, HistoryManager, StorageManager |
| **Utils** | Buffer, Logger, ThreadSafeQueue |

---

## 👤 Author

**Quân** — Software Engineer

---

## 📄 License

This project is developed for educational and portfolio purposes.
