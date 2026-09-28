#pragma once
#include <string>
#include <cstdint>

namespace vstream {

struct Session {
    std::string id;
    std::string url;
    std::uint16_t port = 45821;
};

Session createLocalSession(const std::string& hostAddress);

} // namespace vstream
