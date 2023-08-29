#pragma once

#include <JuceHeader.h>

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

    SineWaveVoice(float targetLevel, double targetFrequency,double targetc,double targetk,double targetx0,double targetf1,double targetf2,double targetf3) 
    {
        currentLevel = targetLevel;
        A3Frequency = targetFrequency;
        c = targetc;
        k = targetk;
        x0 = targetx0;
        f1 = targetf1;
        f2 = targetf2;
        f3 = targetf3;
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        //level = velocity * 0.15;
        level = currentLevel * velocity;

        tailOff = 0.0;
        
        //auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclePerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber, A3Frequency);

        auto cyclePerSample = cyclePerSecond / getSampleRate();
        

        FREQ = cyclePerSecond;
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

            ss = 0;
            cx = 0;
        }
    }
    
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}


    void renderNextBlock (juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override //実際に音声データを作成する関数
    {   
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0)
            {
                while (--numSamples >= 0)
                {
                    //auto currentSample = (float) (std::sin (currentAngle) * level * tailOff);
                    time = ss * PERIOD_SEC;

                    if (time < (1 / (FREQ * 4)))
                    {
                        theta = 4 * 3.14159265358979323846 * FREQ * time; //juce::MathConstants<double>::pi
                        damp = expl((-0.6 * time));
                        x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta));
                        x = x_t;
                    }
                    else
                    {
                        theta = 2 * 3.14159265358979323846 * FREQ * time; //juce::MathConstants<double>::pi
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
                    //キーボードから離したときの処理

                    //音量調節
                    double f[] = { f1, f2, f3 };
                    double C[] = { 10,1,0.1 };
                    double N1 = (((FREQ - f[1]) * (FREQ - f[2])) / ((f[0] - f[1]) * (f[0] - f[2]))) * C[0];
                    double N2 = (((FREQ - f[0]) * (FREQ - f[2])) / ((f[1] - f[0]) * (f[1] - f[2]))) * C[1];
                    double N3 = (((FREQ - f[0]) * (FREQ - f[1])) / ((f[2] - f[0]) * (f[2] - f[1]))) * C[2];
                    double N = N1 + N2 + N3;

                    value = (-2 * (x / k) * c * exp(-(x * x / k))) * v; 

                    auto currentSample = value * level * N*tailOff;

                    cx = x;

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        //outputBuffer.addSample (i, startSample, value);
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
                while (--numSamples >= 0) //音声ブロックの全サンプル数分のループ
                {
                    //auto currentSample = (float) (std::sin (currentAngle) * level);

                    time = ss * PERIOD_SEC;

                    //先端の変位を求める
                    //T=1/FREQ
                    if (time < (1 / (FREQ * 4)))  //0<t<T/4
                    {
                        theta = 4 * 3.14159265358979323846 * FREQ * time; //juce::MathConstants<double>::pi
                        damp = expl((-0.6 * time)); //減衰
                        x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta)); //theta=4πt/T (5)の式
                        x = x_t;
                    }
                    else //T/4<t
                    {
                        theta = 2 * 3.14159265358979323846 * FREQ * time; //juce::MathConstants<double>::pi
                        damp = expl(-0.6 * time);

                        if (3 / (FREQ * 4) < time)
                        {
                            x_t = x0 + damp * Amin * sin(theta); //(1)の式
                            x = x_t;
                        }
                        else //T/4<t<3T/4
                        {
                            x_t = x0 + damp * ((AMax - Amin) * 0.5 + (AMax + Amin) * 0.5 * sin(theta)); //(6)の式
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

                    //キーボードを押したときの処理
                    //ファラデーの法則より誘導起電力を求める（最初のマイナスは本来の磁束と反対の磁束を発生させている）20220414 ここをいじる
                    /*value = (-1 * (sign * (A6 * pow(x, 6) + A5 * (abs(pow(x, 5))) + A4 * pow(x, 4)
                        + A3 * (abs(pow(x, 3))) + A2 * pow(x, 2) + A1 * (abs(x)))) * v);*/
                   //音量調節
                    int n;
                    double s, p;
                    int    i, j;
                    double t;

                    s = 0.0;
                    double f[] = {f1, f2, f3};
                    double C[] = { 10,1,0.1 };
                    double N1 = ( ( (FREQ - f[1]) * (FREQ - f[2]) ) / ( (f[0] - f[1]) * (f[0] - f[2]) ) ) * C[0];
                    double N2 = ( ( (FREQ - f[0]) * (FREQ - f[2]) ) / ( (f[1] - f[0]) * (f[1] - f[2]) ) ) * C[1];
                    double N3 = ( ( (FREQ - f[0]) * (FREQ - f[1]) ) / ( (f[2] - f[0]) * (f[2] - f[1]) ) ) * C[2];
                    double N = N1 + N2 + N3;
                    
                    value = (-2 * (x / k) * c * exp(-(x * x / k))) * v; 
                    
                    auto currentSample = value * level*N;

                        
                    cx = x;

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        //outputBuffer.addSample (i, startSample, value);
                        outputBuffer.addSample(i, startSample, currentSample); //取得した値を格納して音声データを作成
                        
                    ss += 1; //秒数の更新
                    currentAngle += angleDelta; //angleDeltaを更新
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0, angleDelta = 0.0, tailOff = 0.0;
    double cx = 0;
    double ss = 0;

    double FREQ = 0;
    double sample = 48000;
    double PERIOD_SEC = 1 / sample;
    double V0 = 4000; //ハンマー速度 A3の時V0=4000mm/s

    double c = 0.000050;
    double k = 20;

    double f1 = 300;
    double f2 = 2600;
    double f3 = 5300;

    double x0 = 3;  //initial position
    double A1 = -6.49268 * pow(10, -2);
    double A2 = -4.15615 * pow(10, -2);
    double A3 = 1.65023 * pow(10, -2);
    double A4 = -1.84747 * pow(10, -3);
    double A5 = 6.74355 * pow(10, -5);
    double A6 = 0;

    // LX
    double AMax = 0.0019 * V0 + 0.0008;
    double Amin = abs(-0.0019 * V0 + 0.0047);
    double time = 0, maxX = 0, theta = 0, x_t = 0;
    double damp = 0;
    double x = 0;

    float level = 0.1f, currentLevel = 0.1f; 
    double A3Frequency = 440.0;
};


class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState) : keyboardState(keyState)
    {
        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,f1,f2,f3));
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

    void changeFrequency (double targetFrequency)
    {
        synth.clearVoices();

        A3Frequency = targetFrequency;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,f1,f2,f3));
    }

    void changeLevel (float targetLevel)
    {
        synth.clearVoices();

        level = targetLevel;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,f1,f2,f3));
    }

    void changec(float targetc)
    {
        synth.clearVoices();

        c = targetc;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c,k,x0,f1,f2,f3));
    }

    void changek(float targetk)
    {
        synth.clearVoices();

        k = targetk;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k,x0,f1,f2,f3));
    }

    void changex0(float targetx0)
    {
        synth.clearVoices();

        x0 = targetx0;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0,f1,f2,f3));
    }

    void changef1(float targetf1)
    {
        synth.clearVoices();

        f1 = targetf1;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0, f1, f2, f3));
    }

    void changef2(float targetf2)
    {
        synth.clearVoices();

        f2 = targetf2;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0, f1, f2, f3));
    }

    void changef3(float targetf3)
    {
        synth.clearVoices();

        f3 = targetf3;

        for (auto i = 0; i < 4; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0, f1, f2, f3));
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::MidiMessageCollector midiCollector;

    float  level = 0.8f; 
    double A3Frequency = 440.0;
    double c = 0.000050;
    double k = 20.0;
    double x0 = 3.0;
    double f1 = 300;
    double f2 = 2600;
    double f3 = 5300;
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
        
        A3FrequencyLabel.setText ("A4[Hz]:", juce::dontSendNotification);
        addAndMakeVisible (A3FrequencyLabel);
      
        addAndMakeVisible(cSlider);
        cSlider.setRange(0.00001, 0.001);
        cSlider.setValue(currentc, juce::dontSendNotification);
        cSlider.onValueChange = [this] {
            targetc = cSlider.getValue();
            synthAudioSource.changec(targetc);
        };

        cLabel.setText("c:", juce::dontSendNotification);
        addAndMakeVisible(cLabel);

        addAndMakeVisible(kSlider);
        kSlider.setRange(0.0, 100.0);
        kSlider.setValue(currentk, juce::dontSendNotification);
        kSlider.onValueChange = [this] {
            targetk = kSlider.getValue();
            synthAudioSource.changek(targetk);
        };

        kLabel.setText("k:", juce::dontSendNotification);
        addAndMakeVisible(kLabel);

        addAndMakeVisible(x0Slider);
        x0Slider.setRange(-10.0, 10.0);
        x0Slider.setValue(currentx0, juce::dontSendNotification);
        x0Slider.onValueChange = [this] {
            targetx0 = x0Slider.getValue();
            synthAudioSource.changex0(targetx0);
        };

        x0Label.setText("x0:", juce::dontSendNotification);
        addAndMakeVisible(x0Label);

        addAndMakeVisible(f1Slider);
        f1Slider.setRange(-10.0, 10.0);
        f1Slider.setValue(currentf2, juce::dontSendNotification);
        f1Slider.onValueChange = [this] {
            targetf1 = f1Slider.getValue();
            synthAudioSource.changef1(targetf1);
        };

        f1Label.setText("f1:", juce::dontSendNotification);
        addAndMakeVisible(f1Label);

        addAndMakeVisible(f1Slider);
        f1Slider.setRange(0.0, 10000.0);
        f1Slider.setValue(currentf1, juce::dontSendNotification);
        f1Slider.onValueChange = [this] {
            targetf1 = f1Slider.getValue();
            synthAudioSource.changef1(targetf1);
        };

        f2Label.setText("f2:", juce::dontSendNotification);
        addAndMakeVisible(f2Label);

        addAndMakeVisible(f2Slider);
        f2Slider.setRange(0.0, 10000.0);
        f2Slider.setValue(currentf2, juce::dontSendNotification);
        f2Slider.onValueChange = [this] {
            targetf2 = f2Slider.getValue();
            synthAudioSource.changef2(targetf2);
        };

        f3Label.setText("f3:", juce::dontSendNotification);
        addAndMakeVisible(f3Label);

        addAndMakeVisible(f3Slider);
        f3Slider.setRange(0.0, 10000.0);
        f3Slider.setValue(currentf3, juce::dontSendNotification);
        f3Slider.onValueChange = [this] {
            targetf3 = f3Slider.getValue();
            synthAudioSource.changef3(targetf3);
        };

        f3Label.setText("f3:", juce::dontSendNotification);
        addAndMakeVisible(f3Label);
        
        addAndMakeVisible (keyboardComponent);

        setSize (800, 500);

       setAudioChannels (0, 2);
        startTimer (400);
    }

    ~MainComponent() override
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

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

        currentLevel = targetLevel;
        currentA3Frequency = targetA3Frequency;
        currentc = targetc;
        currentk = targetk;
        currentx0 = targetx0;
        currentf1 = targetf1;
        currentf2 = targetf2;
        currentf3 = targetf3;

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

        levelLabel.setBounds(10, 40, 60, 20);
        levelSlider.setBounds(70, 40, getWidth() - 70, 20);
        A3FrequencyLabel.setBounds(10, 70, 60, 20);
        A3FrequencySlider.setBounds(70, 70, getWidth() - 70, 20);
        cLabel.setBounds(10, 100, 60, 20);
        cSlider.setBounds(70, 100, getWidth() - 70, 20);
        kLabel.setBounds(10, 130, 60, 20);
        kSlider.setBounds(70, 130, getWidth() - 70, 20);
        x0Label.setBounds(10, 160, 60, 20);
        x0Slider.setBounds(70, 160, getWidth() - 70, 20);
        f1Label.setBounds(10, 190, 60, 20);
        f1Slider.setBounds(70, 190, getWidth() - 70, 20);
        f2Label.setBounds(10, 210, 60, 20);
        f2Slider.setBounds(70, 210, getWidth() - 70, 20);
        f3Label.setBounds(10, 230, 60, 20);
        f3Slider.setBounds(70, 230, getWidth() - 70, 20);
        //keyboardComponent.setBounds(10, 40, getWidth() - 20, getHeight() - 50);
        keyboardComponent.setBounds (10, 260, getWidth() - 20, getHeight() - 200);
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

    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::Slider A3FrequencySlider;
    juce::Label A3FrequencyLabel;
    juce::Slider cSlider;
    juce::Label cLabel;
    juce::Slider kSlider;
    juce::Label kLabel;
    juce::Slider x0Slider;
    juce::Label x0Label;
    juce::Slider f1Slider;
    juce::Label f1Label;
    juce::Slider f2Slider;
    juce::Label f2Label;
    juce::Slider f3Slider;
    juce::Label f3Label;

    juce::ComboBox midiInputList;
    juce::ComboBox CurveTranceList;
    juce::Label midiInputListLabel;
    juce::Label CurveTranceListLabel;
    int lastInputIndex = 0;

    float currentLevel = 0.1f, targetLevel = 0.1f;
    double currentA3Frequency = 440.0, targetA3Frequency = 440.0;
    double currentc = 0.000050, targetc = 0.000050;
    double currentk = 20.0, targetk = 20.0;
    double currentx0 = 3.0, targetx0 = 3.0;
    double currentf1 = 300, targetf1 = 300;
    double currentf2 = 2600, targetf2 = 2600;
    double currentf3 = 5300, targetf3 = 5300;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};