# Utils Layer - Detailed Documentation

## Tổng quan

Thư mục `src/utils` chứa các utility classes dùng chung cho toàn ứng dụng:
- **Buffer**: Circular buffer cho audio streaming
- **Logger**: Thread-safe singleton logger
- **ThreadSafeQueue**: Hàng đợi cho inter-thread communication

Tất cả utilities đều **thread-safe**.

---

## Cấu trúc thư mục

```
src/utils/
├── IBuffer.h               # Buffer interface
├── Buffer.h/cpp            # Circular buffer implementation
├── ILogger.h               # Logger interface
├── Logger.h/cpp            # Singleton logger
├── IThreadSafeQueue.h      # Queue interface
└── ThreadSafeQueue.h       # Template queue implementation
```

---

## Buffer (Circular Buffer)

### Mô tả

Circular buffer dùng cho **Producer-Consumer pattern** trong audio streaming:
- **Producer**: Decoder thread ghi audio data
- **Consumer**: Playback thread đọc audio data

### Interface

```cpp
class IBuffer {
    virtual size_t write(const uint8_t* data, size_t len) = 0;
    virtual size_t read(uint8_t* dest, size_t len) = 0;
    virtual void clear() = 0;
    virtual size_t available() const = 0;
    virtual size_t capacity() const = 0;
};
```

### Use Case: UC-U01 - Audio Streaming

| Thuộc tính | Mô tả |
|------------|-------|
| **Actors** | Decoder Thread (Producer), Playback Thread (Consumer) |
| **Mô tả** | Truyền audio data giữa decoder và playback |
| **Buffer Size** | 1MB (default) |
| **Thread-Safety** | std::mutex protection |

**Sequence Diagram - Audio Streaming:**
```mermaid
sequenceDiagram
    participant D as Decoder Thread
    participant B as Buffer
    participant P as Playback Thread

    Note over D,P: Concurrent operation
    
    loop Decode loop
        D->>D: Decode audio frame (4096 bytes)
        D->>B: write(audioData, 4096)
        Note over B: Lock mutex
        B->>B: Copy to circular buffer
        B->>B: Update head pointer
        Note over B: Unlock mutex
        B-->>D: return bytesWritten
    end
    
    loop Playback loop
        P->>B: available()
        B-->>P: 8192 bytes
        P->>B: read(buffer, 4096)
        Note over B: Lock mutex
        B->>B: Copy from circular buffer
        B->>B: Update tail pointer
        Note over B: Unlock mutex
        B-->>P: return bytesRead
        P->>P: Play audio data
    end
```

**Circular Buffer Diagram:**
```
┌─────────────────────────────────────────────┐
│  Circular Buffer (1MB)                       │
├─────────────────────────────────────────────┤
│                                              │
│   ████ TAIL ────────────────► HEAD ████     │
│        ↑                        ↑            │
│     Consumer                 Producer        │
│      reads                    writes         │
│                                              │
│   [USED DATA]              [FREE SPACE]      │
└─────────────────────────────────────────────┘
```

### Use Case: UC-U02 - Buffer Full Handling

| Thuộc tính | Mô tả |
|------------|-------|
| **Scenario** | Producer writes faster than consumer reads |
| **Behavior** | Non-blocking, returns bytes actually written |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant D as Decoder
    participant B as Buffer

    D->>B: write(8192 bytes)
    Note over B: Only 4096 bytes free
    B->>B: Write 4096 bytes
    B-->>D: return 4096 (partial write)
    D->>D: Keep remaining 4096 for next call
```

### Use Case: UC-U03 - Buffer Empty Handling

| Thuộc tính | Mô tả |
|------------|-------|
| **Scenario** | Consumer reads faster than producer writes |
| **Behavior** | Returns 0 bytes, no blocking |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant P as Playback
    participant B as Buffer

    P->>B: available()
    B-->>P: 0 bytes
    P->>B: read(buffer, 4096)
    B-->>P: return 0 (nothing to read)
    P->>P: Handle buffer underrun
```

---

## Logger (Thread-Safe Singleton)

### Mô tả

Hệ thống logging với:
- 4 log levels: DEBUG, INFO, WARNING, ERROR
- Thread-safe với mutex
- Singleton pattern

### Interface

