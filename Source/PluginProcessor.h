/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthVoice.h"
#include "SynthSound.h"

class RhodesPluginSynthAudioProcessor  : public juce::AudioProcessor
{
public:

    RhodesPluginSynthAudioProcessor();
    ~RhodesPluginSynthAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) ;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void changeLevel(float targetLevel);
    void changeA3Frequency(double targetA3Frequency);
    void changec(float targetc);
    void changek(float targetk);
    void changex0(float targetx0);
    void changea1(float targeta1);
    void changea2(float targeta2);
   

    juce::MidiKeyboardState& getMidiKeyboardState() { return keyboardState; }

private:
    juce::Synthesiser synth;
    juce::MidiKeyboardState keyboardState;
    juce::MidiMessageCollector midiCollector;
    double  level = 0.1;
    double A3Frequency = 440.0;
    double c = 0.000050;
    double k = 20.0;
    double x0 = 3.0;
    double a1 = 10.0;
    double a2 = 0.01;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhodesPluginSynthAudioProcessor)
};
