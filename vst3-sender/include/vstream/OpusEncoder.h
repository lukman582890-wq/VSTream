#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
namespace vstream {
class OpusEncoder {
public:
    OpusEncoder();
    ~OpusEncoder();
    bool open(int sampleRate,int channels,int bitrate);
    bool encode(const float* interleaved,int frames,std::vector<std::uint8_t>& packet);
    bool isOpen()const noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
