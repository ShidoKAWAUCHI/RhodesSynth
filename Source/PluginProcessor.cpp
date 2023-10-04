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
	,
	level(0.1),
	A3Frequency(440.0),
	c(0.000050),
	k(20.0),
	x0(3.0),
	parameters(*this, nullptr, juce::Identifier("APVTSTutorial"),
		{
			std::make_unique<juce::AudioParameterFloat>("dB:",  // ID
														"DB:",  // name
														juce::NormalisableRange<float>(-60.0f,0.0f),
														 -20.0f,
														 "feedback",
														  juce::AudioProcessorParameter::genericParameter,
														  [](float value, int) {return juce::String(value, 2) ; }
														),// default
			std::make_unique<juce::AudioParameterFloat>("a3:",  // ID
														"A3:",  // name
														juce::NormalisableRange<float>(428.0f,458.0f),
														 440.0f,
														 "feedback",
														  juce::AudioProcessorParameter::genericParameter,
														  [](float value, int) {return juce::String(value, 0); }),// default
			std::make_unique<juce::AudioParameterFloat>("c:",  // ID
														"C:",  // name
														juce::NormalisableRange<float>(0.00001f, 0.001f),
														 0.00005f,
														 "feedback",
														  juce::AudioProcessorParameter::genericParameter,
														  [](float value, int) {return juce::String(value, 5); }
														 ),// default
			std::make_unique<juce::AudioParameterFloat>("k:",  // ID
														"K:",  // name
														juce::NormalisableRange<float>(0.0f, 100.0f),
														 20.0f,
														 "feedback",
														  juce::AudioProcessorParameter::genericParameter,
														  [](float value, int) {return juce::String(value, 2); }
														),// default
			std::make_unique<juce::AudioParameterFloat>("x0:",  // ID
														"X0:",  // name
														juce::NormalisableRange<float>(-10.0f, 10.0f),
														 3.0f,
														 "feedback",
														  juce::AudioProcessorParameter::genericParameter,
														  [](float value, int) {return juce::String(value, 1); }
				)// default
		})
{
	dBParam = parameters.getRawParameterValue("dB:");
	A3Param = parameters.getRawParameterValue("a3:");
	cParam = parameters.getRawParameterValue("c:");
	kParam = parameters.getRawParameterValue("k:");
	x0Param = parameters.getRawParameterValue("x0:");
	for (auto i = 0; i < 128; ++i)
		synth.addVoice(new RhodesWaveVoice(level, A3Frequency, c, k, x0));
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

void RhodesPluginSynthAudioProcessor::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	bufferToFill.clearActiveBufferRegion();
	juce::MidiBuffer incomingMidi;
	midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
	keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);
	synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
}

void RhodesPluginSynthAudioProcessor::releaseResources()
{
	keyboardState.allNotesOff(0);
	keyboardState.reset();
}

void RhodesPluginSynthAudioProcessor::changeLevel(double targetLevel)
{
	level = targetLevel;

	for (int i = 0; i < synth.getNumVoices(); ++i)
		static_cast<RhodesWaveVoice*> (synth.getVoice(i))->setLevel(level);
}

void RhodesPluginSynthAudioProcessor::changeA3Frequency(double targetA3Frequency)
{
	A3Frequency = targetA3Frequency;
	for (int i = 0; i < synth.getNumVoices(); ++i)
		static_cast<RhodesWaveVoice*> (synth.getVoice(i))->setA3frequency(A3Frequency);
}

void RhodesPluginSynthAudioProcessor::changec(double targetc)
{
	c = targetc;
	for (int i = 0; i < synth.getNumVoices(); ++i)
		static_cast<RhodesWaveVoice*> (synth.getVoice(i))->setc(c);
}

void RhodesPluginSynthAudioProcessor::changek(double targetk)
{
	k = targetk;
	for (int i = 0; i < synth.getNumVoices(); ++i)
		static_cast<RhodesWaveVoice*> (synth.getVoice(i))->setk(k);
}


void RhodesPluginSynthAudioProcessor::changex0(double targetx0)
{
	x0 = targetx0;
	for (int i = 0; i < synth.getNumVoices(); ++i)
		static_cast<RhodesWaveVoice*> (synth.getVoice(i))->setx0(x0);
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
	return new RhodesPluginSynthAudioProcessorEditor(*this, parameters);
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
