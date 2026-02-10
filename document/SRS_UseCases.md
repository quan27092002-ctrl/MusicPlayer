# Đặc tả Use Case (Use Case Specification)

## UC1: Play/Pause Music
**Mô tả**: Cho phép người dùng hoặc hệ thống (S32K Board) bắt đầu phát hoặc tạm dừng bài hát đang được chọn.

**Actor**: User, S32K Board

**Tiền điều kiện (Pre-conditions)**:
- Track đã được load (UC2).
- Audio system đã được khởi tạo (initialized).
- Đối với Pause: Music đang phát (PlayerState = PLAYING).

**Hậu điều kiện (Post-conditions)**:
- Trạng thái phát nhạc thay đổi (Playing <-> Paused).
- PlayerState được cập nhật (PLAYING hoặc PAUSED).
- UI hiển thị trạng thái tương ứng (icon Play/Pause thay đổi).

**Luồng sự kiện chính (Main Flow) - Play**:
1. Actor (User click nút Play hoặc Board gửi lệnh).
2. View gọi `AppController.play()`.
3. `PlaybackController` gọi `AudioPlayer.play()`.
4. `AudioPlaybackImpl` gọi `Mix_ResumeMusic()`.
5. `PlayerState` cập nhật trạng thái thành PLAYING.
6. UI cập nhật icon Play thành Pause.

**Luồng sự kiện chính (Main Flow) - Pause**:
1. Actor (User click nút Pause hoặc Board gửi lệnh).
2. View gọi `AppController.pause()`.
3. `PlaybackController` gọi `AudioPlayer.pause()`.
4. `AudioPlaybackImpl` gọi `Mix_PauseMusic()`.
5. `PlayerState` cập nhật trạng thái thành PAUSED.
6. UI cập nhật icon Pause thành Play.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Chưa load track**:
  1. Hiển thị thông báo lỗi trên UI.
  2. Không thực hiện hành động phát nhạc.

---

## UC2: Load Track
**Mô tả**: Người dùng chọn một bài hát từ danh sách để nạp vào hệ thống phát nhạc.

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- Library đã có tracks.
- UI đang hiển thị danh sách nhạc.

**Hậu điều kiện (Post-conditions)**:
- Track được load vào `AudioPlayer`.
- Metadata (Title, Artist, Album) hiển thị trên UI.
- Hệ thống sẵn sàng để phát (Ready to play).

**Luồng sự kiện chính (Main Flow)**:
1. User click vào track trong danh sách nhạc.
2. View gọi `AppController.loadTrack(filePath)`.
3. `PlaybackController` tạo đối tượng `MediaFile`.
4. `MediaFile` trích xuất Metadata sử dụng `TagLib`.
5. `AudioLoaderImpl` load file sử dụng `Mix_LoadMUS()`.
6. `PlayerState` cập nhật `currentTrack`.
7. UI hiển thị thông tin bài hát tại khu vực Now Playing.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. File không tồn tại**:
  1. `AudioLoader` trả về lỗi.
  2. Hiển thị thông báo lỗi cho User.
- **A2. Định dạng file không hỗ trợ**:
  1. `Mix_LoadMUS()` thất bại.
  2. Log error và thông báo cho User.
- **A3. Metadata bị lỗi hoặc thiếu**:
  1. Sử dụng filename làm title.
  2. Đặt Artist/Album là "Unknown".

---

## UC3: Adjust Volume (UI)
**Mô tả**: Người dùng điều chỉnh âm lượng thông qua giao diện ứng dụng (Application UI).

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- UI đang hiển thị.
- Audio system đã được khởi tạo.

**Hậu điều kiện (Post-conditions)**:
- Giá trị Volume của Audio system thay đổi.
- `PlayerState` lưu giá trị Volume mới.
- UI slider hiển thị vị trí mới.

**Luồng sự kiện chính (Main Flow)**:
1. User kéo thanh trượt (slider) volume trên PlayerBar.
2. View gọi `AppController.setVolume(value)`.
3. `VolumeController` gọi `AudioPlayer.setVolume(value)`.
4. `AudioVolumeImpl` gọi `Mix_VolumeMusic(sdlValue)`. (Quy đổi: sdlValue = value * 128 / 100).
5. `PlayerState.setVolume(value)`.
6. UI cập nhật vị trí slider.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Giá trị Volume ngoài khoảng (0-100)**:
  1. Hệ thống clamp giá trị về 0 hoặc 100.
  2. Tiếp tục thực hiện Main Flow.

