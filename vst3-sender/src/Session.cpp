#include "vstream/Session.h"
#include <JuceHeader.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace vstream {

static std::string getLocalHostAddress()
{
    const auto addresses = juce::IPAddress::getAllAddresses(false);
    for (const auto& address : addresses)
    {
        const auto text = address.toString();
        if (text == "127.0.0.1" || text.startsWith("169.254.") || text.isEmpty())
            continue;

        const bool privateV4 = text.startsWith("10.")
            || text.startsWith("192.168.")
            || text.startsWith("172.16.") || text.startsWith("172.17.")
            || text.startsWith("172.18.") || text.startsWith("172.19.")
            || text.startsWith("172.2") || text.startsWith("172.30.")
            || text.startsWith("172.31.");

        if (privateV4)
            return text.toStdString();
    }

    const auto fallback = juce::IPAddress::getLocalAddress(false).toString();
    return fallback.isEmpty() ? std::string("127.0.0.1") : fallback.toStdString();
}

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

    const auto host = hostAddress.empty() ? getLocalHostAddress() : hostAddress;
    session.url = "http://" + host + ":" + std::to_string(session.port) + "/s/" + session.id;
    return session;
}

} // namespace vstream
