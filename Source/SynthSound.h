/*
  ==============================================================================

    SynthSound.h
    Created: 15 Jan 2023 12:58:55am
    Author:  1923031

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class RhodesWaveSound : public juce::SynthesiserSound
{
public:
    RhodesWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};
