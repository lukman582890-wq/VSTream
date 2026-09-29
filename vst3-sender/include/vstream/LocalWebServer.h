#pragma once
#include <JuceHeader.h>
#include <atomic>

namespace vstream {
class LocalWebServer final : private juce::Thread
{
public:
    LocalWebServer();
    ~LocalWebServer() override;

    bool start(std::uint16_t port, const juce::String& sessionId);
    void stop();
    bool isRunning() const noexcept { return running_.load(std::memory_order_acquire) && isThreadRunning(); }

private:
    void run() override;

    std::unique_ptr<juce::StreamingSocket> listener_;
    std::uint16_t port_ = 0;
    juce::String sessionId_;
    std::atomic<bool> running_{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LocalWebServer)
};
}
