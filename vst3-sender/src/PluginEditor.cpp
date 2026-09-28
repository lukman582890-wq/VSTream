#include "PluginEditor.h"

VSTreamAudioProcessorEditor::VSTreamAudioProcessorEditor(VSTreamAudioProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    setSize(480, 260);
    addAndMakeVisible(streamButton);
    addAndMakeVisible(copyButton);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(linkLabel);
    addAndMakeVisible(meterLabel);

    streamButton.onClick = [this] { processor.setStreaming(!processor.isStreaming()); updateStatus(); };
    copyButton.onClick = [this] {
        if (processor.getStreamUrl().isNotEmpty())
            juce::SystemClipboard::copyTextToClipboard(processor.getStreamUrl());
    };
    for (auto* l : {&statusLabel, &linkLabel, &meterLabel})
        l->setJustificationType(juce::Justification::centred);
    startTimerHz(20);
    updateStatus();
}

void VSTreamAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(26.0f);
    g.drawFittedText("VSTream", 20, 18, getWidth()-40, 36, juce::Justification::centred, 1);
    g.setFont(14.0f);
    g.drawFittedText("Cubase → Android realtime audio", 20, 52, getWidth()-40, 24, juce::Justification::centred, 1);
}
void VSTreamAudioProcessorEditor::resized()
{
    streamButton.setBounds(35, 90, 180, 42);
    copyButton.setBounds(230, 90, 180, 42);
    statusLabel.setBounds(20, 145, getWidth()-40, 25);
    linkLabel.setBounds(20, 172, getWidth()-40, 25);
    meterLabel.setBounds(20, 205, getWidth()-40, 25);
}
void VSTreamAudioProcessorEditor::timerCallback() { updateStatus(); }
void void VSTreamAudioProcessorEditor::updateStatus()
{
    const bool active = processor.isStreaming();
    streamButton.setButtonText(active ? "STOP STREAM" : "START STREAM");
    statusLabel.setText(active ? "● STREAMING" : "○ STOPPED", juce::dontSendNotification);
    linkLabel.setText(active ? processor.getStreamUrl() : "Start a stream to generate a link",
                      juce::dontSendNotification);
    const auto peak = processor.getInputPeak();
    const auto db = juce::Decibels::gainToDecibels(juce::jmax(peak, 0.000001f));
    meterLabel.setText("INPUT " + juce::String(db, 1) + " dB", juce::dontSendNotification);
}
