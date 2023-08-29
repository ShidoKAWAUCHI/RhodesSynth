/*
  ==============================================================================

    SynthVoice.cpp
    Created: 15 Jan 2023 12:57:42am
    Author:  1923031

  ==============================================================================
*/
#include "SynthVoice.h"
#include "PluginEditor.h"
bool RhodesWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<RhodesWaveSound*> (sound) != nullptr;
}

void RhodesWaveVoice::startNote(int midiNoteNumber, float velocity,
    juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentAngle = 0.0;
    level = currentLevel * velocity;
    tailOff = 0.0;
    auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber, A3Frequency);
    auto cyclePerSample = cyclePerSecond / getSampleRate();

    FREQ = cyclePerSecond;
    angleDelta = cyclePerSample * 2.0 * juce::MathConstants<double>::pi;
}
void RhodesWaveVoice::test() {
    sample = 48000;

}

void RhodesWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        angleDelta = 0.0;

        ss = 0;
        cx = 0;
    }
}

void RhodesWaveVoice::pitchWheelMoved(int) {}
void RhodesWaveVoice::controllerMoved(int, int) {}

void RhodesWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                    //sample = 48000;
                    PERIOD_SEC = 1 / sample;
                time = ss * PERIOD_SEC;
                if (time < (1 / (FREQ * 4)))
                {
                    theta = 4 * 3.14159265358979323846 * FREQ * time; 
                    damp = expl((-0.6 * time));
                    x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta));
                    x = x_t;
                }
                else
                {
                    theta = 2 * 3.14159265358979323846 * FREQ * time; 
                    damp = expl(-0.6 * time);

                    if (3 / (FREQ * 4) < time)
                    {
                        x_t = x0 + damp * Amin * sin(theta);
                        x = x_t;
                    }
                    else
                    {
                        x_t = x0 + damp * ((AMax - Amin) * 0.5 + (AMax + Amin) * 0.5 * sin(theta));
                        x = x_t;
                    }
                }

                double v = 0;
                double value = 0;
                double sign = 0;

                if (time == 0)
                {
                    v = 0;
                }
                else
                {
                    v = ((x - cx) / PERIOD_SEC);
                    if (x > 0)
                    {
                        sign = 1;
                    }
                    else if (x < 0)
                    {
                        sign = -1;
                    }
                    else
                    {
                        sign = 0;
                    }
                }
                /*value = (-1 * (sign * (A6 * pow(x, 6) + A5 * (abs(pow(x, 5))) + A4 * pow(x, 4)
                    + A3 * (abs(pow(x, 3))) + A2 * pow(x, 2) + A1 * (abs(x)))) * v);
                    value = (-2 * (x / k) * c * exp(-(x * x / k))) * v;*/

                double f[] = { f1, f2 };
                double a[] = { a1,a2 };
                double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2))) * (pow((FREQ - f[1]), 2)) + a[1];

                value = (-2 * (x / k) * c * exp(-((pow(x, 2)) / k))) * v;

                auto currentSample = value * level * A * tailOff;
                //auto currentSample = value * level *  tailOff;
                printf("%f\n", value);

                cx = x;

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample);

                ss += 1;

                currentAngle += angleDelta;
                ++startSample;


                tailOff *= 0.99;

                if (tailOff <= 0.005)
                {
                    clearCurrentNote();

                    angleDelta = 0.0;
                    ss = 0;
                    cx = 0;
                    break;
                }
            }
        }
        else
        {
            while (--numSamples >= 0)
            {

                time = ss * PERIOD_SEC;

                if (time < (1 / (FREQ * 4)))
                {
                    theta = 4 * 3.14159265358979323846 * FREQ * time; 
                    damp = expl((-0.6 * time)); 
                    x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta));
                    x = x_t;


                }
                else 
                {
                    theta = 2 * 3.14159265358979323846 * FREQ * time; 
                    damp = expl(-0.6 * time);

                    if (3 / (FREQ * 4) < time)
                    {
                        x_t = x0 + damp * Amin * sin(theta); 
                        x = x_t;
                    }
                    else
                    {
                        x_t = x0 + damp * ((AMax - Amin) * 0.5 + (AMax + Amin) * 0.5 * sin(theta)); 
                        x = x_t;
                    }
                }

                double v = 0;
                double value = 0;
                double sign = 0;

                if (time == 0)
                {
                    v = 0;
                }
                else
                {
                    v = ((x - cx) / PERIOD_SEC);
                    if (x > 0)
                    {
                        sign = 1;
                    }
                    else if (x < 0)
                    {
                        sign = -1;
                    }
                    else
                    {
                        sign = 0;
                    }
                }

               /* value = (-1 * (sign * (A6 * pow(x, 6) + A5 * (abs(pow(x, 5))) + A4 * pow(x, 4)
                    + A3 * (abs(pow(x, 3))) + A2 * pow(x, 2) + A1 * (abs(x)))) * v);*/
                printf("value");

                int n;
                double s, p;
                int    i, j;
                double t;

                s = 0.0;
                double f[] = { f1, f2 };
                double a[] = { a1,a2 };
                double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2))) * (pow((FREQ - f[1]), 2)) + a[1];
                value = (-2 * (x / k) * c * exp(-((pow(x, 2)) / k))) * v; 

                auto currentSample = value * level * A;

                cx = x;

                for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                    outputBuffer.addSample(i, startSample, currentSample); 
                ss += 1; 
                currentAngle += angleDelta;
                ++startSample;
            }
        }
    }
}

