/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

RhodesPluginSynthAudioProcessorEditor::RhodesPluginSynthAudioProcessorEditor(RhodesPluginSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
    , keyboardComponent(p.getMidiKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    AButton.setButtonText("TestA");
    AButton.addListener(this);
    BButton.setButtonText("TestB");
    BButton.addListener(this);

    addAndMakeVisible(AButton);
    addAndMakeVisible(BButton);
    addAndMakeVisible(keyboardComponent);
    setSize(800, 600);
}


RhodesPluginSynthAudioProcessorEditor::~RhodesPluginSynthAudioProcessorEditor()
{
}

void RhodesPluginSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void RhodesPluginSynthAudioProcessorEditor::resized()
{
    AButton.setBoundsRelative(0.2, 0.2, 0.2, 0.2);
    BButton.setBoundsRelative(0.6, 0.2, 0.2, 0.2);
    keyboardComponent.setBoundsRelative(0, 0.7, 1.0, 0.3);
}

void RhodesPluginSynthAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &AButton) {
        f = 0;
    }
    else if (button == &BButton) {
        f = 1;
    }
}