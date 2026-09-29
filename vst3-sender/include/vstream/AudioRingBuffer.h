#pragma once
#include <vector>
#include <atomic>
#include <cstddef>

namespace vstream {

class AudioRingBuffer {
public:
    explicit AudioRingBuffer(std::size_t capacitySamples)
        : buffer_(capacitySamples), capacity_(capacitySamples) {}

    bool pushInterleaved(const float* const* channels,
                         int numChannels,
                         std::size_t frames) noexcept;
    std::size_t pop(float* destinationInterleaved,
                    std::size_t maxFrames,
                    int numChannels) noexcept;
    std::size_t availableFrames(int numChannels) const noexcept;

private:
    std::vector<float> buffer_;
    const std::size_t capacity_;
    std::atomic<std::size_t> write_{0};
    std::atomic<std::size_t> read_{0};
};

} // namespace vstream
