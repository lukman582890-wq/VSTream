#include "vstream/Session.h"
#include <JuceHeader.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace vstream {

static std::string getLocalHostAddress()
{
    for (const auto& address : juce::IPAddress::getLocalAddresses())
    {
        const auto text = address.toString();
        if (text != "127.0.0.1" && !text.startsWith("169.254."))
            return text.toStdString();
    }
    return "127.0.0.1";
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
