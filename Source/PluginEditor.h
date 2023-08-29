/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RhodesPluginSynthAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Button::Listener
{
public:
    RhodesPluginSynthAudioProcessorEditor(RhodesPluginSynthAudioProcessor&);
    ~RhodesPluginSynthAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    juce::MidiKeyboardComponent keyboardComponent;

    void buttonClicked(juce::Button* button) override;
    juce::TextButton AButton;
    juce::TextButton BButton;
    int f;
    RhodesPluginSynthAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhodesPluginSynthAudioProcessorEditor)
};
