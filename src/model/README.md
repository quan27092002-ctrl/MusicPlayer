# Model Layer - Detailed Documentation

## Tổng quan

Thư mục `src/model` chứa các thành phần dữ liệu và trạng thái ứng dụng. Layer này:
- Lưu trữ trạng thái player (đang phát, volume, vị trí...)
- Lưu trữ thông tin media file (tên, artist, duration...)
- Được đọc bởi **View** và ghi bởi **Controller**
- **Thread-safe** hoàn toàn với atomic operations

---

## Cấu trúc thư mục

```
src/model/
├── IMediaFile.h                # Aggregate interface
├── MediaFile.h/cpp             # Facade implementation
├── IPlayerState.h              # Aggregate interface
├── PlayerState.h/cpp           # Facade implementation
├── mediafile/
│   ├── CoverArt.h/cpp              # Cover art data
│   ├── MediaFileInfo.h/cpp         # File name, path
│   ├── MediaMetadata.h/cpp         # Artist, album, duration
│   └── interfaces/
│       ├── ICoverArt.h
│       ├── IMediaFileInfo.h
│       └── IMediaMetadata.h
└── playerstate/
    ├── PlaybackStateImpl.h/cpp     # PLAYING/PAUSED/STOPPED
    ├── VolumeStateImpl.h/cpp       # Volume (0-100), muted
    ├── TrackPositionImpl.h/cpp     # Current position (ms)
    ├── PlaylistNavigationImpl.h/cpp # Track index, repeat, shuffle
    └── interfaces/
        ├── IPlaybackState.h
        ├── IVolumeState.h
        ├── ITrackPosition.h
        └── IPlaylistNavigation.h
```

---

## Features & Use Cases

### UC-M01: Quản lý trạng thái Playback

| Thuộc tính | Mô tả |
|------------|-------|
| **Component** | PlaybackStateImpl |
| **Interface** | IPlaybackState |
| **States** | STOPPED, PLAYING, PAUSED |
| **Thread-Safety** | std::atomic\<int\> |

**Operations:**

| Method | Mô tả | Return |
|--------|-------|--------|
| `getPlaybackStatus()` | Lấy trạng thái hiện tại | PlaybackStatus |
| `setPlaybackStatus(status)` | Đặt trạng thái mới | void |
| `isPlaying()` | Kiểm tra đang phát | bool |
| `togglePlayPause()` | Chuyển đổi Play/Pause | PlaybackStatus |

**State Diagram:**
```mermaid
stateDiagram-v2
    [*] --> STOPPED
    STOPPED --> PLAYING : play()
    PLAYING --> PAUSED : pause()
    PAUSED --> PLAYING : play()
    PLAYING --> STOPPED : stop()
    PAUSED --> STOPPED : stop()
```

**Sequence Diagram - Toggle Play/Pause:**
```mermaid
sequenceDiagram
    participant C as Controller
    participant PS as PlayerState
    participant PB as PlaybackStateImpl

    C->>PS: togglePlayPause()
    PS->>PB: togglePlayPause()
    Note over PB: atomic load current status
    alt status == PLAYING
        PB->>PB: atomic store PAUSED
        PB-->>PS: return PAUSED
    else status == PAUSED or STOPPED
        PB->>PB: atomic store PLAYING
        PB-->>PS: return PLAYING
    end
    PS-->>C: new status
```

---

### UC-M02: Quản lý Volume

| Thuộc tính | Mô tả |
|------------|-------|
| **Component** | VolumeStateImpl |
| **Interface** | IVolumeState |
| **Range** | 0 - 100 |
| **Thread-Safety** | std::atomic\<int\>, std::atomic\<bool\> |

**Operations:**

| Method | Mô tả | Return |
|--------|-------|--------|
| `getVolume()` | Lấy volume hiện tại (0-100) | int |
| `setVolume(volume)` | Đặt volume (auto clamp 0-100) | void |
| `isMuted()` | Kiểm tra mute | bool |
| `setMuted(muted)` | Đặt trạng thái mute | void |
| `toggleMute()` | Chuyển đổi mute | bool |

**Sequence Diagram - Set Volume:**
```mermaid
sequenceDiagram
    participant C as Controller
    participant PS as PlayerState
    participant VS as VolumeStateImpl

    C->>PS: setVolume(80)
    PS->>VS: setVolume(80)
    Note over VS: clamp(80, 0, 100) = 80
    VS->>VS: mVolume.store(80)
    VS-->>PS: void
    PS-->>C: void
```

