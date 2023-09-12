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
    BASE_FREQ = cyclePerSecond;
    angleDelta = cyclePerSample * 2.0 * juce::MathConstants<double>::pi;
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

void RhodesWaveVoice::pitchWheelMoved(int newPitchWheelValue) {
    double wheelPos = ((float)newPitchWheelValue - 8192.0f) / 8192.0f; // ƒsƒbƒ`ƒzƒC[ƒ‹‚Ì’l‚ð-1.0‚©‚ç1.0‚Ì”ÍˆÍ‚É³‹K‰»
    double semitones = wheelPos * 2.0; // ”ÍˆÍ‚ð2”¼‰¹i‘S‰¹j•ª‚É‚·‚é
    pitchShift = std::pow(2.0, semitones / 12.0); // ”{‰¹”ä‚É•ÏŠ·‚µ‚Äƒsƒbƒ`ƒVƒtƒg—Ê‚ðŒvŽZ
    FREQ = BASE_FREQ * pitchShift;
}
void RhodesWaveVoice::controllerMoved(int, int) {}

void RhodesWaveVoice::aftertouchChanged(int newAftertouchValue)
{
    float aftertoutchPos = static_cast<float>(newAftertouchValue) / 127.0f; // ƒAƒtƒ^[ƒ^ƒbƒ`‚Ì’l‚ð-1.0‚©‚ç1.0‚Ì”ÍˆÍ‚É³‹K‰»
    double semitones = aftertoutchPos * 2.0; // ”ÍˆÍ‚ð2”¼‰¹i‘S‰¹j•ª‚É‚·‚é
    pitchShift = std::pow(2.0, semitones / 12.0); // ”{‰¹”ä‚É•ÏŠ·‚µ‚Äƒsƒbƒ`ƒVƒtƒg—Ê‚ðŒvŽZ
    FREQ = BASE_FREQ * pitchShift; //ƒsƒbƒ`ƒVƒtƒg‚ðŽü”g”‚É“K—p

}

double RhodesWaveVoice::renderNextSample()
{
    double theta = 0;
    double damp = 0;

    time = ss * PERIOD_SEC;
    theta = 2 * 3.14159265358979323846 * FREQ * time;
    damp = expl((-0.6 * time));
    if (time < (1 / (BASE_FREQ * 4)))
    {
        theta *= 2;
        x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta));
        x = x_t;
    }
    else
    {

        if (3 / (BASE_FREQ * 4) < time)
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

    value = ((-2 * (x / k) * c * exp(-((pow(x, 2)) / k))) * v) * A ;
    return value;
}

void RhodesWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (angleDelta != 0.0)
    {
        if (tailOff > 0.0)
        {
            while (--numSamples >= 0)
            {
                
                auto currentSample = renderNextSample() * level  * tailOff;

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

                auto currentSample = renderNextSample() * level ;
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

