#pragma once

#include <JuceHeader.h>

//SynthsiserSoundクラスを継承し、Synthsiserクラスが発音の際に使用する情報を返す構造体
struct SineWaveSound    : public juce::SynthesiserSound
{
    SineWaveSound() {}
    bool appliesToNote (int) override {return true; }
    bool appliesToChannel (int) override {return true; }
};

//SynthsiserVoiceクラスを継承し、Synthsiserオブジェクトの発音を担当する構造体
struct SineWaveVoice    : public juce::SynthesiserVoice
{

public:
    SineWaveVoice () {}
    
    //コンストラクタの呼び出し
    SineWaveVoice(float targetLevel, double targetFrequency,double targetc,double targetk,double targetx0,double targeta1,double targeta2) 
    {
        currentLevel = targetLevel;
        A3Frequency = targetFrequency;
        c = targetc;
        k = targetk;
        x0 = targetx0;
        a1 = targeta1;
        a2 = targeta2;
    }

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        midiNoteNumber = midiNoteNumber;
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
                    double f[] = { f1, f2 };
                    double a[] = { a1,a2 };
                    double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2))) * (pow((FREQ - f[1]), 2)) + a[1];

                    value = (-2* (x / k) * c * exp(-((pow(x,2))/ k))) * v; //一応鳴る
                   
                   //auto currentSample = value * level * A*tailOff;
                   auto currentSample = value * level *  tailOff;

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
                while (--numSamples >= 0) //音声ブロックの全サンプル数分のループ
                {

                    time = ss * PERIOD_SEC;

                    //先端の変位を求める
                    //T=1/FREQ
                    if (time < (1 / (FREQ * 4)))  //0<t<T/4
                    {
                        theta = 4 * 3.14159265358979323846 * FREQ * time; //juce::MathConstants<double>::pi
                        damp = expl((-0.6 * time)); //damp=減衰？
                        x_t = x0 + damp * 0.5 * AMax * (1 - cos(theta)); //theta=4πt/T (5)の式？
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
                    double f[] = {f1, f2};
                    double a[] = { a1,a2 };
                    double A = ((a[0] - a[1]) / (pow((f[0] - f[1]), 2))) * (pow((FREQ - f[1]), 2)) + a[1];
                    value = (-2 * (x / k) * c * exp(-((pow(x,2)) / k))) * v; 
                    
                    auto currentSample = value * level*A;                        
                    
                    cx = x;
                   
                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
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
    double PERIOD_SEC = 1 / sample;//サンプリング周期
    double V0 = 4000; //ハンマー速度 A3の時V0=4000mm/s
    
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

    // LX
    double AMax = 0.0019 * V0 + 0.0008;
    double Amin = abs(-0.0019 * V0 + 0.0047);
    double time = 0, maxX = 0, theta = 0, x_t = 0;
    double damp = 0;
    double x = 0;

    float level = 0.1f, currentLevel = 0.1f; 
    double A3Frequency = 440.0;
};

//AudioSourceクラスを継承し、音声出力を出来るようにするクラス
class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState) : keyboardState(keyState)
    {
        //シンセの同時発音数
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,a1,a2));
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
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,a1,a2));
    }

    void changeLevel (float targetLevel)
    {
        synth.clearVoices();

        level = targetLevel;

        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency,c,k,x0,a1,a2));
    }

    void changec(float targetc)
    {
        synth.clearVoices();
        c = targetc;
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c,k,x0,a1,a2));
    }

    void changek(float targetk)
    {
        synth.clearVoices();
        k = targetk;
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k,x0,a1,a2));
    }

    void changex0(float targetx0)
    {
        synth.clearVoices();
        x0 = targetx0;
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0,a1,a2));
    }

    void changea1(float targeta1)
    {
        synth.clearVoices();
        a1 = targeta1;
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0, a1, a2));
    }

    void changea2(float targeta2)
    {
        synth.clearVoices();
        a2 = targeta2;
        for (auto i = 0; i < 128; ++i)
            synth.addVoice(new SineWaveVoice(level, A3Frequency, c, k, x0, a1, a2));
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
    double a1 = 10.0;
    double a2 = 0.01;
    };

//AudioAppComponentクラスを継承し、主にGUIのオブジェクトを作成するクラス
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

        cLabel.setText("C:", juce::dontSendNotification);
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

        addAndMakeVisible(a1Slider);
        a1Slider.setRange(1.0, 100.0);
        a1Slider.setValue(currenta1, juce::dontSendNotification);
        a1Slider.onValueChange = [this] {
            targeta1 = a1Slider.getValue();
            synthAudioSource.changea1(targeta1);
        };

        a1Label.setText("a1:", juce::dontSendNotification);
        addAndMakeVisible(a1Label);

        addAndMakeVisible(a2Slider);
        a2Slider.setRange(0.001, 1.0);
        a2Slider.setValue(currenta2, juce::dontSendNotification);
        a2Slider.onValueChange = [this] {
            targeta2 = a2Slider.getValue();
            synthAudioSource.changea2(targeta2);
        };

        a2Label.setText("a2:", juce::dontSendNotification);
        addAndMakeVisible(a2Label);
        addAndMakeVisible (keyboardComponent);
        
        setSize (800, 500);
        
        setAudioChannels (0, 2);
        startTimer (400);
    }

    ~MainComponent() override
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        currentLevel = targetLevel;
        currentA3Frequency = targetA3Frequency;
        currentc = targetc;
        currentk = targetk;
        currentx0 = targetx0;
        currenta1 = targeta1;
        currenta2 = targeta2;
        synthAudioSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {     
        synthAudioSource.releaseResources();
    }

    //==============================================================================
    void paint (juce::Graphics& g) override
    {
       g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
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
        a1Label.setBounds(10, 190, 60, 20);
        a1Slider.setBounds(70, 190, getWidth() - 70, 20);
        a2Label.setBounds(10, 220, 60, 20);
        a2Slider.setBounds(70, 220, getWidth() - 70, 20);
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
    juce::Slider a1Slider;
    juce::Label a1Label;
    juce::Slider a2Slider;
    juce::Label a2Label;

    juce::ComboBox midiInputList;
    juce::Label midiInputListLabel;
    int lastInputIndex = 0;

    float currentLevel = 0.1f, targetLevel = 0.1f;
    double currentA3Frequency = 440.0, targetA3Frequency = 440.0;
    double currentc = 0.000050, targetc = 0.000050;
    double currentk = 20.0, targetk = 20.0;
    double currentx0 = 3.0, targetx0 = 3.0;
    double currenta1 = 10, targeta1 = 10;
    double currenta2 = 0.01, targeta2 = 0.01;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};