/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class RhodesPluginSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    RhodesPluginSynthAudioProcessorEditor(RhodesPluginSynthAudioProcessor&);
    ~RhodesPluginSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:

    juce::MidiKeyboardComponent keyboardComponent;

    RhodesPluginSynthAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhodesPluginSynthAudioProcessorEditor)
};