#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace vstream {
class UdpSender {
public:
    UdpSender();
    ~UdpSender();
    bool open(const std::string& host, std::uint16_t port);
    void close();
    bool isOpen() const noexcept;
    bool send(const std::uint8_t* data, std::size_t size);
private:
    struct Impl;
    Impl* impl_;
};
}
