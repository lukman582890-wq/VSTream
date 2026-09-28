#pragma once
#include <vector>
#include <atomic>
#include <cstddef>

namespace vstream {

class AudioRingBuffer {
public:
    explicit AudioRingBuffer(std::size_t capacitySamples)
        : buffer_(capacitySamples), capacity_(capacitySamples) {}

    bool push(const float* samples, std::size_t count) noexcept;
    std::size_t pop(float* destination, std::size_t maxCount) noexcept;
    std::size_t available() const noexcept;

private:
    std::vector<float> buffer_;
    const std::size_t capacity_;
    std::atomic<std::size_t> write_{0};
    std::atomic<std::size_t> read_{0};
};

} // namespace vstream
