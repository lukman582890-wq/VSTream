#include "vstream/Session.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace vstream {

Session createLocalSession(const std::string& hostAddress)
{
    static std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::uint32_t> dist(0, 0xFFFFFFFFu);

    const auto value = dist(rng);
    std::ostringstream id;
    id << std::hex << std::setw(8) << std::setfill('0') << value;

    Session session;
    session.id = id.str();
    session.port = 45821;
    session.url = "https://vstream.local/s/" + session.id;
    return session;
}

} // namespace vstream
