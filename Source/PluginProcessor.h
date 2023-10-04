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
    void changeLevel(double targetLevel);
    void changeA3Frequency(double targetA3Frequency);
    void changec(double targetc);
    void changek(double targetk);
    void changex0(double targetx0);

    juce::MidiKeyboardState& getMidiKeyboardState() { return keyboardState; }

private:
    juce::Synthesiser synth;
    juce::MidiKeyboardState keyboardState;
    juce::MidiMessageCollector midiCollector;

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* dBParam = nullptr;
    std::atomic<float>* A3Param = nullptr;
    std::atomic<float>* cParam = nullptr;
    std::atomic<float>* kParam = nullptr;
    std::atomic<float>* x0Param = nullptr;

    double  level;
    double A3Frequency;
    double c;
    double k;
    double x0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhodesPluginSynthAudioProcessor)
};