```cpp
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

class ILogger {
    virtual void setLevel(LogLevel level) = 0;
    virtual LogLevel getLevel() const = 0;
    virtual void log(LogLevel level, const std::string& message) = 0;
};
```

### Macros

```cpp
LOG_DEBUG(msg);   // [DEBUG] message
LOG_INFO(msg);    // [INFO] message
LOG_WARNING(msg); // [WARNING] message
LOG_ERROR(msg);   // [ERROR] message
```

### Use Case: UC-U04 - Application Logging

| Thuộc tính | Mô tả |
|------------|-------|
| **Actors** | Tất cả components trong ứng dụng |
| **Mô tả** | Ghi log messages với timestamp và level |
| **Output** | Console (stdout) |

**Sequence Diagram - Logging từ nhiều threads:**
```mermaid
sequenceDiagram
    participant T1 as Thread 1
    participant T2 as Thread 2
    participant L as Logger (Singleton)
    participant C as Console

    par Concurrent logging
        T1->>L: LOG_INFO("Track loaded")
        Note over L: Lock mutex
        L->>L: Format: "[timestamp] [INFO] Track loaded"
        L->>C: std::cout << message
        Note over L: Unlock mutex
    and
        T2->>L: LOG_ERROR("Connection failed")
        Note over L: Wait for mutex
        Note over L: Lock mutex
        L->>L: Format: "[timestamp] [ERROR] Connection failed"
        L->>C: std::cout << message
        Note over L: Unlock mutex
    end
```

### Use Case: UC-U05 - Log Level Filtering

| Thuộc tính | Mô tả |
|------------|-------|
| **Scenario** | Chỉ hiển thị log từ mức WARNING trở lên |
| **Config** | `Logger::getInstance().setLevel(LogLevel::WARNING)` |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant A as Application
    participant L as Logger

    A->>L: setLevel(WARNING)
    
    A->>L: LOG_DEBUG("Debug msg")
    Note over L: level DEBUG < WARNING
    Note over L: Message discarded
    
    A->>L: LOG_INFO("Info msg")
    Note over L: level INFO < WARNING
    Note over L: Message discarded
    
    A->>L: LOG_WARNING("Warning msg")
    Note over L: level WARNING >= WARNING
    L-->>A: Output to console
    
    A->>L: LOG_ERROR("Error msg")
    Note over L: level ERROR >= WARNING
    L-->>A: Output to console
```

---

## ThreadSafeQueue (Template)

### Mô tả

Hàng đợi thread-safe cho inter-thread communication:
- Producer push, Consumer pop
- Blocking và non-blocking options
- Condition variable cho efficient waiting

### Interface

```cpp
template <typename T>
class IThreadSafeQueue {
    virtual void push(const T& value) = 0;
    virtual bool tryPop(T& value) = 0;
    virtual void waitAndPop(T& value) = 0;
    virtual bool empty() const = 0;
};
```

### Use Case: UC-U06 - Command Queue

| Thuộc tính | Mô tả |
|------------|-------|
| **Actors** | Serial Thread (Producer), Main Thread (Consumer) |
| **Mô tả** | Truyền commands từ S32K board đến main thread |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant S as Serial Thread
    participant Q as ThreadSafeQueue
    participant M as Main Thread

    S->>S: Receive "PLAY" from serial
    S->>Q: push("PLAY")
    Note over Q: Lock mutex
    Q->>Q: mQueue.push("PLAY")
    Q->>Q: mCondVar.notify_one()
    Note over Q: Unlock mutex
    
    M->>Q: waitAndPop(cmd)
    Note over Q: Lock mutex
    Note over Q: mCondVar.wait() - blocked
    Note over Q: Woken up by notify
    Q->>Q: cmd = mQueue.front()
    Q->>Q: mQueue.pop()
    Note over Q: Unlock mutex
    Q-->>M: cmd = "PLAY"
    M->>M: Process "PLAY" command
```

### Use Case: UC-U07 - Non-blocking Poll

| Thuộc tính | Mô tả |
|------------|-------|
| **Scenario** | Main loop polls queue mà không block |
| **Method** | tryPop() |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant M as Main Thread
    participant Q as ThreadSafeQueue

    loop Main loop
        M->>Q: tryPop(cmd)
        alt Queue has data
            Q-->>M: return true, cmd = "NEXT"
            M->>M: Process command
        else Queue empty
            Q-->>M: return false
            M->>M: Continue other work
        end
    end
