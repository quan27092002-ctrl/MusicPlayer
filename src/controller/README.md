# Controller Layer - Detailed Documentation

## Tổng quan

Thư mục `src/controller` chứa logic điều khiển của ứng dụng Music Player:
- Nhận lệnh từ **View** (user interactions)
- Điều khiển **AudioPlayer** để phát nhạc
- Giao tiếp với **S32K Board** qua serial
- Cập nhật trạng thái vào **Model** (PlayerState)

---

## Cấu trúc thư mục

```
src/controller/
├── IAppController.h            # Aggregate interface
├── AppController.h/cpp         # Main facade
├── IAudioPlayer.h              # Audio interface
├── AudioPlayer.h/cpp           # Audio implementation
├── ISerialManager.h            # Serial interface
├── SerialManager.h/cpp         # Serial implementation
├── appcontroller/
│   ├── PlaybackController.h/cpp    # Play/Pause/Stop/Next/Previous
│   ├── VolumeController.h/cpp      # Volume/Mute control
│   ├── PlaylistManager.h/cpp       # Playlist operations
│   ├── HistoryManager.h/cpp        # Navigation history
│   ├── BoardCommunicator.h/cpp     # S32K communication
│   └── interfaces/
│       ├── IAppLifecycle.h
│       ├── IPlaybackController.h
│       ├── IVolumeController.h
│       ├── IPlaylistManager.h
│       ├── IHistoryManager.h
│       └── IBoardCommunicator.h
├── audioplayer/
│   ├── AudioLifecycleImpl.h/cpp
│   ├── AudioLoaderImpl.h/cpp
│   ├── AudioPlaybackImpl.h/cpp
│   ├── AudioVolumeImpl.h/cpp
│   └── interfaces/
└── serialmanager/
    ├── SerialConnectionImpl.h/cpp
    ├── SerialIOImpl.h/cpp
    └── interfaces/
```

---

## Features & Use Cases

---

### UC-C01: Khởi tạo ứng dụng (Initialize)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | Application |
| **Interface** | IAppLifecycle |
| **Method** | `initialize()` |
| **Postcondition** | AppState = RUNNING |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant Main as main()
    participant AC as AppController
    participant AP as AudioPlayer
    participant SM as SerialManager
    participant PS as PlayerState

    Main->>AC: initialize()
    AC->>AC: mAppState = INITIALIZING
    AC->>AP: initialize()
    AP-->>AC: true
    AC->>SM: initialize()
    SM-->>AC: true
    AC->>PS: reset()
    AC->>AC: mAppState = RUNNING
    AC->>AC: notifyStateChange(RUNNING)
    AC-->>Main: return true
```

---

### UC-C02: Phát nhạc (Play)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (via View) |
| **Interface** | IPlaybackController |
| **Method** | `play()` |
| **Precondition** | Playlist không rỗng |
| **Postcondition** | PlaybackStatus = PLAYING |

**Luồng chính:**
1. View gọi `controller->play()`
2. PlaybackController kiểm tra có track đang load không
3. Nếu chưa có, load track đầu tiên trong playlist
4. Gọi AudioPlayer.play()
5. Cập nhật PlayerState.setPlaybackStatus(PLAYING)

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant PM as PlaylistManager
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: play()
    AC->>PC: play()
    
    alt Không có track đang load
        PC->>PM: getPlaylistRef()
        PM-->>PC: playlist (not empty)
        PC->>PC: mCurrentTrackIterator = playlist.begin()
        PC->>AP: load(trackPath)
        AP-->>PC: true
    end
    
    PC->>PS: isMuted()
    alt Đang muted
        PC->>PS: setMuted(false)
        PC->>AP: setVolume(currentVolume)
    end
    
    PC->>AP: play()
    Note over AP: Start audio playback
    AP->>PS: setPlaybackStatus(PLAYING)
```

---

