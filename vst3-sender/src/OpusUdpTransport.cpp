#include "vstream/OpusUdpTransport.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace vstream {

namespace {
constexpr int kChannels = 2;
constexpr int kOutputSampleRate = 48000;
constexpr int kFrameSize = 960;
constexpr std::uint16_t kAudioPort = 45822;
constexpr const char* kAudioMulticast = "239.255.77.77";
constexpr std::uint32_t kMagic = 0x5653544Du; // "VSTM"

void putU16(std::vector<std::uint8_t>& out, std::size_t& p, std::uint16_t v)
{
    out[p++] = static_cast<std::uint8_t>((v >> 8) & 0xff);
    out[p++] = static_cast<std::uint8_t>(v & 0xff);
}

void putU32(std::vector<std::uint8_t>& out, std::size_t& p, std::uint32_t v)
{
    out[p++] = static_cast<std::uint8_t>((v >> 24) & 0xff);
    out[p++] = static_cast<std::uint8_t>((v >> 16) & 0xff);
    out[p++] = static_cast<std::uint8_t>((v >> 8) & 0xff);
    out[p++] = static_cast<std::uint8_t>(v & 0xff);
}
}

OpusUdpTransport::OpusUdpTransport(AudioRingBuffer& ring)
    : juce::Thread("VSTream Opus UDP"),
      ring_(ring),
      sourceFifo_(),
      inputScratch_(4096 * kChannels),
      outputFrame_(kFrameSize * kChannels),
      opusPacket_(4000),
      networkPacket_(4096)
{
}

OpusUdpTransport::~OpusUdpTransport()
{
    stop();
}

bool OpusUdpTransport::start(double inputSampleRate, const std::string& sessionId)
{
    stop();

    if (inputSampleRate <= 0.0 || sessionId.empty())
        return false;

    inputSampleRate_ = inputSampleRate;
    sourcePosition_ = 0.0;
    sessionId_ = sessionId;
    sourceFifo_.clear();
    packetsSent_.store(0, std::memory_order_release);

    if (!encoder_.open(kOutputSampleRate, kChannels, 128000))
        return false;

    if (!udp_.open(kAudioMulticast, kAudioPort))
        return false;

    running_.store(true, std::memory_order_release);
    if (!startThread())
    {
        running_.store(false, std::memory_order_release);
        udp_.close();
        return false;
    }

    return true;
}

void OpusUdpTransport::stop()
{
    running_.store(false, std::memory_order_release);
    signalThreadShouldExit();
    stopThread(1500);
    udp_.close();
    sourceFifo_.clear();
}

bool OpusUdpTransport::buildPacket(const std::vector<std::uint8_t>& opusPayload,
                                   std::uint32_t sequence,
                                   std::vector<std::uint8_t>& packet) const
{
    // Header: magic(4), version(2), channels(1), flags(1), sequence(4),
    // sampleRate(4), frames(2), payloadBytes(2), sessionId(8) = 28 bytes.
    constexpr std::size_t headerSize = 28;
    if (opusPayload.size() > 65535 || opusPayload.size() + headerSize > packet.max_size())
        return false;

    packet.resize(headerSize + opusPayload.size());
    std::size_t p = 0;
    putU32(packet, p, kMagic);
    putU16(packet, p, 1);
    packet[p++] = kChannels;
    packet[p++] = 0;
    putU32(packet, p, sequence);
    putU32(packet, p, kOutputSampleRate);
    putU16(packet, p, kFrameSize);
    putU16(packet, p, static_cast<std::uint16_t>(opusPayload.size()));

    std::string id = sessionId_;
    if (id.size() > 8) id.resize(8);
    id.resize(8, '0');
    std::memcpy(packet.data() + p, id.data(), 8);
    p += 8;

    std::memcpy(packet.data() + p, opusPayload.data(), opusPayload.size());
    return true;
}

void OpusUdpTransport::run()
{
    std::uint32_t sequence = 0;
    const double ratio = inputSampleRate_ / static_cast<double>(kOutputSampleRate);

    while (!threadShouldExit() && running_.load(std::memory_order_acquire))
    {
        const std::size_t availableFrames = sourceFifo_.size() / kChannels;
        const std::size_t requiredFrames =
            static_cast<std::size_t>(std::ceil(sourcePosition_ + ratio * kFrameSize + 2.0));

        if (availableFrames < requiredFrames)
        {
            const auto toRead = std::min<std::size_t>(4096, requiredFrames - availableFrames);
            inputScratch_.resize(toRead * kChannels);
            const auto popped = ring_.pop(inputScratch_.data(), toRead, kChannels);
            if (popped > 0)
            {
                sourceFifo_.insert(sourceFifo_.end(),
                                   inputScratch_.begin(),
                                   inputScratch_.begin() + static_cast<std::ptrdiff_t>(popped * kChannels));
                continue;
            }

            wait(2.0);
            continue;
        }

        for (int outFrame = 0; outFrame < kFrameSize; ++outFrame)
        {
            const double pos = sourcePosition_ + static_cast<double>(outFrame) * ratio;
            const auto i0 = static_cast<std::size_t>(pos);
            const auto i1 = std::min(i0 + 1, availableFrames - 1);
            const float frac = static_cast<float>(pos - static_cast<double>(i0));

            for (int ch = 0; ch < kChannels; ++ch)
            {
                const float a = sourceFifo_[i0 * kChannels + ch];
                const float b = sourceFifo_[i1 * kChannels + ch];
                outputFrame_[outFrame * kChannels + ch] = a + (b - a) * frac;
            }
        }

        const auto consumedFrames = static_cast<std::size_t>(sourcePosition_ + ratio * kFrameSize);
        sourcePosition_ += ratio * kFrameSize - static_cast<double>(consumedFrames);

        const auto consumedSamples = consumedFrames * kChannels;
        sourceFifo_.erase(sourceFifo_.begin(),
                          sourceFifo_.begin() + static_cast<std::ptrdiff_t>(consumedSamples));

        if (!encoder_.encode(outputFrame_.data(), kFrameSize, opusPacket_))
            continue;

        if (!buildPacket(opusPacket_, sequence++, networkPacket_))
            continue;

        if (udp_.send(networkPacket_.data(), networkPacket_.size()))
            packetsSent_.fetch_add(1, std::memory_order_relaxed);
    }
}

} // namespace vstream
