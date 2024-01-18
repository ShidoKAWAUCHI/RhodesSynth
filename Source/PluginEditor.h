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
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
private:

    double currentLevel, targetLevel;
    double currentA3Frequency, targetA3Frequency;
    double currentc, targetc;
    double currentk, targetk;
    double currentx0, targetx0;

    juce::MidiKeyboardComponent keyboardComponent;

    RhodesPluginSynthAudioProcessor& processor;
    juce::Synthesiser synth;
    juce::Slider levelSlider, A3FrequencySlider, cSlider, kSlider, x0Slider;
    juce::Label levelLabel, A3FrequencyLabel, cLabel, kLabel, x0Label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RhodesPluginSynthAudioProcessorEditor)
};