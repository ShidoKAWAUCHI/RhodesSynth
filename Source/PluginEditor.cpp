/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RhodesPluginSynthAudioProcessorEditor::RhodesPluginSynthAudioProcessorEditor(RhodesPluginSynthAudioProcessor& p)
    : AudioProcessorEditor(&p),
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
	targetx0(3.0),
	currenta1(10.0),
	targeta1(10.0),
	currenta2(0.01),
	targeta2(0.01)
{
    addAndMakeVisible(keyboardComponent);

	addAndMakeVisible(levelSlider);
	levelSlider.setRange(-100, -12);
	targetLevel = currentLevel;
	processor.changeLevel(targetLevel);
	levelSlider.onValueChange = [this] {
		targetLevel = juce::Decibels::decibelsToGain((double)levelSlider.getValue());
		processor.changeLevel(targetLevel);
	};
	levelSlider.setValue(juce::Decibels::gainToDecibels(targetLevel), juce::dontSendNotification);
	
	/*
	levelSlider.setRange(0.0, 0.125);
	levelSlider.setValue((double)currentLevel, juce::dontSendNotification);
	targetLevel = currentLevel;
	processor.changeLevel(targetLevel);
	levelSlider.onValueChange = [this] {
		targetLevel = (float)levelSlider.getValue();
		processor.changeLevel(targetLevel);
	};*/
	

	levelLabel.setText("dB:", juce::dontSendNotification);
	addAndMakeVisible(levelLabel);

	addAndMakeVisible(A3FrequencySlider);
	A3FrequencySlider.setRange(428.0, 458.0,1);
	A3FrequencySlider.setValue(currentA3Frequency, juce::dontSendNotification);
	A3FrequencySlider.setSkewFactorFromMidPoint(440.0);
	A3FrequencySlider.onValueChange = [this] {
		targetA3Frequency = A3FrequencySlider.getValue();
		processor.changeA3Frequency(targetA3Frequency);
	};

	A3FrequencyLabel.setText("A3[Hz]:", juce::dontSendNotification);
	addAndMakeVisible(A3FrequencyLabel);


	addAndMakeVisible(cSlider);
	cSlider.setRange(0.00001, 0.001);
	cSlider.setValue(currentc, juce::dontSendNotification);
	cSlider.onValueChange = [this] {
		targetc = cSlider.getValue();
		processor.changec(targetc);
	};

	cLabel.setText("C:", juce::dontSendNotification);
	addAndMakeVisible(cLabel);

	addAndMakeVisible(kSlider);
	kSlider.setRange(0.0, 100.0);
	kSlider.setValue(currentk, juce::dontSendNotification);
	kSlider.onValueChange = [this] {
		targetk = kSlider.getValue();
		processor.changek(targetk);
	};

	kLabel.setText("k:", juce::dontSendNotification);
	addAndMakeVisible(kLabel);

	addAndMakeVisible(x0Slider);
	x0Slider.setRange(-10.0, 10.0);
	x0Slider.setValue(currentx0, juce::dontSendNotification);
	x0Slider.onValueChange = [this] {
		targetx0 = x0Slider.getValue();
		processor.changex0(targetx0);
	};

	x0Label.setText("x0:", juce::dontSendNotification);
	addAndMakeVisible(x0Label);

	addAndMakeVisible(a1Slider);
	a1Slider.setRange(1.0, 100.0);
	a1Slider.setValue(currenta1, juce::dontSendNotification);
	a1Slider.onValueChange = [this] {
		targeta1 = a1Slider.getValue();
		processor.changea1(targeta1);
	};

	a1Label.setText("a1:", juce::dontSendNotification);
	addAndMakeVisible(a1Label);

	addAndMakeVisible(a2Slider);
	a2Slider.setRange(0.001, 1.0);
	a2Slider.setValue(currenta2, juce::dontSendNotification);
	a2Slider.onValueChange = [this] {
		targeta2 = a2Slider.getValue();
		processor.changea2(targeta2);
	};

	a2Label.setText("a2:", juce::dontSendNotification);
	addAndMakeVisible(a2Label);
	addAndMakeVisible(keyboardComponent);

    setSize(600, 450);
}
	

void RhodesPluginSynthAudioProcessorEditor::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
	currentLevel = targetLevel;
	currentA3Frequency = targetA3Frequency;
	currentc = targetc;
	currentk = targetk;
	currentx0 = targetx0;
	currenta1 = targeta1;
	currenta2 = targeta2;
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
	levelLabel.setBounds(10, 10, 60, 20);
	levelSlider.setBounds(70, 10, getWidth() - 70, 20);
	A3FrequencyLabel.setBounds(10, 40, 60, 20);
	A3FrequencySlider.setBounds(70, 40, getWidth() - 70, 20);
	cLabel.setBounds(10, 70, 60, 20);
	cSlider.setBounds(70, 70, getWidth() - 70, 20);
	kLabel.setBounds(10, 100, 60, 20);
	kSlider.setBounds(70, 100, getWidth() - 70, 20);
	x0Label.setBounds(10, 130, 60, 20);
	x0Slider.setBounds(70, 130, getWidth() - 70, 20);
	a1Label.setBounds(10, 160, 60, 20);
	a1Slider.setBounds(70, 160, getWidth() - 70, 20);
	a2Label.setBounds(10, 190, 60, 20);
	a2Slider.setBounds(70, 190, getWidth() - 70, 20);
	//keyboardComponent.setBounds(10, 220, getWidth() - 20, getHeight());
	keyboardComponent.setBoundsRelative(0, 0.5, 1.2, 0.5);
}