---

## UC4: Adjust Volume (Board)
**Mô tả**: Người dùng điều chỉnh âm lượng bằng cách xoay núm vặn trên S32K Board.

**Actor**: User, S32K Board

**Tiền điều kiện (Pre-conditions)**:
- Serial connection đã được thiết lập (UC9).
- Board đang hoạt động và kết nối với biến trở (potentiometer).

**Hậu điều kiện (Post-conditions)**:
- Giá trị Volume thay đổi đồng bộ với thao tác trên Board.
- UI slider cập nhật theo giá trị mới từ Board.

**Luồng sự kiện chính (Main Flow)**:
1. User xoay núm volume trên Board.
2. Board đọc giá trị ADC (0-4095).
3. Board gửi bản tin `VR: {value}\n` qua Serial.
4. `SerialIOImpl` nhận dữ liệu.
5. `BoardCommunicator` parse dữ liệu và quy đổi: `volume = adc * 100 / 4095`.
6. Gọi `AppController.setVolume(percent)`.
7. `VolumeController` cập nhật `AudioPlayer`.
8. UI slider tự động đồng bộ hiển thị giá trị mới.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Serial bị ngắt kết nối**:
  1. Dữ liệu không được nhận.
  2. Volume không thay đổi.
- **A2. Giá trị ADC không hợp lệ**:
  1. `BoardCommunicator` log warning.
  2. Bỏ qua lệnh điều chỉnh.

---

## UC5: Control Playback (Board)
**Mô tả**: Người dùng điều khiển phát nhạc (Play, Pause, Next, Prev) thông qua các nút bấm vật lý trên S32K Board.

**Actor**: User, S32K Board

**Tiền điều kiện (Pre-conditions)**:
- Serial connection đã được thiết lập (UC9).
- Track đã được load (đối với lệnh Play/Pause).
- Playlist có bài (đối với Next/Prev).

**Hậu điều kiện (Post-conditions)**:
- Trạng thái playback thay đổi tương ứng (Playing/Paused/Track Changed).
- UI cập nhật trạng thái mới.

**Luồng sự kiện chính (Main Flow)**:
1. User nhấn nút (SW2, SW3...) trên Board.
2. Board gửi lệnh `cmd:{command}\n` qua Serial.
3. `SerialIOImpl` nhận dữ liệu.
4. `BoardCommunicator` parse lệnh.
5. Gọi `AppController` tương ứng:
   - `play` -> `play()`
   - `pause` -> `pause()`
   - `next` -> `next()`
   - `prev` -> `previous()`
6. `PlaybackController` thực hiện hành động.
7. UI cập nhật trạng thái mới.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Command không hợp lệ**:
  1. `BoardCommunicator` log warning.
  2. Bỏ qua lệnh.
- **A2. Không có track nào được load (khi Play)**:
  1. `PlaybackController` trả về lỗi.
  2. Không có thay đổi trạng thái.

---

## UC6: Shuffle/Repeat
**Mô tả**: Người dùng thay đổi chế độ phát ngẫu nhiên (Shuffle) hoặc lặp lại (Repeat).

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- Có tracks trong Queue.

**Hậu điều kiện (Post-conditions)**:
- Thứ tự phát trong Queue thay đổi (Shuffle).
- Chế độ Repeat của `PlayerState` thay đổi.
- UI icon hiển thị trạng thái tương ứng.

**Luồng sự kiện chính - Toggle Shuffle (Enable)**:
1. User click nút Shuffle.
2. View gọi `AppController.toggleShuffle()`.
3. `PlaybackController` lưu thứ tự gốc (`originalOrder`).
4. `PlaylistManager` xáo trộn (randomize) Queue.
5. `PlayerState.setShuffleEnabled(true)`.
6. UI highlight icon Shuffle.

**Luồng sự kiện chính - Toggle Repeat (Cycle)**:
1. User click nút Repeat.
2. View gọi `AppController.toggleRepeat()`.
3. Hệ thống chuyển đổi chế độ theo vòng lặp: OFF -> REPEAT_ALL -> REPEAT_ONE -> OFF.
4. `PlayerState.setRepeatMode(newMode)`.
5. UI thay đổi icon Repeat tương ứng.

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Disable Shuffle**:
  1. `PlaylistManager` khôi phục thứ tự gốc từ `originalOrder`.
  2. `PlayerState.setShuffleEnabled(false)`.
  3. UI un-highlight icon Shuffle.

