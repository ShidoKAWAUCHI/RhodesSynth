/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RhodesPluginSynthAudioProcessorEditor::RhodesPluginSynthAudioProcessorEditor(RhodesPluginSynthAudioProcessor& p,juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p),
	valueTreeState(vts),
	processor(p),
	keyboardComponent(p.getMidiKeyboardState(),
	juce::MidiKeyboardComponent::horizontalKeyboard),
	currentLevel(0.1),
	targetLevel(0.1),
	currentA3Frequency(440.0),
	targetA3Frequency(440.0),
	currentc(0.000050),
	targetc(0.000050),
	currentk(20.0),
	targetk(20.0),
	currentx0(3.0),
	targetx0(3.0)
{
    addAndMakeVisible(keyboardComponent);

	addAndMakeVisible(levelSlider);
	levelSlider.setRange(-60, 0);
	targetLevel = currentLevel;
	processor.changeLevel(targetLevel);
	levelSlider.onValueChange = [this] {
		targetLevel = juce::Decibels::decibelsToGain((double)levelSlider.getValue());
		processor.changeLevel(targetLevel);
	};
	levelSlider.setValue(juce::Decibels::gainToDecibels(targetLevel), juce::dontSendNotification);
	levelLabel.setText("dB", juce::dontSendNotification);
	levelSlider.setSliderStyle(juce::Slider::Rotary);
	levelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
	addAndMakeVisible(levelLabel);
	dBAttachment.reset(new SliderAttachment(valueTreeState, "dB:", levelSlider));

	addAndMakeVisible(A3FrequencySlider);
	A3FrequencySlider.setRange(428.0, 458.0,1);
	A3FrequencySlider.setValue(currentA3Frequency, juce::dontSendNotification);
	A3FrequencySlider.setSkewFactorFromMidPoint(440.0);
	A3FrequencySlider.onValueChange = [this] {
		targetA3Frequency = A3FrequencySlider.getValue();
		processor.changeA3Frequency(targetA3Frequency);
	};
	A3FrequencyLabel.setText("A3[Hz]", juce::dontSendNotification);
	A3FrequencySlider.setSliderStyle(juce::Slider::Rotary);
	A3FrequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
	addAndMakeVisible(A3FrequencyLabel);
	A3Attachment.reset(new SliderAttachment(valueTreeState, "a3:", A3FrequencySlider));

	addAndMakeVisible(cSlider);
	cSlider.setRange(0.00001, 0.001);
	cSlider.setValue(currentc, juce::dontSendNotification);
	cSlider.onValueChange = [this] {
		targetc = cSlider.getValue();
		processor.changec(targetc);
	};

	cLabel.setText("C", juce::dontSendNotification);
	cSlider.setSliderStyle(juce::Slider::Rotary);
	cSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
	addAndMakeVisible(cLabel);
	cAttachment.reset(new SliderAttachment(valueTreeState, "c:", cSlider));

	addAndMakeVisible(kSlider);
	kSlider.setRange(0.0, 100.0);
	kSlider.setValue(currentk, juce::dontSendNotification);
	kSlider.onValueChange = [this] {
		targetk = kSlider.getValue();
		processor.changek(targetk);
	};
	kLabel.setText("K", juce::dontSendNotification);
	kSlider.setSliderStyle(juce::Slider::Rotary);
	kSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
	addAndMakeVisible(kLabel);
	kAttachment.reset(new SliderAttachment(valueTreeState, "k:", kSlider));

	addAndMakeVisible(x0Slider);
	x0Slider.setRange(-10.0, 10.0);
	x0Slider.setValue(currentx0, juce::dontSendNotification);
	x0Slider.onValueChange = [this] {
		targetx0 = x0Slider.getValue();
		processor.changex0(targetx0);
	};
	x0Label.setText("x0", juce::dontSendNotification);
	x0Slider.setSliderStyle(juce::Slider::Rotary);
	x0Slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
	addAndMakeVisible(x0Label);
	x0Attachment.reset(new SliderAttachment(valueTreeState, "x0:", x0Slider));

    setSize(600, 350);
}
	

void RhodesPluginSynthAudioProcessorEditor::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	currentLevel = targetLevel;
	currentA3Frequency = targetA3Frequency;
	currentc = targetc;
	currentk = targetk;
	currentx0 = targetx0;
	processor.getNextAudioBlock(bufferToFill);
}

RhodesPluginSynthAudioProcessorEditor::~RhodesPluginSynthAudioProcessorEditor()
{
}

//==============================================================================
void RhodesPluginSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}


void RhodesPluginSynthAudioProcessorEditor::resized()
{
	levelLabel.setBounds(38, 0, 60, 30);
	levelSlider.setBounds(0,20,100,100);
	A3FrequencyLabel.setBounds(96, 0, 60, 30);
	A3FrequencySlider.setBounds(70, 20, 100, 100);
	cLabel.setBounds(180, 0, 60, 30);
	cSlider.setBounds(140, 20, 100, 100);
	kLabel.setBounds(250, 0, 60, 30);
	kSlider.setBounds(210, 20,100, 100);
	x0Label.setBounds(318, 0, 60, 30);
	x0Slider.setBounds(280, 20, 100, 100);
	keyboardComponent.setBoundsRelative(0, 0.45, 1.2, 0.55);
}