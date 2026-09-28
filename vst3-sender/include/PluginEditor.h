#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VSTreamAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VSTreamAudioProcessorEditor(VSTreamAudioProcessor&);
    ~VSTreamAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void updateStatus();
    VSTreamAudioProcessor& processor;
    juce::TextButton streamButton{"START STREAM"};
    juce::TextButton copyButton{"COPY LINK"};
    juce::Label statusLabel, linkLabel, meterLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VSTreamAudioProcessorEditor)
};