```

### Use Case: UC-U08 - Blocking Wait

| Thuộc tính | Mô tả |
|------------|-------|
| **Scenario** | Dedicated consumer thread chờ data |
| **Method** | waitAndPop() - blocks until data available |
| **Benefit** | CPU efficient (thread sleeps instead of spinning) |

**Sequence Diagram:**
```mermaid
sequenceDiagram
    participant C as Consumer Thread
    participant Q as ThreadSafeQueue
    participant P as Producer Thread

    C->>Q: waitAndPop(data)
    Note over Q: Queue empty
    Note over C: Thread SLEEPING<br/>(no CPU usage)
    
    Note over P: After 5 seconds...
    P->>Q: push(newData)
    Q->>Q: notify_one()
    
    Note over C: Thread WAKES UP
    Q-->>C: data = newData
    C->>C: Process data
```

---

## Class Diagram

```mermaid
classDiagram
    class IBuffer {
        <<interface>>
        +write(data, len) size_t
        +read(dest, len) size_t
        +clear() void
        +available() size_t
        +capacity() size_t
    }
    
    class Buffer {
        -mBuffer: vector~uint8_t~
        -mHead: size_t
        -mTail: size_t
        -mCapacity: size_t
        -mMutex: mutex
        +write(data, len) size_t
        +read(dest, len) size_t
    }
    
    class ILogger {
        <<interface>>
        +setLevel(level) void
        +getLevel() LogLevel
        +log(level, message) void
    }
    
    class Logger {
        -mLevel: LogLevel
        -mMutex: mutex
        +getInstance() Logger&
        +log(level, message) void
    }
    
    class IThreadSafeQueue~T~ {
        <<interface>>
        +push(value) void
        +tryPop(value) bool
        +waitAndPop(value) void
        +empty() bool
    }
    
    class ThreadSafeQueue~T~ {
        -mQueue: queue~T~
        -mMutex: mutex
        -mCondVar: condition_variable
        +push(value) void
        +tryPop(value) bool
        +waitAndPop(value) void
    }
    
    IBuffer <|.. Buffer
    ILogger <|.. Logger
    IThreadSafeQueue <|.. ThreadSafeQueue
```

---

## Thread Safety Summary

| Utility | Protection | Pattern |
|---------|------------|---------|
| Buffer | std::mutex | Circular Buffer + Lock |
| Logger | std::mutex | Singleton + Lock |
| ThreadSafeQueue | std::mutex + std::condition_variable | Producer-Consumer |

---

## Usage Examples

### Buffer

```cpp
Utils::Buffer audioBuffer(1024 * 1024); // 1MB

// Producer (Decoder thread)
uint8_t decodedAudio[4096];
size_t written = audioBuffer.write(decodedAudio, 4096);

// Consumer (Playback thread)
uint8_t playbackBuffer[4096];
if (audioBuffer.available() >= 4096) {
    size_t read = audioBuffer.read(playbackBuffer, 4096);
    // Play audio
}

// Reset
audioBuffer.clear();
```

### Logger

```cpp
// Configure
Utils::Logger::getInstance().setLevel(Utils::LogLevel::DEBUG);

// Use macros
LOG_INFO("Application started");
LOG_DEBUG("Loading track: " << trackName);
LOG_WARNING("Buffer running low: " << available << " bytes");
LOG_ERROR("Failed to connect: " << errorMessage);

// Output:
// [2024-01-15 10:30:45] [INFO] Application started
// [2024-01-15 10:30:45] [DEBUG] Loading track: song.mp3
```

### ThreadSafeQueue

```cpp
Utils::ThreadSafeQueue<std::string> commandQueue;

// Producer (Serial thread)
void onSerialReceive(const std::string& cmd) {
    commandQueue.push(cmd);
}

// Consumer Option 1: Non-blocking
std::string cmd;
while (commandQueue.tryPop(cmd)) {
    processCommand(cmd);
}

// Consumer Option 2: Blocking (dedicated thread)
void commandProcessor() {
    std::string cmd;
    while (running) {
        commandQueue.waitAndPop(cmd); // Blocks efficiently
        processCommand(cmd);
    }
}
```
