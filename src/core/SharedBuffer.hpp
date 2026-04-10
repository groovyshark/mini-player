#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include <cstdint>

struct SharedBuffer {
    std::vector<uint8_t> data;
    
    int width{0};
    int height{0};

    std::atomic<bool> isDirty{false}; // flag to indicate if the buffer has been modified
    std::mutex mutex;
};

using SharedBufferPtr = std::shared_ptr<SharedBuffer>;