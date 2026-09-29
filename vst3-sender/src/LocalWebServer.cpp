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
        client->read(request, static_cast<int>(sizeof(request) - 1), true);

        const auto html =
            "<!doctype html><html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
            "<title>VSTream</title></head><body style=\"font-family:sans-serif;text-align:center;padding:40px\">"
            "<h1>VSTream</h1><p>Session <b>" + sessionId_ + "</b> is active.</p>"
            "<p>VSTream sender is reachable on this local network.</p>"
            "<p>Audio receiver connection endpoint is ready for the session.</p></body></html>";

        const auto response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + juce::String(html.getNumBytesAsUTF8()) + "\r\n"
            "Connection: close\r\n\r\n" + html;

        client->write(response.toRawUTF8(), response.getNumBytesAsUTF8());
        delete client;
    }
}

} // namespace vstream
