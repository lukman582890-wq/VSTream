#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "vstream/LocalWebServer.h"

VSTreamAudioProcessor::VSTreamAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
   ringBuffer(std::make_unique<vstream::AudioRingBuffer>(96000))
{}

void VSTreamAudioProcessor::prepareToPlay(double sampleRate, int)
{
    ringBuffer = std::make_unique<vstream::AudioRingBuffer>(static_cast<std::size_t>(sampleRate * 2.0));
}
void VSTreamAudioProcessor::releaseResources() {}

bool VSTreamAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getChannelSet(true, 0);
    const auto out = layouts.getChannelSet(false, 0);
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo()) && in == out;
}

void VSTreamAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    float peak = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        peak = juce::jmax(peak, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    inputPeak.store(peak, std::memory_order_relaxed);

    if (streaming.load(std::memory_order_relaxed))
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            ringBuffer->push(buffer.getReadPointer(ch), static_cast<std::size_t>(buffer.getNumSamples()));
}

void VSTreamAudioProcessor::setStreaming(bool enabled)
{
    if (enabled && !streaming.load())
    {
        auto session = vstream::createLocalSession("");
        sessionId = session.id;
        streamUrl = session.url;

        if (!webServer)
            webServer = std::make_unique<vstream::LocalWebServer>();

        if (!webServer->start(session.port, sessionId))
        {
            streamUrl.clear();
            sessionId.clear();
            return;
        }
    }
    else if (!enabled && webServer)
    {
        webServer->stop();
    }

    streaming.store(enabled && webServer != nullptr && webServer->isThreadRunning(),
                    std::memory_order_release);
}

juce::AudioProcessorEditor* VSTreamAudioProcessor::createEditor()
{
    return new VSTreamAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VSTreamAudioProcessor();
}