### UC-C03: Tạm dừng (Pause)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (via View) |
| **Interface** | IPlaybackController |
| **Method** | `pause()` |
| **Precondition** | PlaybackStatus = PLAYING |
| **Postcondition** | PlaybackStatus = PAUSED |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: pause()
    AC->>PC: pause()
    PC->>AP: pause()
    AP->>PS: setPlaybackStatus(PAUSED)
    Note over AP: Audio paused at current position
```

---

### UC-C04: Dừng phát (Stop)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (via View) |
| **Interface** | IPlaybackController |
| **Method** | `stop()` |
| **Postcondition** | PlaybackStatus = STOPPED, Position = 0 |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: stop()
    AC->>PC: stop()
    PC->>AP: stop()
    AP->>PS: setPlaybackStatus(STOPPED)
    AP->>PS: setCurrentPosition(0)
    Note over AP: Audio stopped, position reset
```

---

### UC-C05: Chuyển bài tiếp theo (Next)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (via View) hoặc S32K Board |
| **Interface** | IPlaybackController |
| **Method** | `next()` |
| **Precondition** | Playlist không rỗng |
| **Postcondition** | Track tiếp theo được phát |

**Luồng chính:**
1. Lưu track hiện tại vào history
2. Tăng track iterator (wrap around nếu cuối playlist)
3. Load track mới
4. Bắt đầu phát

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant HM as HistoryManager
    participant PM as PlaylistManager
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: next()
    AC->>PC: next()
    
    PC->>PM: getPlaylistRef()
    PM-->>PC: playlist
    
    alt Có track hiện tại
        PC->>HM: pushHistory(currentTrack)
    end
    
    PC->>PC: mCurrentTrackIterator++
    alt Đến cuối playlist
        PC->>PC: mCurrentTrackIterator = playlist.begin()
    end
    
    PC->>AP: load(newTrackPath)
    AP-->>PC: true
    PC->>PS: setCurrentTrackIndex(newIndex)
    PC->>PC: play()
    Note over AP: New track playing
```

---

### UC-C06: Chuyển bài trước đó (Previous)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (via View) hoặc S32K Board |
| **Interface** | IPlaybackController |
| **Method** | `previous()` |
| **Behavior** | Kiểm tra history trước, nếu không có thì lùi playlist |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant HM as HistoryManager
    participant PM as PlaylistManager
    participant AP as AudioPlayer

    V->>AC: previous()
    AC->>PC: previous()
    
    PC->>HM: popHistory()
    alt History có track
        HM-->>PC: historyTrack
        PC->>PC: Find track in playlist
        PC->>AP: load(historyTrackPath)
    else History rỗng
        HM-->>PC: nullptr
        PC->>PC: mCurrentTrackIterator--
        alt Ở đầu playlist
            PC->>PC: mCurrentTrackIterator = playlist.end() - 1
        end
        PC->>AP: load(prevTrackPath)
    end
    
    PC->>PC: play()
```

---

### UC-C07: Phát bài theo index (Play Track)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (click vào track trong playlist) |
| **Interface** | IPlaybackController |
| **Method** | `playTrack(int index)` |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant HM as HistoryManager
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: playTrack(5)
    AC->>PC: playTrack(5)
    
    PC->>PC: Find iterator for index 5
    
    alt Track hiện tại khác track mới
        PC->>HM: pushHistory(currentTrack)
    end
    
    PC->>PC: mCurrentTrackIterator = iterator[5]
    PC->>AP: load(trackPath)
    AP-->>PC: true
    PC->>PS: setCurrentTrackIndex(5)
    PC->>PC: play()
```

---

### UC-C08: Seek trong track

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User (kéo seek bar) |
| **Interface** | IPlaybackController |
| **Method** | `seek(uint32_t positionMs)` |
| **Unit** | Milliseconds |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant PC as PlaybackController
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: seek(90000)
    Note over V: User dragged to 1:30
    AC->>PC: seek(90000)
    PC->>AP: seek(90000)
    
    Note over AP: Seek audio decoder<br/>to 90 seconds
    AP->>PS: setCurrentPosition(90000)
```

---

