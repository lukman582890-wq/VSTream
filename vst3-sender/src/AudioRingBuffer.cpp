#include "vstream/AudioRingBuffer.h"
#include <algorithm>

namespace vstream {

bool AudioRingBuffer::pushInterleaved(const float* const* channels,
                                      int numChannels,
                                      std::size_t frames) noexcept
{
    if (channels == nullptr || numChannels <= 0 || numChannels > 2 || frames == 0)
        return false;

    const auto samples = frames * static_cast<std::size_t>(numChannels);
    if (samples > capacity_ - (write_.load(std::memory_order_acquire)
                               - read_.load(std::memory_order_acquire)))
        return false;

    auto w = write_.load(std::memory_order_relaxed);
    for (std::size_t frame = 0; frame < frames; ++frame)
        for (int ch = 0; ch < numChannels; ++ch)
            buffer_[(w + frame * static_cast<std::size_t>(numChannels) + static_cast<std::size_t>(ch)) % capacity_]
                = channels[ch][frame];

    write_.store(w + samples, std::memory_order_release);
    return true;
}

std::size_t AudioRingBuffer::pop(float* destinationInterleaved,
                                 std::size_t maxFrames,
                                 int numChannels) noexcept
{
    if (destinationInterleaved == nullptr || numChannels <= 0 || numChannels > 2)
        return 0;

    const auto availableSamples = write_.load(std::memory_order_acquire)
                                 - read_.load(std::memory_order_acquire);
    const auto availableFrames = availableSamples / static_cast<std::size_t>(numChannels);
    const auto frames = std::min(maxFrames, availableFrames);
    auto r = read_.load(std::memory_order_relaxed);

    for (std::size_t i = 0; i < frames * static_cast<std::size_t>(numChannels); ++i)
        destinationInterleaved[i] = buffer_[(r + i) % capacity_];

    read_.store(r + frames * static_cast<std::size_t>(numChannels), std::memory_order_release);
    return frames;
}

std::size_t AudioRingBuffer::availableFrames(int numChannels) const noexcept
{
    if (numChannels <= 0)
        return 0;
    return (write_.load(std::memory_order_acquire)
          - read_.load(std::memory_order_acquire))
         / static_cast<std::size_t>(numChannels);
}

} // namespace vstream
