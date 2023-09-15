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
    void setA3frequency(double a3f);
    void setc(double c);
    void setk(double k);
    void setx0(double x0);
    void seta1(double a1);
    void seta2(double a2);

private:

    double a3f_;
    double A3Frequency_;
    double tailOff_;
    double cx_;
    double ss_;
    double freq_;
    double base_freq_;
    double targetFreq_;
    double period_sec_;
    double c_;
    double k_;
    double alpha_;

    double f1_;
    double f2_;

    double a1_;
    double a2_;

    double x0_;  //initial position
    double A1_;
    double A2_;
    double A3_;
    double A4_; 
    double A5_;
    double A6_;

    double theta_;
    double damp_;

    double level_;
    double attack_;
    double decay_;


    void pitchShiftPos(double pos);
    void pitchShiftA3();
    void clearNote();
};

inline void RhodesWaveVoice::setLevel(double level)
{
    level_ = level;
}

inline void RhodesWaveVoice::setA3frequency(double A)
{
    freq_ *= A / a3f_;
    a3f_ = A;
}

inline void RhodesWaveVoice::setc(double C)
{
    c_ = C;
}

inline void RhodesWaveVoice::setk(double K)
{
    k_ = K;
}

inline void RhodesWaveVoice::setx0(double x)
{
    x0_ = x;
}

inline void RhodesWaveVoice::seta1(double A1)
{
    //a1_ = A1;
    alpha_ = A1;
}

inline void RhodesWaveVoice::seta2(double A2)
{
    a2_ = A2;
}