### UC-C09: Điều chỉnh Volume

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User hoặc S32K Board |
| **Interface** | IVolumeController |
| **Method** | `setVolume(int volume)` |
| **Range** | 0 - 100 |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant VC as VolumeController
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: setVolume(80)
    AC->>VC: setVolume(80)
    VC->>PS: setVolume(80)
    VC->>AP: setVolume(80)
    Note over AP: Audio volume updated
```

---

### UC-C10: Toggle Mute

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Interface** | IVolumeController |
| **Method** | `toggleMute()` |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant V as View
    participant AC as AppController
    participant VC as VolumeController
    participant AP as AudioPlayer
    participant PS as PlayerState

    V->>AC: toggleMute()
    AC->>VC: toggleMute()
    VC->>PS: toggleMute()
    PS-->>VC: newMuteState = true
    
    alt Now muted
        VC->>AP: setVolume(0)
    else Now unmuted
        VC->>PS: getVolume()
        PS-->>VC: 80
        VC->>AP: setVolume(80)
    end
```

---

### UC-C11: Load thư mục nhạc

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | Application (startup) |
| **Interface** | IPlaylistManager |
| **Method** | `loadDirectory(string path)` |
| **Return** | Số lượng tracks được load |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant Main as main()
    participant AC as AppController
    participant PM as PlaylistManager
    participant MF as MediaFile

    Main->>AC: loadDirectory("/music")
    AC->>PM: loadDirectory("/music")
    
    loop Mỗi file .mp3/.wav/.flac
        PM->>PM: Parse metadata (artist, album, duration)
        PM->>PM: Extract cover art
        PM->>MF: new MediaFile(name, path, duration, artist, album, coverArt)
        PM->>PM: playlist.push_back(mediaFile)
    end
    
    PM-->>AC: return 15 (tracks loaded)
    AC-->>Main: 15
```

---

### UC-C12: Kết nối S32K Board

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | Application hoặc User |
| **Interface** | IBoardCommunicator |
| **Method** | `connectToBoard(portName, baudRate)` |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant UI as View
    participant AC as AppController
    participant BC as BoardCommunicator
    participant SM as SerialManager
    participant SC as SerialConnection

    UI->>AC: connectToBoard("/dev/ttyUSB0", 115200)
    AC->>BC: connect("/dev/ttyUSB0", 115200)
    BC->>SM: connect("/dev/ttyUSB0", 115200)
    SM->>SC: connect("/dev/ttyUSB0", 115200)
    
    Note over SC: Open serial port<br/>Configure baud rate
    SC->>SC: mState = CONNECTED
    SC-->>SM: true
    SM-->>BC: true
    
    BC->>SM: setDataCallback(onDataReceived)
    BC->>SM: startReading()
    Note over SM: Start read thread
    
    BC-->>AC: true
    AC-->>UI: Connected
```

---

### UC-C13: Nhận lệnh từ S32K Board

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | S32K Board (hardware) |
| **Trigger** | User xoay núm volume, nhấn nút |
| **Commands** | "PLAY", "PAUSE", "NEXT", "PREV", "VOL:80" |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant HW as S32K Board
    participant SM as SerialManager
    participant AC as AppController
    participant PC as PlaybackController
    participant VC as VolumeController
    participant PS as PlayerState

    HW->>SM: Send "VOL:80\n"
    SM->>SM: Read from serial
    SM->>AC: onSerialDataReceived("VOL:80")
    AC->>AC: processCommand("VOL:80")
    
    alt Command = "PLAY"
        AC->>PC: play()
    else Command = "PAUSE"
        AC->>PC: pause()
    else Command = "NEXT"
        AC->>PC: next()
    else Command = "PREV"
        AC->>PC: previous()
    else Command = "VOL:xx"
        AC->>AC: Parse volume value
        AC->>VC: setVolume(80)
        VC->>PS: setVolume(80)
    end
