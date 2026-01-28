#include "Buffer.h"
#include <cstring>  // Dùng cho std::memcpy
#include <algorithm> // Dùng cho std::min
#include <iostream>

namespace Utils {

// Constructor: Khởi tạo buffer với kích thước cố định
Buffer::Buffer(size_t size) 
    : m_buffer(size), m_readPos(0), m_writePos(0), m_dataAvailable(0), m_capacity(size) {
}

Buffer::~Buffer() {
    // Vector tự giải phóng bộ nhớ, không cần code dọn dẹp thủ công
}

// Ghi dữ liệu vào Buffer
// Trả về số byte thực tế đã ghi được
size_t Buffer::write(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(m_mutex); // Thread-safe

    size_t freeSpace = m_capacity - m_dataAvailable;
    size_t toWrite = std::min(len, freeSpace);

    if (toWrite == 0) {
        return 0;
    }

    // Tính toán phần ghi ở cuối buffer (trước khi vòng lại)
    size_t spaceAtEnd = m_capacity - m_writePos;
    size_t chunk1 = std::min(toWrite, spaceAtEnd);
    
    std::memcpy(&m_buffer[m_writePos], data, chunk1);

    // Nếu còn dư, vòng lại ghi từ đầu buffer
    if (chunk1 < toWrite) {
        size_t chunk2 = toWrite - chunk1;
        std::memcpy(&m_buffer[0], data + chunk1, chunk2);
    }

    // Cập nhật vị trí ghi và số lượng dữ liệu
    m_writePos = (m_writePos + toWrite) % m_capacity;
    m_dataAvailable += toWrite;

    return toWrite;
}

// Đọc dữ liệu từ Buffer
// Trả về số byte thực tế đọc được
size_t Buffer::read(uint8_t* dest, size_t len) {
    std::lock_guard<std::mutex> lock(m_mutex); // Thread-safe

    size_t toRead = std::min(len, m_dataAvailable);

    if (toRead == 0) {
        return 0;
    }

    // Tính toán phần đọc ở cuối buffer
    size_t dataAtEnd = m_capacity - m_readPos;
    size_t chunk1 = std::min(toRead, dataAtEnd);

    std::memcpy(dest, &m_buffer[m_readPos], chunk1);

    // Nếu cần đọc tiếp, vòng lại đầu buffer
    if (chunk1 < toRead) {
        size_t chunk2 = toRead - chunk1;
        std::memcpy(dest + chunk1, &m_buffer[0], chunk2);
    }

    // Cập nhật vị trí đọc và số lượng dữ liệu
    m_readPos = (m_readPos + toRead) % m_capacity;
    m_dataAvailable -= toRead;

    return toRead;
}

// Kiểm tra số byte dữ liệu hiện có thể đọc
size_t Buffer::getAvailable() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_dataAvailable;
}

// Kiểm tra khoảng trống còn lại để ghi
size_t Buffer::getFreeSpace() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_capacity - m_dataAvailable;
}

// Xóa buffer (reset các chỉ số)
void Buffer::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_readPos = 0;
    m_writePos = 0;
    m_dataAvailable = 0;
}

} // namespace Utils