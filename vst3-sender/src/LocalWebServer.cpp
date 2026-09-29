#include "vstream/LocalWebServer.h"

namespace vstream {

LocalWebServer::LocalWebServer() : juce::Thread("VSTream HTTP Server") {}

LocalWebServer::~LocalWebServer() { stop(); }

bool LocalWebServer::start(std::uint16_t port, const juce::String& sessionId)
{
    stop();
    auto socket = std::make_unique<juce::StreamingSocket>();
    if (!socket->createListener(port))
        return false;

    port_ = port;
    sessionId_ = sessionId;
    listener_ = std::move(socket);
    running_.store(true, std::memory_order_release);
    startThread();
    return true;
}

void LocalWebServer::stop()
{
    running_.store(false, std::memory_order_release);
    if (listener_)
        listener_->close();
    stopThread(1000);
    listener_.reset();
}

void LocalWebServer::run()
{
    while (!threadShouldExit() && running_.load(std::memory_order_acquire))
    {
        auto* client = listener_ != nullptr ? listener_->waitForNextConnection() : nullptr;
        if (client == nullptr)
            continue;

        char request[4096] = {};
        if (client->waitUntilReady(true, 1000) > 0)
            client->read(request, static_cast<int>(sizeof(request) - 1), false);

        const auto requestText = juce::String::fromUTF8(request);
        const auto firstLine = requestText.upToFirstOccurrenceOf("\r\n", false, false);
        const auto pathStart = firstLine.indexOfChar(' ') + 1;
        const auto pathEnd = pathStart > 0 ? firstLine.indexOfChar(pathStart, ' ') : -1;
        const auto path = (pathStart > 0 && pathEnd > pathStart)
            ? firstLine.substring(pathStart, pathEnd)
            : juce::String("/");

        const bool validSessionPath = path == "/s/" + sessionId_ || path == "/health";
        const auto status = validSessionPath ? "200 OK" : "404 Not Found";
        const auto appUrl = "vstream://connect?url=" + juce::URL::addEscapeChars(
            "http://" + juce::IPAddress::getLocalAddress(false).toString() + ":" +
            juce::String(port_) + "/s/" + sessionId_, true);

        const auto body =
            validSessionPath
                ? "<!doctype html><html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                  "<title>VSTream</title></head><body style=\"font-family:sans-serif;text-align:center;padding:40px\">"
                  "<h1>VSTream</h1><p>Session <b>" + sessionId_ + "</b> is active.</p>"
                  "<p>Sender is reachable on the local network.</p>"
                  "<p>Transport: Opus UDP multicast is active.</p>"
                  "<p><a href=\"" + appUrl + "\">Open VSTream Receiver</a></p>"
                  "</body></html>"
                : "<!doctype html><html><body><h1>404</h1></body></html>";

        const auto response =
            "HTTP/1.1 " + juce::String(status) + "\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + juce::String(body.getNumBytesAsUTF8()) + "\r\n"
            "Connection: close\r\n\r\n" + body;

        client->write(response.toRawUTF8(), response.getNumBytesAsUTF8());
        delete client;
    }
}

} // namespace vstream
