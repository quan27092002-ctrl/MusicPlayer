#include "Buffer.h"
#include <cstring>  // Dùng cho std::memcpy
#include <algorithm> // Dùng cho std::min

namespace Utils {

// Constructor: Khởi tạo buffer với kích thước cố định
Buffer::Buffer(size_t capacityBytes) 
    : mBuffer(capacityBytes), mHead(0), mTail(0), mIsFull(false), mCapacity(capacityBytes) {
}

// Ghi dữ liệu vào Buffer
// Trả về số byte thực tế đã ghi được
size_t Buffer::write(const uint8_t* data, size_t len) {
    std::lock_guard<std::mutex> lock(mMutex);

    size_t freeSpace = mCapacity - getAvailableInternal();
    size_t toWrite = std::min(len, freeSpace);

    if (toWrite == 0) {
        return 0;
    }

    for (size_t i = 0; i < toWrite; ++i) {
        mBuffer[mHead] = data[i];
        mHead = (mHead + 1) % mCapacity;
    }

    // Cập nhật trạng thái full
    if (mHead == mTail && toWrite > 0) {
        mIsFull = true;
    }

    return toWrite;
}

// Đọc dữ liệu từ Buffer
// Trả về số byte thực tế đọc được
size_t Buffer::read(uint8_t* dest, size_t len) {
    std::lock_guard<std::mutex> lock(mMutex);

    size_t avail = getAvailableInternal();
    size_t toRead = std::min(len, avail);

    if (toRead == 0) {
        return 0;
    }

    for (size_t i = 0; i < toRead; ++i) {
        dest[i] = mBuffer[mTail];
        mTail = (mTail + 1) % mCapacity;
    }

    mIsFull = false;

    return toRead;
}

// Kiểm tra số byte dữ liệu hiện có thể đọc
size_t Buffer::available() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return getAvailableInternal();
}

// Trả về capacity của buffer
size_t Buffer::capacity() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mCapacity;
}

// Xóa buffer (reset các chỉ số)
void Buffer::clear() {
    std::lock_guard<std::mutex> lock(mMutex);
    mHead = 0;
    mTail = 0;
    mIsFull = false;
}

// Internal helper: tính số byte available (không lock)
size_t Buffer::getAvailableInternal() const {
    if (mIsFull) {
        return mCapacity;
    }
    if (mHead >= mTail) {
        return mHead - mTail;
    }
    return mCapacity - mTail + mHead;
}

bool Buffer::isFullInternal() const {
    return mIsFull;
}

bool Buffer::isEmptyInternal() const {
    return (!mIsFull && (mHead == mTail));
}

} // namespace Utils