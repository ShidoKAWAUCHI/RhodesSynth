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
    RhodesWaveVoice(double targetLevel,double targetA3Freqency, double targetc, double targetk, double targetx0, double targeta1, double targeta2) ;
    ~RhodesWaveVoice() override {}

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int, int) override {}
    void aftertouchChanged(int newAftertouchValue) override;
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    void setCurrentPlaybackSampleRate(double 	newRate) override;
    void setLevel(double level);
    void setA3frequency(double A3Frequency);
    void setc(double c);
    void setk(double k);
    void setx0(double x0);
    void seta1(double a1);
    void seta2(double a2);

private:

    double A3Frequency;
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
    double attack;
    double decay;


    void pitchShiftPos(double pos);
    void clearNote();
};

inline void RhodesWaveVoice::setLevel(double l)
{
    level = l;
}

inline void RhodesWaveVoice::setA3frequency(double A)
{
    A3Frequency = A;
}

inline void RhodesWaveVoice::setc(double C)
{
    c = C;
}

inline void RhodesWaveVoice::setk(double K)
{
    k = K;
}

inline void RhodesWaveVoice::setx0(double x)
{
    x0 = x;
}

inline void RhodesWaveVoice::seta1(double A1)
{
    a1 = A1;
}

inline void RhodesWaveVoice::seta2(double A2)
{
    a2 = A2;
}