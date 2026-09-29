#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "vstream/AudioRingBuffer.h"
#include "vstream/OpusEncoder.h"
#include "vstream/UdpSender.h"

namespace vstream {

class OpusUdpTransport final : private juce::Thread
{
public:
    explicit OpusUdpTransport(AudioRingBuffer& ring);
    ~OpusUdpTransport() override;

    bool start(double inputSampleRate, const std::string& sessionId);
    void stop();
    bool isRunning() const noexcept { return running_.load(std::memory_order_acquire); }
    std::uint64_t packetsSent() const noexcept { return packetsSent_.load(std::memory_order_relaxed); }

private:
    void run() override;
    bool buildPacket(const std::vector<std::uint8_t>& opusPayload,
                     std::uint32_t sequence,
                     std::vector<std::uint8_t>& packet) const;

    AudioRingBuffer& ring_;
    OpusEncoder encoder_;
    UdpSender udp_;
    double inputSampleRate_ = 48000.0;
    double sourcePosition_ = 0.0;
    std::string sessionId_;
    std::vector<float> sourceFifo_;
    std::vector<float> inputScratch_;
    std::vector<float> outputFrame_;
    std::vector<std::uint8_t> opusPacket_;
    std::vector<std::uint8_t> networkPacket_;
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> packetsSent_{0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpusUdpTransport)
};

} // namespace vstream
