#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "vstream/LocalWebServer.h"

VSTreamAudioProcessor::VSTreamAudioProcessor()
 : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
   ringBuffer(std::make_unique<vstream::AudioRingBuffer>(192000))
{
}

void VSTreamAudioProcessor::prepareToPlay(double newSampleRate, int)
{
    sampleRate = newSampleRate;
    ringBuffer = std::make_unique<vstream::AudioRingBuffer>(
        static_cast<std::size_t>(juce::jmax(96000.0, newSampleRate * 4.0)));
}

void VSTreamAudioProcessor::releaseResources()
{
    if (transport)
        transport->stop();
}

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

    if (streaming.load(std::memory_order_relaxed) && ringBuffer != nullptr)
    {
        const int inputChannels = buffer.getNumChannels();
        if (inputChannels >= 2)
        {
            const float* channels[2] = {
                buffer.getReadPointer(0),
                buffer.getReadPointer(1)
            };
            ringBuffer->pushInterleaved(channels, 2,
                                        static_cast<std::size_t>(buffer.getNumSamples()));
        }
        else if (inputChannels == 1)
        {
            const float* channels[2] = {
                buffer.getReadPointer(0),
                buffer.getReadPointer(0)
            };
            ringBuffer->pushInterleaved(channels, 2,
                                        static_cast<std::size_t>(buffer.getNumSamples()));
        }
    }
}

void VSTreamAudioProcessor::setStreaming(bool enabled)
{
    if (enabled && !streaming.load(std::memory_order_acquire))
    {
        auto session = vstream::createLocalSession("");
        sessionId = session.id;
        streamUrl = session.url;

        if (!webServer)
            webServer = std::make_unique<vstream::LocalWebServer>();
        if (!transport)
            transport = std::make_unique<vstream::OpusUdpTransport>(*ringBuffer);

        if (!webServer->start(session.port, sessionId))
        {
            streamUrl.clear();
            sessionId.clear();
            streaming.store(false, std::memory_order_release);
            return;
        }

        if (!transport->start(sampleRate, sessionId.toStdString()))
        {
            webServer->stop();
            streamUrl.clear();
            sessionId.clear();
            streaming.store(false, std::memory_order_release);
            return;
        }

        streaming.store(true, std::memory_order_release);
    }
    else if (!enabled)
    {
        streaming.store(false, std::memory_order_release);
        if (transport)
            transport->stop();
        if (webServer)
            webServer->stop();
    }
}

juce::AudioProcessorEditor* VSTreamAudioProcessor::createEditor()
{
    return new VSTreamAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VSTreamAudioProcessor();
}
