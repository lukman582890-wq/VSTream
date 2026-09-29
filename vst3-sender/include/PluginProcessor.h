#pragma once
#include <JuceHeader.h>
#include "vstream/AudioRingBuffer.h"
#include "vstream/Session.h"
#include "vstream/LocalWebServer.h"

class VSTreamAudioProcessor : public juce::AudioProcessor
{
public:
    VSTreamAudioProcessor();
    ~VSTreamAudioProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "VSTream"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    void setStreaming(bool);
    bool isStreaming() const noexcept { return streaming.load(); }
    juce::String getStreamUrl() const { return streamUrl; }
    juce::String getSessionId() const { return sessionId; }
    float getInputPeak() const noexcept { return inputPeak.load(); }
private:
    std::unique_ptr<vstream::AudioRingBuffer> ringBuffer;
    std::unique_ptr<vstream::LocalWebServer> webServer;
    std::atomic<bool> streaming{false};
    std::atomic<float> inputPeak{0.0f};
    juce::String sessionId;
    juce::String streamUrl;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VSTreamAudioProcessor)
};
