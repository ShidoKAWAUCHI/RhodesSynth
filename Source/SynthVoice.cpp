/*
  ==============================================================================

	SynthVoice.cpp
	Created: 15 Jan 2023 12:57:42am
	Author:  1923031

  ==============================================================================
*/
#include "SynthVoice.h"
#include "PluginEditor.h"

RhodesWaveVoice::RhodesWaveVoice(double targetlevel, double targetA3Frequency, double targetc, double targetk, double targetx0, double targeta1, double targeta2)
	:
	juce::SynthesiserVoice(),
	a3f_(targetA3Frequency),
	A3Frequency_(440.0),
	tailOff_(0.0),
	cx_(0.0),
	ss_(0),
	freq_(0.0),
	base_freq_(0.0),
	targetFreq_(0.0),
	period_sec_(0.0),
	c_(targetc),
	k_(targetk),
	f1_(8.0),
	f2_(4186.0),
	a1_(targeta1),
	a2_(targeta2),
	x0_(targetx0),
	A1_(-6.49268 * pow(10, -2)),
	A2_(-4.15615 * pow(10, -2)),
	A3_(1.65023 * pow(10, -2)),
	A4_(-1.84747 * pow(10, -3)),
	A5_(6.74355 * pow(10, -5)),
	A6_(0.0),
	theta_(0.0),
	damp_(1.0),
	level_(targetlevel),
	attack_(0.0),
	decay_(0.0),
	wheelPos_(0.0),
	aftertoutchPos_(0.0)
{
}

bool RhodesWaveVoice::canPlaySound(juce::SynthesiserSound* sound)
{
	return dynamic_cast<RhodesWaveSound*> (sound) != nullptr;
}

void RhodesWaveVoice::startNote(int midiNoteNumber, float velocity,
	juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
	level_ = velocity * 0.1;
	tailOff_ = 1.0;

	auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber, A3Frequency_);
	auto cyclePerSample = cyclePerSecond / getSampleRate();

	freq_ = cyclePerSecond;
	base_freq_ = cyclePerSecond;
	targetFreq_ = cyclePerSecond;
	theta_ = 0.0;
	damp_ = 1.0;
	attack_ = getSampleRate() / (base_freq_ * 4.0);
	decay_ = 3.0 * getSampleRate() / (base_freq_ * 4.0);

}

void RhodesWaveVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
	if (allowTailOff)
	{
		if (tailOff_ == 1.0)
			tailOff_ *= 0.99;
	}
	else
	{
		clearNote();
	}
}

void RhodesWaveVoice::pitchWheelMoved(int newPitchWheelValue)
{
	wheelPos_ = (static_cast<double>(newPitchWheelValue) - 8192.0) / 8192.0;
	pitchShiftPos(wheelPos_+aftertoutchPos_);
}


void RhodesWaveVoice::aftertouchChanged(int newAftertouchValue)
{
	aftertoutchPos_ = static_cast<double>(newAftertouchValue) / 127.0;
	pitchShiftPos(aftertoutchPos_+wheelPos_);
}

void RhodesWaveVoice::pitchShiftPos(double pos)
{
	double semitones = pos * 2.0;
	double pitchShift = std::pow(2.0, semitones / 12.0);
	targetFreq_ = base_freq_ * pitchShift;
}

void RhodesWaveVoice::setCurrentPlaybackSampleRate(double 	newRate)
{
	juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);
	double sampleRate = newRate;
	period_sec_ = 1.0 / sampleRate;
}

void RhodesWaveVoice::clearNote()
{
	clearCurrentNote();
	ss_ = 0.0;
	cx_ = 0.0;
	tailOff_ = 0.0;
}

void RhodesWaveVoice::renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
	if (tailOff_ > 0.0)
	{
		static double constexpr V0 = 4000.0;
		static double constexpr AMax = 0.0019 * V0 + 0.0008;
		static double constexpr Amin = 0.0019 * V0 < 0.0047 ? -0.0019 * V0 + 0.0047 : 0.0019 * V0 - 0.0047;
		static double constexpr alpha = 0.999;
		double x = 0.0;
		while (--numSamples >= 0)
		{
	
			freq_ = (1-alpha)*targetFreq_+(alpha*freq_);
			

			theta_ += 2.0 * juce::MathConstants<double>::pi * freq_ * period_sec_; //theta = 2 * 3.14159265358979323846 * freq * time; 
			damp_ *= expl((-0.6 * period_sec_));

			if (ss_ == 0.0)
			{
				theta_ = 0.0;
				damp_ = 1.0;
			}

			if (ss_ < attack_)
			{
				x = x0_ + damp_ * 0.5 * AMax * (1.0 - cos(theta_ * 2.0));
			}
			else if (ss_ < decay_)
			{
				x = x0_ + damp_ * ((AMax - Amin) * 0.5 + (AMax + Amin) * 0.5 * sin(theta_));
			}
			else
			{
				x = x0_ + damp_ * Amin * sin(theta_);
			}

			double v = 0.0;
			double value = 0.0;
			double sign = 0.0;

			if (ss_ == 0.0)
			{
				v = 0.0;
			}
			else
			{
				v = ((x - cx_) * getSampleRate());
				if (x > 0.0)
				{
					sign = 1.0;
				}
				else if (x < 0.0)
				{
					sign = -1.0;
				}
				else
				{
					sign = 0.0;
				}
			}

			cx_ = x;

			/*value = (-1 * (sign * (A6 * pow(x, 6) + A5 * (abs(pow(x, 5))) + A4 * pow(x, 4)
				+ A3 * (abs(pow(x, 3))) + A2 * pow(x, 2) + A1 * (abs(x)))) * v);
				value = (-2 * (x / k) * c * exp(-(x * x / k))) * v;*/

			double f[] = { f1_, f2_ };
			double a[] = { a1_,a2_ };
			double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2.0))) * (pow((freq_ - f[1]), 2.0)) + a[1];

			//value = ((-2.0 * (x / k_) * c_ * exp(-((x * x) / k_))) * v) * A * level_;
			value = ((-2.0 * (x / k_) * c_ * exp(-((x * x) / k_))) * v) * A *0.05;
			ss_ += 1.0;

			auto currentSample = value * tailOff_;

			for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
				outputBuffer.addSample(i, startSample, currentSample);

			++startSample;
			if (tailOff_ < 1.0)
			{
				tailOff_ *= 0.99;
				if (tailOff_ <= 0.005)
				{
					clearNote();
					break;
				}
			}
		}
	}
}

