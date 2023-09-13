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

RhodesWaveVoice::RhodesWaveVoice() :
	juce::SynthesiserVoice(),
	tailOff(0.0),
	cx(0),
	ss(0),
	freq(0),
	base_freq(0),
	period_sec(0),
	c(0.000050),
	k(20),
	f1(8),
	f2(4186),
	a1(10),
	a2(0.01),
	x0(3),
	A1(-6.49268 * pow(10, -2)),
	A2(-4.15615 * pow(10, -2)),
	A3(1.65023 * pow(10, -2)),
	A4(-1.84747 * pow(10, -3)),
	A5(6.74355 * pow(10, -5)),
	A6(0),
	theta(0),
	damp(1),
	level(0.1)
{

}

bool RhodesWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
	return dynamic_cast<RhodesWaveSound*> (sound) != nullptr;
}

void RhodesWaveVoice::startNote(int midiNoteNumber, float velocity,
	juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
	level = velocity * 0.1;
	tailOff = 1.0;

	auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber, A3Frequency);
	auto cyclePerSample = cyclePerSecond / getSampleRate();

	freq = cyclePerSecond;
	base_freq = cyclePerSecond;
	theta = 0.0;
	damp = 1.0;
}

void RhodesWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
	if (allowTailOff)
	{
		if (tailOff == 1.0)
			tailOff *= 0.99;
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
	ss = 0;
	cx = 0.0;
	tailOff = 0.0;
}

double RhodesWaveVoice::renderNextSample()
{
	double x = 0;
	double V0 = 4000;
	double AMax = 0.0019 * V0 + 0.0008;
	double Amin = abs(-0.0019 * V0 + 0.0047);
	double maxX = 0;

	theta += 2 * juce::MathConstants<double>::pi * freq * period_sec;
	damp *= expl((-0.6 * period_sec));

	if (ss == 0)
	{
		theta = 0;
		damp = 1;
	}


	if (ss < (getSampleRate() / (base_freq * 4)))
	{
		x = x0 + damp * 0.5 * AMax * (1 - cos(theta * 2));
	}
	else if (ss < 3 *getSampleRate()/ (base_freq * 4))
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

	if (ss == 0)
	{
		v = 0;
	}
	else
	{
		v = ((x - cx) * getSampleRate());
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

	value = ((-2 * (x / k) * c * exp(-((x*x) / k))) * v) * A * level;

	ss += 1;

	return value;
}

void RhodesWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
	if (tailOff > 0.0)
	{
		while (--numSamples >= 0)
		{
			auto currentSample = renderNextSample() * tailOff;

			for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
				outputBuffer.addSample(i, startSample, currentSample);

			++startSample;
			if(tailOff<1.0)
			{
				tailOff *= 0.99;
				if (tailOff <= 0.005)
				{
					clearNote();
					break;
				}
			}
		}
	}
}

