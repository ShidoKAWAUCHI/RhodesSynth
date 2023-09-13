/*
  ==============================================================================

    SynthVoice.h
    Created: 15 Jan 2023 12:57:42am
    Author:  1923031

  ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "SynthSound.h"

class RhodesWaveVoice : public juce::SynthesiserVoice
{
public:
    RhodesWaveVoice() ;
    ~RhodesWaveVoice() override {}

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int, int) override;
    void aftertouchChanged(int newAftertouchValue) override;
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    void setCurrentPlaybackSampleRate(double 	newRate) override;

private:
    static double constexpr A3Frequency = 440.0;

    double tailOff;
    double cx;
    double ss;
    double freq;
    double base_freq;
    double period_sec;
    double c;
    double k;

    double f1;
    double f2;

    double a1;
    double a2;

    double x0;  //initial position
    double A1;
    double A2;
    double A3;
    double A4; 
    double A5;
    double A6;

    

    double theta;
    double damp;

    double level;

    void pitchShiftPos(double pos);
    void clearNote();
    double renderNextSample();
};
