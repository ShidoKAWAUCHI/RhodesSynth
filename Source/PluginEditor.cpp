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
    addAndMakeVisible(keyboardComponent);
    setSize(600, 300);
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
    keyboardComponent.setBoundsRelative(0, 0, 1.0, 1.0);
}