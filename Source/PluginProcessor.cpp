/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



RhodesPluginSynthAudioProcessor::RhodesPluginSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif

        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    for (auto i = 0; i < 128; ++i)
        synth.addVoice(new RhodesWaveVoice());

    synth.addSound(new RhodesWaveSound());
}

RhodesPluginSynthAudioProcessor::~RhodesPluginSynthAudioProcessor()
{
}

const juce::String RhodesPluginSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RhodesPluginSynthAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool RhodesPluginSynthAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool RhodesPluginSynthAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double RhodesPluginSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RhodesPluginSynthAudioProcessor::getNumPrograms()
{
    return 1;                
}

int RhodesPluginSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RhodesPluginSynthAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RhodesPluginSynthAudioProcessor::getProgramName(int index)
{
    return {};
}

void RhodesPluginSynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void RhodesPluginSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    keyboardState.reset();
}

void RhodesPluginSynthAudioProcessor::releaseResources()
{
    keyboardState.allNotesOff(0);
    keyboardState.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RhodesPluginSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void RhodesPluginSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

}

bool RhodesPluginSynthAudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* RhodesPluginSynthAudioProcessor::createEditor()
{
    return new RhodesPluginSynthAudioProcessorEditor(*this);
}

void RhodesPluginSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void RhodesPluginSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhodesPluginSynthAudioProcessor();
}