```

---

### UC-C14: Xử lý Track kết thúc

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | AudioPlayer (internal) |
| **Trigger** | Track phát xong |
| **Behavior** | Phụ thuộc RepeatMode |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant AP as AudioPlayer
    participant AC as AppController
    participant PS as PlayerState
    participant PC as PlaybackController

    AP->>AC: onAudioStateChanged(STOPPED, endPosition)
    AC->>PS: getRepeatMode()
    
    alt RepeatMode = ONE
        PS-->>AC: ONE
        AC->>PC: play()
        Note over PC: Replay current track
    else RepeatMode = ALL
        PS-->>AC: ALL
        AC->>PC: next()
        Note over PC: Play next track<br/>(wrap to first if last)
    else RepeatMode = NONE
        PS-->>AC: NONE
        AC->>PS: getCurrentTrackIndex()
        alt Not last track
            AC->>PC: next()
        else Last track
            AC->>PS: setPlaybackStatus(STOPPED)
            Note over AC: Playlist finished
        end
    end
```

---

## Class Diagram

```mermaid
classDiagram
    class IAppController {
        <<interface>>
    }
    class IAppLifecycle {
        <<interface>>
        +initialize() bool
        +shutdown() void
        +getState() AppState
    }
    class IPlaybackController {
        <<interface>>
        +play() void
        +pause() void
        +stop() void
        +next() void
        +previous() void
        +playTrack(index) void
        +seek(positionMs) void
    }
    class IVolumeController {
        <<interface>>
        +setVolume(volume) void
        +getVolume() int
        +toggleMute() void
    }
    class IPlaylistManager {
        <<interface>>
        +loadDirectory(path) size_t
        +getPlaylistSize() size_t
        +getTrackName(index) string
    }
    class IBoardCommunicator {
        <<interface>>
        +connectToBoard(port, baud) bool
        +disconnectFromBoard() void
    }
    
    class AppController {
        -mAudioPlayer: IAudioPlayer*
        -mSerialManager: ISerialManager*
        -mPlayerState: IPlayerState*
        -mPlaybackController: PlaybackControllerImpl
        -mVolumeController: VolumeControllerImpl
        -mPlaylistManager: PlaylistManagerImpl
        -mHistoryManager: HistoryManagerImpl
        -mBoardCommunicator: BoardCommunicatorImpl
    }
    
    IAppController <|-- IAppLifecycle
    IAppController <|-- IPlaybackController
    IAppController <|-- IVolumeController
    IAppController <|-- IPlaylistManager
    IAppController <|-- IBoardCommunicator
    IAppController <|.. AppController
```

---

## State Diagram - AppState

```mermaid
stateDiagram-v2
    [*] --> UNINITIALIZED
    UNINITIALIZED --> INITIALIZING : initialize()
    INITIALIZING --> RUNNING : success
    INITIALIZING --> ERROR : failure
    RUNNING --> SHUTTINGDOWN : shutdown()
    SHUTTINGDOWN --> [*]
    ERROR --> SHUTTINGDOWN : shutdown()
```

---

## Command Processing

| Command | Source | Action |
|---------|--------|--------|
| `PLAY` | S32K Board | `playbackController->play()` |
| `PAUSE` | S32K Board | `playbackController->pause()` |
| `STOP` | S32K Board | `playbackController->stop()` |
| `NEXT` | S32K Board | `playbackController->next()` |
| `PREV` | S32K Board | `playbackController->previous()` |
| `VOL:xx` | S32K Board | `volumeController->setVolume(xx)` |

---

## Dependencies Injection

```cpp
// Create dependencies
auto audioPlayer = std::make_shared<AudioPlayer>();
auto serialManager = std::make_shared<SerialManager>();
auto playerState = std::make_shared<PlayerState>();

// Inject into AppController
auto controller = std::make_shared<AppController>(
    audioPlayer,
    serialManager,
    playerState
);

// Initialize
controller->initialize();

// Load music
controller->loadDirectory("/path/to/music");

// Connect board (optional)
controller->connectToBoard("/dev/ttyUSB0", 115200);
```