**Sequence Diagram - Toggle Mute:**
```mermaid
sequenceDiagram
    participant V as View
    participant S as PlayerState
    participant VS as VolumeStateImpl

    V->>S: toggleMute()
    S->>VS: toggleMute()
    Note over VS: current = mMuted.load()
    VS->>VS: mMuted.store(!current)
    VS-->>S: return !current
    S-->>V: true (now muted)
```

---

### UC-M03: Quản lý vị trí Track

| Thuộc tính | Mô tả |
|------------|-------|
| **Component** | TrackPositionImpl |
| **Interface** | ITrackPosition |
| **Unit** | Milliseconds |
| **Thread-Safety** | std::atomic\<uint32_t\> |

**Operations:**

| Method | Mô tả | Return |
|--------|-------|--------|
| `getCurrentPosition()` | Lấy vị trí hiện tại (ms) | uint32_t |
| `setCurrentPosition(pos)` | Đặt vị trí mới | void |

**Sequence Diagram - Update Position (from AudioPlayer):**
```mermaid
sequenceDiagram
    participant AP as AudioPlayer
    participant C as Controller
    participant PS as PlayerState
    participant TP as TrackPositionImpl

    loop Mỗi 100ms
        AP->>C: onPositionChanged(45000)
        C->>PS: setCurrentPosition(45000)
        PS->>TP: setCurrentPosition(45000)
        TP->>TP: mPosition.store(45000)
    end
    
    Note over V: View render loop
    participant V as View
    V->>PS: getCurrentPosition()
    PS->>TP: getCurrentPosition()
    TP-->>PS: 45000
    PS-->>V: 45000
```

---

### UC-M04: Điều hướng Playlist

| Thuộc tính | Mô tả |
|------------|-------|
| **Component** | PlaylistNavigationImpl |
| **Interface** | IPlaylistNavigation |
| **Thread-Safety** | std::atomic cho tất cả fields |

**Operations:**

| Method | Mô tả | Return |
|--------|-------|--------|
| `getCurrentTrackIndex()` | Index track đang phát (-1 nếu không có) | int |
| `setCurrentTrackIndex(index)` | Đặt track index | void |
| `getRepeatMode()` | Lấy chế độ lặp | RepeatMode |
| `setRepeatMode(mode)` | Đặt chế độ lặp | void |
| `cycleRepeatMode()` | Xoay vòng: NONE→ONE→ALL→NONE | RepeatMode |
| `isShuffleEnabled()` | Kiểm tra shuffle | bool |
| `toggleShuffle()` | Bật/tắt shuffle | bool |

**Repeat Mode State Diagram:**
```mermaid
stateDiagram-v2
    [*] --> NONE
    NONE --> ONE : cycleRepeatMode()
    ONE --> ALL : cycleRepeatMode()
    ALL --> NONE : cycleRepeatMode()
```

**Sequence Diagram - Cycle Repeat Mode:**
```mermaid
sequenceDiagram
    participant V as View
    participant PS as PlayerState
    participant PN as PlaylistNavigationImpl

    V->>PS: cycleRepeatMode()
    PS->>PN: cycleRepeatMode()
    Note over PN: current = mRepeatMode.load()
    alt current == NONE
        PN->>PN: mRepeatMode.store(ONE)
        PN-->>PS: return ONE
    else current == ONE
        PN->>PN: mRepeatMode.store(ALL)
        PN-->>PS: return ALL
    else current == ALL
        PN->>PN: mRepeatMode.store(NONE)
        PN-->>PS: return NONE
    end
    PS-->>V: new mode
```

---

### UC-M05: Lưu trữ thông tin MediaFile

| Thuộc tính | Mô tả |
|------------|-------|
| **Component** | MediaFile (Facade) |
| **Sub-components** | MediaFileInfo, MediaMetadata, CoverArt |

**Operations:**

| Interface | Method | Mô tả |
|-----------|--------|-------|
| IMediaFileInfo | `getFilename()` | Tên file (e.g., "song.mp3") |
| IMediaFileInfo | `getPath()` | Đường dẫn đầy đủ |
| IMediaFileInfo | `isValid()` | Kiểm tra file hợp lệ |
| IMediaMetadata | `getDuration()` | Độ dài (seconds) |
| IMediaMetadata | `getArtist()` | Tên artist |
| IMediaMetadata | `getAlbum()` | Tên album |
| ICoverArt | `getCoverArt()` | Dữ liệu ảnh cover |
| ICoverArt | `hasCoverArt()` | Có cover art không |

