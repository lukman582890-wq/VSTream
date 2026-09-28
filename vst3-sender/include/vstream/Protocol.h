#pragma once

#include <cstdint>

namespace vstream {

constexpr std::uint16_t kDefaultPort = 45821;
constexpr std::uint32_t kProtocolVersion = 1;

struct StreamConfig {
    std::uint16_t port = kDefaultPort;
    std::uint32_t sampleRate = 48000;
    std::uint32_t channels = 2;
    std::uint32_t bitrate = 192000;
};

} // namespace vstream
