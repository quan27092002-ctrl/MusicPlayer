# View Layer - Detailed Documentation

## Tổng quan

Thư mục `src/view` chứa các thành phần giao diện người dùng (UI) của ứng dụng Music Player. Layer này:
- Hiển thị trạng thái từ **Model** (PlayerState)
- Gửi lệnh đến **Controller** (AppController)
- Sử dụng **ImGui** làm thư viện đồ họa

---

## Cấu trúc thư mục

```
src/view/
├── IView.h                     # Interface trừu tượng
├── ImGuiView.h/cpp             # Facade implementation
├── imgui/                      # Thư viện ImGui (third-party)
└── imguiview/
    ├── AssetManager.h/cpp          # Quản lý fonts, images
    ├── LifecycleManager.h/cpp      # GLFW/OpenGL lifecycle
    ├── components/
    │   ├── MainContent.h/cpp       # Playlist view, tabs
    │   ├── RightSidebar.h/cpp      # Track info, cover art
    │   └── PlayerBar.h/cpp         # Playback controls, seek bar
    └── interfaces/
        └── IWindowComponent.h      # Interface cho UI components
```

---

## Use Cases

### UC-V01: Hiển thị danh sách nhạc (Display Playlist)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User xem danh sách các bài hát đã load |
| **Precondition** | Playlist đã được load từ thư mục |
| **Postcondition** | Danh sách nhạc hiển thị trên màn hình |

**Luồng chính:**
1. User mở ứng dụng
2. View gọi `controller->getPlaylistSize()` để lấy số lượng track
3. View gọi `controller->getTrackName(i)` cho mỗi track
4. MainContent render danh sách trong tab "Music"

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant MC as MainContent
    participant C as IAppController
    
    U->>MC: Mở ứng dụng / Tab Music
    MC->>C: getPlaylistSize()
    C-->>MC: size = 10
    loop Mỗi track i
        MC->>C: getTrackName(i)
        C-->>MC: "Song Name"
        MC->>C: getTrackArtist(i)
        C-->>MC: "Artist"
        MC->>C: getTrackDuration(i)
        C-->>MC: 180 (seconds)
    end
    MC->>MC: Render danh sách
    MC-->>U: Hiển thị playlist
```

---

### UC-V02: Chọn và phát bài hát (Select and Play Track)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User click vào bài hát để phát |
| **Precondition** | Playlist không rỗng |
| **Postcondition** | Bài hát được phát, UI cập nhật trạng thái PLAYING |

**Luồng chính:**
1. User click vào track trong MainContent
2. View gọi `controller->playTrack(index)`
3. Controller load track và phát
4. Model cập nhật `PlaybackStatus = PLAYING`
5. PlayerBar đọc state và hiển thị nút Pause

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant MC as MainContent
    participant C as IAppController
    participant S as IPlayerState
    participant PB as PlayerBar

    U->>MC: Click track[3]
    MC->>C: playTrack(3)
    Note over C: Load & play track
    C->>S: setPlaybackStatus(PLAYING)
    C->>S: setCurrentTrackIndex(3)
    
    loop Render loop
        PB->>S: getPlaybackStatus()
        S-->>PB: PLAYING
        PB->>S: getCurrentTrackIndex()
        S-->>PB: 3
        PB->>C: getTrackName(3)
        C-->>PB: "Song Name"
        PB->>PB: Render Play/Pause button
    end
    PB-->>U: UI shows PLAYING state
```

---

### UC-V03: Điều khiển Play/Pause

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User click nút Play/Pause |
| **Precondition** | Có track đang được load |
| **Postcondition** | Trạng thái playback thay đổi |

**Luồng chính (Play):**
1. User click nút Play
2. PlayerBar gọi `controller->play()`
3. Controller phát nhạc
4. Model cập nhật `PlaybackStatus = PLAYING`
5. PlayerBar đọc state, đổi icon thành Pause

**Luồng thay thế (Pause):**
1. User click nút Pause (khi đang PLAYING)
2. PlayerBar gọi `controller->pause()`
3. Model cập nhật `PlaybackStatus = PAUSED`
4. PlayerBar đổi icon thành Play

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant PB as PlayerBar
    participant C as IAppController
    participant S as IPlayerState

    alt Đang STOPPED hoặc PAUSED
        U->>PB: Click Play button
        PB->>C: play()
        C->>S: setPlaybackStatus(PLAYING)
        PB->>S: getPlaybackStatus()
        S-->>PB: PLAYING
        PB-->>U: Show Pause icon
    else Đang PLAYING
        U->>PB: Click Pause button
        PB->>C: pause()
        C->>S: setPlaybackStatus(PAUSED)
        PB->>S: getPlaybackStatus()
        S-->>PB: PAUSED
        PB-->>U: Show Play icon
    end
```

---

### UC-V04: Điều khiển Next/Previous

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User chuyển bài tiếp theo/trước đó |
| **Precondition** | Playlist có nhiều hơn 1 track |
| **Postcondition** | Track mới được phát |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant PB as PlayerBar
    participant C as IAppController
    participant S as IPlayerState

    U->>PB: Click Next button
    PB->>C: next()
    Note over C: Calculate next index<br/>Push current to history<br/>Load & play next track
    C->>S: setCurrentTrackIndex(newIndex)
    C->>S: setPlaybackStatus(PLAYING)
    
    PB->>S: getCurrentTrackIndex()
    S-->>PB: newIndex
    PB->>C: getTrackName(newIndex)
    C-->>PB: "New Song"
    PB-->>U: Update track info
```

---

### UC-V05: Điều chỉnh Seek Bar

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User kéo seek bar để thay đổi vị trí phát |
| **Precondition** | Có track đang phát hoặc pause |
| **Postcondition** | Vị trí phát được cập nhật |