**Sequence Diagram - Create MediaFile:**
```mermaid
sequenceDiagram
    participant PM as PlaylistManager
    participant MF as MediaFile
    participant FI as MediaFileInfo
    participant MM as MediaMetadata
    participant CA as CoverArt

    PM->>MF: new MediaFile("song.mp3", "/path/song.mp3", 180, "Artist", "Album", coverData)
    MF->>FI: setFilename("song.mp3")
    MF->>FI: setPath("/path/song.mp3")
    MF->>MM: setDuration(180)
    MF->>MM: setArtist("Artist")
    MF->>MM: setAlbum("Album")
    MF->>CA: setCoverArt(coverData)
    MF-->>PM: MediaFile created
```

---

## Class Diagram

```mermaid
classDiagram
    class IPlayerState {
        <<interface>>
    }
    class IPlaybackState {
        <<interface>>
        +getPlaybackStatus() PlaybackStatus
        +setPlaybackStatus(status)
        +isPlaying() bool
        +togglePlayPause() PlaybackStatus
    }
    class IVolumeState {
        <<interface>>
        +getVolume() int
        +setVolume(volume)
        +isMuted() bool
        +toggleMute() bool
    }
    class ITrackPosition {
        <<interface>>
        +getCurrentPosition() uint32_t
        +setCurrentPosition(pos)
    }
    class IPlaylistNavigation {
        <<interface>>
        +getCurrentTrackIndex() int
        +getRepeatMode() RepeatMode
        +isShuffleEnabled() bool
    }
    
    class PlayerState {
        -mPlaybackState: PlaybackStateImpl
        -mVolumeState: VolumeStateImpl
        -mTrackPosition: TrackPositionImpl
        -mNavigation: PlaylistNavigationImpl
    }
    
    IPlayerState <|-- IPlaybackState
    IPlayerState <|-- IVolumeState
    IPlayerState <|-- ITrackPosition
    IPlayerState <|-- IPlaylistNavigation
    IPlayerState <|.. PlayerState
    
    class IMediaFile {
        <<interface>>
    }
    class IMediaFileInfo {
        <<interface>>
        +getFilename() string
        +getPath() string
        +isValid() bool
    }
    class IMediaMetadata {
        <<interface>>
        +getDuration() uint32_t
        +getArtist() string
        +getAlbum() string
    }
    class ICoverArt {
        <<interface>>
        +getCoverArt() vector~uint8_t~
        +hasCoverArt() bool
    }
    
    class MediaFile {
        -mFileInfo: MediaFileInfo
        -mMetadata: MediaMetadata
        -mCoverArt: CoverArt
    }
    
    IMediaFile <|-- IMediaFileInfo
    IMediaFile <|-- IMediaMetadata
    IMediaFile <|-- ICoverArt
    IMediaFile <|.. MediaFile
```

---

## Thread Safety Architecture

```mermaid
sequenceDiagram
    participant VT as View Thread
    participant CT as Controller Thread
    participant AT as Audio Thread
    participant PS as PlayerState (atomic)

    par Concurrent Access
        AT->>PS: setCurrentPosition(50000)
        Note over PS: atomic store
    and
        VT->>PS: getCurrentPosition()
        Note over PS: atomic load
        PS-->>VT: 50000
    and
        CT->>PS: setPlaybackStatus(PLAYING)
        Note over PS: atomic store
    end
    
    Note over PS: All operations are lock-free<br/>using std::atomic
```

---

## Enums Reference

### PlaybackStatus

| Value | Int | Mô tả |
|-------|-----|-------|
| STOPPED | 0 | Không có media hoặc đã dừng |
| PLAYING | 1 | Đang phát |
| PAUSED | 2 | Tạm dừng |

### RepeatMode

| Value | Int | Mô tả |
|-------|-----|-------|
| NONE | 0 | Không lặp |
| ONE | 1 | Lặp bài hiện tại |
| ALL | 2 | Lặp toàn bộ playlist |

---

## Usage Examples

```cpp
// Tạo PlayerState
auto playerState = std::make_shared<PlayerState>();

// Playback control
playerState->setPlaybackStatus(PlaybackStatus::PLAYING);
if (playerState->isPlaying()) {
    playerState->pause(); // -> PAUSED
}

// Volume control
playerState->setVolume(80);
playerState->toggleMute(); // -> muted = true

// Position (cập nhật từ AudioPlayer callback)
playerState->setCurrentPosition(45000); // 45 seconds

// Navigation
playerState->setCurrentTrackIndex(3);
playerState->cycleRepeatMode(); // NONE -> ONE

// MediaFile
MediaFile track("song.mp3", "/music/song.mp3", 180, "Artist", "Album");
std::cout << track.getFilename(); // "song.mp3"
std::cout << track.getDuration(); // 180
```
