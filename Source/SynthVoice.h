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
    RhodesWaveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved(int) override;
    void controllerMoved(int, int) override;
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    void test();
private:
    double currentAngle = 0.0, angleDelta = 0.0, tailOff = 0.0;
    double cx = 0;
    double ss = 0;
    int num = 0;
    double FREQ = 0;
    //double sample = 48000;
    double sample;
    double sample2 = 44000;
    double PERIOD_SEC=0;
    //double PERIOD_SEC = 1 / sample;
    double PERIOD_SEC2 = 1 / sample2;
    double V0 = 4000; 
    double c = 0.000050;
    double k = 20;

    double f1 = 8;
    double f2 = 4186;

    double a1 = 10;
    double a2 = 0.01;

    double x0 = 3;  //initial position
    double A1 = -6.49268 * pow(10, -2);
    double A2 = -4.15615 * pow(10, -2);
    double A3 = 1.65023 * pow(10, -2);
    double A4 = -1.84747 * pow(10, -3); 
    double A5 = 6.74355 * pow(10, -5);
    double A6 = 0;

    double AMax = 0.0019 * V0 + 0.0008;
    double Amin = abs(-0.0019 * V0 + 0.0047);
    double time = 0, maxX = 0, theta = 0, x_t = 0;
    double damp = 0;
    double x = 0;

    float level = 0.1f, currentLevel = 0.1f;
    double A3Frequency = 440.0;

};
