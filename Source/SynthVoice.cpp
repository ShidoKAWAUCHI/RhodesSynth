/*
  ==============================================================================

	SynthVoice.cpp
	Created: 15 Jan 2023 12:57:42am
	Author:  1923031

  ==============================================================================
*/
#include "SynthVoice.h"
#include "PluginEditor.h"

double constexpr RhodesWaveVoice::A3Frequency;

bool RhodesWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
	return dynamic_cast<RhodesWaveSound*> (sound) != nullptr;
}

void RhodesWaveVoice::startNote(int midiNoteNumber, float velocity,
	juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
	level = velocity * 0.1f;
	tailOff = 0.0;

	auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber, A3Frequency);
	auto cyclePerSample = cyclePerSecond / getSampleRate();

	freq = cyclePerSecond;
	base_freq = cyclePerSecond;
	angleDelta = cyclePerSample * 2.0 * juce::MathConstants<double>::pi;
	theta = 0.0;
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
		clearNote();
	}
}

void RhodesWaveVoice::pitchWheelMoved(int newPitchWheelValue)
{
	double wheelPos = ((float)newPitchWheelValue - 8192.0f) / 8192.0f;
	pitchShiftPos(wheelPos);
}
void RhodesWaveVoice::controllerMoved(int, int) {}

void RhodesWaveVoice::aftertouchChanged(int newAftertouchValue)
{
	float aftertoutchPos = static_cast<float>(newAftertouchValue) / 127.0f;
	pitchShiftPos(aftertoutchPos);
}

void RhodesWaveVoice::pitchShiftPos(double pos)
{
	double semitones = pos * 2.0;
	double pitchShift = std::pow(2.0, semitones / 12.0);
	freq = base_freq * pitchShift;
}

void RhodesWaveVoice::setCurrentPlaybackSampleRate(double 	newRate)
{
	juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
	double sampleRate = newRate;
	period_sec = 1 / sampleRate;
}

void RhodesWaveVoice::clearNote()
{
	clearCurrentNote();
	angleDelta = 0.0;
	ss = 0;
	cx = 0;
}

double RhodesWaveVoice::renderNextSample()
{
	double  damp = 0, time = 0, x = 0;
	double V0 = 4000;
	double AMax = 0.0019 * V0 + 0.0008;
	double Amin = abs(-0.0019 * V0 + 0.0047);
	double maxX = 0;

	time = ss * period_sec;
	theta += 2 * juce::MathConstants<double>::pi * freq * period_sec;

	//time‚Ætheta‚Ì“ª‚ð‘µ‚¦‚é
	if (time == 0)
	{
		theta = 0;
	}

	damp = expl((-0.6 * time));

	if (time < (1 / (base_freq * 4)))
	{
		x = x0 + damp * 0.5 * AMax * (1 - cos(theta * 2));
	}
	else if (time < 3 / (base_freq * 4))
	{
		x = x0 + damp * ((AMax - Amin) * 0.5 + (AMax + Amin) * 0.5 * sin(theta));
	}
	else
	{
		x = x0 + damp * Amin * sin(theta);
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
		v = ((x - cx) / period_sec);
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

	cx = x;

	/*value = (-1 * (sign * (A6 * pow(x, 6) + A5 * (abs(pow(x, 5))) + A4 * pow(x, 4)
		+ A3 * (abs(pow(x, 3))) + A2 * pow(x, 2) + A1 * (abs(x)))) * v);
		value = (-2 * (x / k) * c * exp(-(x * x / k))) * v;*/

	double f[] = { f1, f2 };
	double a[] = { a1,a2 };
	double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2))) * (pow((freq - f[1]), 2)) + a[1];

	value = ((-2 * (x / k) * c * exp(-((pow(x, 2)) / k))) * v) * A * level;

	ss += 1;

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
				auto currentSample = renderNextSample() * tailOff;

				for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
					outputBuffer.addSample(i, startSample, currentSample);

				++startSample;

				tailOff *= 0.99;

				if (tailOff <= 0.005)
				{
					clearNote();
					break;
				}
			}
		}
		else
		{
			while (--numSamples >= 0)
			{
				auto currentSample = renderNextSample();

				for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
					outputBuffer.addSample(i, startSample, currentSample);

				++startSample;
			}
		}
	}
}