**Luồng chính:**
1. User kéo slider trên seek bar
2. PlayerBar tính toán position mới (ms)
3. Khi release, gọi `controller->seek(positionMs)`
4. Controller gọi AudioPlayer seek
5. Model cập nhật `currentPosition`

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant PB as PlayerBar
    participant C as IAppController
    participant S as IPlayerState

    U->>PB: Drag seek slider to 50%
    Note over PB: Calculate: position = duration * 0.5
    U->>PB: Release slider
    PB->>C: seek(90000)
    Note over C: Seek audio to 90s
    C->>S: setCurrentPosition(90000)
    
    loop Render loop
        PB->>S: getCurrentPosition()
        S-->>PB: 90000
        PB->>S: getTrackDuration()
        S-->>PB: 180000
        PB->>PB: Render slider at 50%
    end
```

---

### UC-V06: Điều chỉnh Volume

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User kéo slider để thay đổi âm lượng |
| **Precondition** | None |
| **Postcondition** | Volume được cập nhật (0-100) |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant PB as PlayerBar
    participant C as IAppController
    participant S as IPlayerState

    U->>PB: Drag volume slider to 80%
    PB->>C: setVolume(80)
    C->>S: setVolume(80)
    
    PB->>S: getVolume()
    S-->>PB: 80
    PB-->>U: Update volume slider UI
```

---

### UC-V07: Toggle Mute

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User click nút mute/unmute |
| **Precondition** | None |
| **Postcondition** | Trạng thái mute được toggle |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant PB as PlayerBar
    participant C as IAppController
    participant S as IPlayerState

    U->>PB: Click Mute button
    PB->>C: toggleMute()
    C->>S: toggleMute()
    
    PB->>S: isMuted()
    S-->>PB: true
    PB-->>U: Show Muted icon
```

---

### UC-V08: Tìm kiếm bài hát (Search)

| Thuộc tính | Mô tả |
|------------|-------|
| **Actor** | User |
| **Mô tả** | User nhập từ khóa để lọc danh sách |
| **Precondition** | Playlist không rỗng |
| **Postcondition** | Danh sách được lọc theo từ khóa |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant U as User
    participant MC as MainContent
    participant C as IAppController

    U->>MC: Nhập "love" vào search box
    Note over MC: Lưu query = "love"
    
    loop Mỗi track i
        MC->>C: getTrackName(i)
        C-->>MC: "Love Song"
        MC->>MC: matchesSearch("Love Song", "love")
        alt Match
            MC->>MC: Add to filtered list
        end
    end
    MC-->>U: Hiển thị tracks có chứa "love"
```

---

## Class Diagram

```mermaid
classDiagram
    class IView {
        <<interface>>
        +initialize() bool
        +shutdown() void
        +isRunning() bool
        +processEvents() void
        +render() void
    }
    
    class ImGuiView {
        -mController: IAppController*
        -mPlayerState: IPlayerState*
        -mLifecycle: LifecycleManager
        -mAssetManager: AssetManager
        -mMainContent: MainContent
        -mRightSidebar: RightSidebar
        -mPlayerBar: PlayerBar
        +initialize() bool
        +shutdown() void
        +render() void
    }
    
    class IWindowComponent {
        <<interface>>
        +render() void
    }
    
    class MainContent {
        -mPlaylistDisplay: vector~string~
        -mSearchQuery: char[]
        +render() void
        +setPlaylist(playlist) void
    }
    
    class PlayerBar {
        -mWasPlaying: bool
        -mIsDraggingSlider: bool
        +render() void
        +update(width, height) void
    }
    
    class RightSidebar {
        +render() void
    }
    
    IView <|.. ImGuiView
    IWindowComponent <|.. MainContent
    IWindowComponent <|.. PlayerBar
    IWindowComponent <|.. RightSidebar
    ImGuiView *-- MainContent
    ImGuiView *-- PlayerBar
    ImGuiView *-- RightSidebar
    ImGuiView --> IAppController : uses
    ImGuiView --> IPlayerState : reads
```

---

## Component Responsibilities

| Component | Trách nhiệm | Gọi Controller Methods | Đọc Model State |
|-----------|-------------|------------------------|-----------------|
| **MainContent** | Hiển thị playlist, tabs, search | `playTrack()`, `getTrackName()`, `getPlaylistSize()` | `getCurrentTrackIndex()` |
| **PlayerBar** | Điều khiển playback, seek, volume | `play()`, `pause()`, `next()`, `previous()`, `seek()`, `setVolume()`, `toggleMute()` | `getPlaybackStatus()`, `getCurrentPosition()`, `getVolume()`, `isMuted()` |
| **RightSidebar** | Hiển thị thông tin track hiện tại | `getTrackArtist()`, `getTrackAlbum()`, `getTrackCoverArt()` | `getCurrentTrackIndex()` |
| **AssetManager** | Load fonts, textures | - | - |
| **LifecycleManager** | Khởi tạo GLFW, OpenGL, ImGui | - | - |

---

## Main Loop Flow

```mermaid
sequenceDiagram
    participant App as main()
    participant V as ImGuiView
    participant LC as LifecycleManager
    participant MC as MainContent
    participant PB as PlayerBar
    participant RS as RightSidebar

    App->>V: initialize()
    V->>LC: initialize()
    LC-->>V: OK
    
    loop while isRunning()
        App->>V: processEvents()
        V->>LC: pollEvents()
        
        App->>V: render()
        V->>V: BeginFrame()
        V->>MC: render()
        V->>RS: render()
        V->>PB: render()
        V->>V: EndFrame()
        V->>LC: swapBuffers()
    end
    
    App->>V: shutdown()
    V->>LC: shutdown()
```