---

## UC7: Manage Playlist
**Mô tả**: Người dùng tạo mới, thêm bài hát, xóa hoặc phát Playlist.

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- UI đang hiển thị tab Playlist.

**Hậu điều kiện (Post-conditions)**:
- Playlist mới được tạo, cập nhật hoặc xóa.

**Luồng sự kiện chính - Create Playlist**:
1. User click "Create Playlist".
2. Hệ thống hiển thị Modal nhập thông tin.
3. User nhập Tên, Mô tả và chọn màu đại diện.
4. `PlaylistManager.createPlaylist()` được gọi.
5. Modal đóng, danh sách Playlist được làm mới.

**Luồng sự kiện chính - Add Track to Playlist**:
1. User click icon "+" trên một track trong Library.
2. Menu hiển thị danh sách Playlist hiện có.
3. User chọn Playlist đích.
4. `PlaylistManager.addTrackToPlaylist()` được gọi.
5. Hệ thống hiển thị thông báo xác nhận.

**Luồng sự kiện chính - Play Playlist**:
1. User click nút Play trên Playlist.
2. View lấy danh sách đường dẫn các bài hát (`trackPaths`) trong Playlist.
3. `AppController.playPlaylist(paths)`.
4. Queue hiện tại bị thay thế bởi danh sách mới.
5. Hệ thống bắt đầu phát bài đầu tiên.

---

## UC8: Load from USB
**Mô tả**: Người dùng quét và nạp nhạc từ thiết bị lưu trữ USB được kết nối.

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- USB đã được cắm vào máy tính.
- Hệ điều hành đã mount USB.

**Hậu điều kiện (Post-conditions)**:
- Các file nhạc trong USB được tìm thấy và thêm vào Library.
- UI hiển thị số lượng bài hát đã load.

**Luồng sự kiện chính (Main Flow)**:
1. User click "Refresh" trên Storage Panel.
2. `StorageManager` quét các ổ đĩa đã mount.
3. User chọn thiết bị USB từ danh sách và click "Load".
4. `AppController.loadFromStorage(path)`.
5. `PlaylistManager.loadDirectoryAsync(path)` chạy quét đệ quy.
6. Hệ thống lọc các file `.mp3`, `.wav`, `.flac`.
7. Với mỗi file, tạo `MediaFile` và trích xuất Metadata.
8. Thêm track hợp lệ vào Library.
9. Hiển thị thông báo "Loaded X tracks".

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. USB bị ngắt kết nối trong khi quét**:
  1. Quá trình quét dừng lại.
  2. Hiển thị thông báo lỗi.
- **A2. File bị lỗi (Corrupt)**:
  1. Bỏ qua file lỗi.
  2. Tiếp tục quét các file còn lại.
  3. Log warning.

---

## UC9: Connect Serial Port
**Mô tả**: Người dùng thiết lập kết nối Serial với S32K Board để kích hoạt tính năng điều khiển phần cứng.

**Actor**: User

**Tiền điều kiện (Pre-conditions)**:
- Cổng Serial (COM/tty) khả dụng trên hệ thống.

**Hậu điều kiện (Post-conditions)**:
- Kết nối Serial được thiết lập.
- Thread đọc dữ liệu nền (`Read thread`) đang chạy.
- UI hiển thị trạng thái "Connected".

**Luồng sự kiện chính (Main Flow)**:
1. User click "Refresh Ports" để cập nhật danh sách cổng.
2. User chọn cổng và click "Connect".
3. `AppController.connectToPort(name)`.
4. `SerialConnectionImpl.connect(name, 115200)`.
5. Hệ thống mở cổng với cờ `O_RDWR | O_NOCTTY` và cấu hình `termios` (115200, 8N1).
6. `SerialIOImpl` khởi chạy thread đọc dữ liệu.
7. UI cập nhật trạng thái kết nối thành "Connected".

**Luồng sự kiện thay thế (Alternative Flows)**:
- **A1. Không mở được cổng**:
  1. Lỗi "Permission denied" hoặc "Device busy".
  2. UI hiển thị thông báo lỗi.
- **A2. Ngắt kết nối (Disconnect)**:
  1. User click "Disconnect".
  2. `SerialIOImpl` dừng thread đọc.
  3. `SerialConnectionImpl` đóng cổng và giải phóng tài nguyên.
  4. UI cập nhật trạng thái "Disconnected".
