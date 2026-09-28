#include "vstream/AudioRingBuffer.h"
#include <algorithm>

namespace vstream {

bool AudioRingBuffer::push(const float* samples, std::size_t count) noexcept
{
    if (count > capacity_ - available())
        return false;

    auto w = write_.load(std::memory_order_relaxed);
    for (std::size_t i = 0; i < count; ++i)
        buffer_[(w + i) % capacity_] = samples[i];

    write_.store(w + count, std::memory_order_release);
    return true;
}

std::size_t AudioRingBuffer::pop(float* destination, std::size_t maxCount) noexcept
{
    const auto availableSamples = available();
    const auto count = std::min(maxCount, availableSamples);
    auto r = read_.load(std::memory_order_relaxed);

    for (std::size_t i = 0; i < count; ++i)
        destination[i] = buffer_[(r + i) % capacity_];

    read_.store(r + count, std::memory_order_release);
    return count;
}

std::size_t AudioRingBuffer::available() const noexcept
{
    return write_.load(std::memory_order_acquire)
         - read_.load(std::memory_order_acquire);
}

} // namespace vstream
