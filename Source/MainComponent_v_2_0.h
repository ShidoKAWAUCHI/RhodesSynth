#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
struct SineWaveSound    : public juce::SynthesiserSound
{
    SineWaveSound() {}
    
    bool appliesToNote (int) override {return true; }
    bool appliesToChannel (int) override {return true; }
};

struct SineWaveVoice    : public juce::SynthesiserVoice
{
public:
    SineWaveVoice () {}

    //enami added start
    SineWaveVoice(float targetLevel, double targetFrequency) 
    {
        currentLevel = targetLevel;
        A3Frequency = targetFrequency;
    }
    //enami added end

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;

        //enami modified start
        //level = velocity * 0.15;
        level = currentLevel * velocity;
        //enami modified end

        tailOff = 0.0;
        
        //enami modified start
        //auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber, A3Frequency);
        //enami modified end

        auto cyclePerSample = cyclePerSecond / getSampleRate();
        
        angleDelta = cyclePerSample * 2.0 * juce::MathConstants<double>::pi;
    }
    
    void stopNote (float /*velocity*/, bool allowTailOff) override
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
        }
    }
    
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    
    void renderNextBlock (juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {   
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (std::sin (currentAngle) * level * tailOff);
                    
                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);
                    
                    currentAngle += angleDelta;
                    ++startSample;
                    
                    tailOff *= 0.99;
                    
                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();
                        
                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float) (std::sin (currentAngle) * level);
                    
                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);
                        
                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0, angleDelta = 0.0, tailOff = 0.0;

    //enami added start
    float level = 0.1f, currentLevel = 0.1f; 
    double A3Frequency = 440.0;
    //enami added end
};

class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState) : keyboardState(keyState)
    {
        //enami modified start
        //for (auto i = 0; i < 4; ++i)
            //synth.addVoice(new SineWaveVoice());

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency));
        //enami modified end

        synth.addSound(new SineWaveSound());
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay(int /*samplesPerBlockexpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);
        midiCollector.reset(sampleRate);
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);

        keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }

    juce::MidiMessageCollector* getMidiCollector()
    {
        return &midiCollector;
    }

    //enami added start
    void changeFrequency (double targetFrequency)
    {
        synth.clearVoices();

        A3Frequency = targetFrequency;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency));
    }

    void changeLevel (float targetLevel)
    {
        synth.clearVoices();

        level = targetLevel;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency));
    }
    //enami added end

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;

    //enami added start
    float  level = 0.8f; 
    double A3Frequency = 440.0;
    //enami added end
};

class MainComponent  : public juce::AudioAppComponent, private juce::Timer
{
public:
    //==============================================================================
    MainComponent()  : synthAudioSource (keyboardState), keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        addAndMakeVisible (midiInputListLabel);
        midiInputListLabel.setText ("MIDI Input:", juce::dontSendNotification);
        midiInputListLabel.attachToComponent (&midiInputList, true);
        
        auto midiInputs = juce::MidiInput::getAvailableDevices();
        addAndMakeVisible (midiInputList);
        midiInputList.setTextWhenNoChoicesAvailable ("No MIDI Inputs Enabled");
        
        juce::StringArray midiInputNames;
        for (auto input : midiInputs)
            midiInputNames.add (input.name);
        
        midiInputList.addItemList (midiInputNames, 1);
        midiInputList.onChange = [this] { setMidiInput (midiInputList.getSelectedItemIndex()); };
        
        for (auto input : midiInputs)
        {
            if (deviceManager.isMidiInputDeviceEnabled (input.identifier))
            {
                setMidiInput (midiInputs.indexOf (input));
                break;
            }
        }
        
        if (midiInputList.getSelectedId() == 0)
            setMidiInput (-21);
        
        //enami added start
        addAndMakeVisible(levelSlider);
        levelSlider.setRange(0.0, 0.125f);
        levelSlider.setValue ((double) currentLevel, juce::dontSendNotification);
        targetLevel = currentLevel;
        synthAudioSource.changeLevel (targetLevel);
        levelSlider.onValueChange = [this] {
            targetLevel = (float)levelSlider.getValue();
            synthAudioSource.changeLevel (targetLevel);
        };

        levelLabel.setText("level:", juce::dontSendNotification);
        addAndMakeVisible(levelLabel);

        addAndMakeVisible(A3FrequencySlider);
        A3FrequencySlider.setRange(220.0, 880.0);
        A3FrequencySlider.setValue(currentA3Frequency, juce::dontSendNotification);
        A3FrequencySlider.setSkewFactorFromMidPoint(440.0);
        A3FrequencySlider.onValueChange = [this] {
            targetA3Frequency = A3FrequencySlider.getValue();
            synthAudioSource.changeFrequency (targetA3Frequency);
        };
        
        A3FrequencyLabel.setText ("A3[Hz]:", juce::dontSendNotification);
        addAndMakeVisible (A3FrequencyLabel);
        //enami added end
        
        addAndMakeVisible (keyboardComponent);
        // Make sure you set the size of the component after
        // you add any child components.
        
        //enami modified start
        //setSize (600, 160);
        setSize (600, 260);
        //enami modified end

        // Some platforms require permissions to open input channels so request that here
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
        startTimer (400);
    }

    ~MainComponent() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
        synthAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)

        //enami added start
        currentLevel = targetLevel;
        currentA3Frequency = targetA3Frequency;
        //enami added end

        synthAudioSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
        synthAudioSource.releaseResources();
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

        // You can add your drawing code here!
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        midiInputList.setBounds (200, 10, getWidth() - 210, 20);

        //enami added start
        levelLabel.setBounds(10, 40, 60, 20);
        levelSlider.setBounds(70, 40, getWidth() - 70, 20);
        A3FrequencyLabel.setBounds(10, 70, 60, 20);
        A3FrequencySlider.setBounds(70, 70, getWidth() - 70, 20);
        //enami added end

        //enami modified start
        //keyboardComponent.setBounds(10, 40, getWidth() - 20, getHeight() - 50);
        keyboardComponent.setBounds (10, 100, getWidth() - 20, getHeight() - 110);
        //enami modified end
    }


private:
    //==============================================================================
    // Your private member variables go here...
    void timerCallback() override
    {
        keyboardComponent.grabKeyboardFocus();
        stopTimer();
    }
    
    void setMidiInput (int index)
    {
        auto list = juce::MidiInput::getAvailableDevices();
        
        deviceManager.removeMidiInputDeviceCallback (list[lastInputIndex].identifier, synthAudioSource.getMidiCollector());
        
        auto newInput = list[index];
        
        if (! deviceManager.isMidiInputDeviceEnabled (newInput.identifier))
            deviceManager.setMidiInputDeviceEnabled (newInput.identifier, true);
        
        deviceManager.addMidiInputDeviceCallback (newInput.identifier, synthAudioSource.getMidiCollector());
        
        midiInputList.setSelectedId (index + 1, juce::dontSendNotification);
        
        lastInputIndex = index;
    }
    
    juce::MidiKeyboardState keyboardState;
    SynthAudioSource synthAudioSource;
    juce::MidiKeyboardComponent keyboardComponent;

    //enami added start
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::Slider A3FrequencySlider;
    juce::Label A3FrequencyLabel;
    //enami added end

    juce::ComboBox midiInputList;
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;

    //enami added start
    float currentLevel = 0.1f, targetLevel = 0.1f;
    double currentA3Frequency = 440.0, targetA3Frequency = 440.0;
    //enami added end

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};