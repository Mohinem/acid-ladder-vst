#pragma once
#include <JuceHeader.h>

//==============================================================================
// Minimal mono "acid voice" stub so the project compiles + makes sound.
// Replace later with your ladder filter voice if you already had one.
class AcidVoice
{
public:
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        phase = 0.0f;
        env = 0.0f;
        envCoef = 0.0f;
        targetFreq = 110.0f;
        currentFreq = 110.0f;
        gate = false;
    }

    void setParams (float waveIn, float cutoffIn, float resIn, float envmodIn,
                    float decayIn, float accentIn, float glideMsIn,
                    float driveIn, float gainIn)
    {
        wave   = waveIn;
        cutoff = cutoffIn;
        res    = resIn;
        envmod = envmodIn;
        decay  = decayIn;
        accent = accentIn;
        glideMs = glideMsIn;
        drive  = driveIn;
        gain   = gainIn;

        // simple exponential decay envelope coefficient
        // decay is in seconds
        const float d = juce::jmax(0.001f, decay);
        envCoef = std::exp(-1.0f / (sr * d));
    }

    void noteOn (int midiNote, float velocity)
    {
        gate = true;
        vel = juce::jlimit(0.0f, 1.0f, velocity);
        targetFreq = juce::MidiMessage::getMidiNoteInHertz (midiNote);
        env = 1.0f; // instant attack for now
    }

    void noteOff (int /*midiNote*/)
    {
        gate = false;
        // let envelope decay naturally
    }

    void reset()
    {
        gate = false;
        env = 0.0f;
    }

    float render()
    {
        // glide
        const float glideSec = juce::jmax(0.0f, glideMs) * 0.001f;
        const float glideCoef = (glideSec <= 0.0f) ? 0.0f : std::exp(-1.0f / (sr * glideSec));
        currentFreq = glideCoef * currentFreq + (1.0f - glideCoef) * targetFreq;

        // oscillator: saw (good enough for now)
        phase += currentFreq / sr;
        if (phase >= 1.0f) phase -= 1.0f;
        float osc = 2.0f * phase - 1.0f;

        // optional wave morph (0..1): saw -> square-ish
        // not a real 303 wave, but gives variation
        float sq = (phase < 0.5f) ? 1.0f : -1.0f;
        osc = juce::jmap(wave, osc, sq);

        // simple "filter-ish" tone control via one-pole lowpass
        // cutoff param is Hz; map to coefficient
        const float fc = juce::jlimit(20.0f, 18000.0f, cutoff + envmod * 8000.0f * env);
        const float x = std::exp(-2.0f * juce::MathConstants<float>::pi * fc / sr);
        lp = x * lp + (1.0f - x) * osc;

        // crude "res/drive" coloration
        float y = lp;
        y *= (1.0f + drive * 6.0f);
        y = std::tanh(y);

        // envelope decay
        env *= envCoef;

        // accent boosts volume a bit based on velocity
        const float acc = 1.0f + accent * 0.6f * (vel > 0.7f ? 1.0f : 0.0f);
        y *= env * acc * gain;

        // avoid denormals
        if (std::abs(y) < 1e-12f) y = 0.0f;
        return y;
    }

private:
    float sr = 44100.0f;

    float wave = 0.0f;
    float cutoff = 800.0f;
    float res = 0.35f;
    float envmod = 0.75f;
    float decay = 0.18f;
    float accent = 0.75f;
    float glideMs = 80.0f;
    float drive = 0.35f;
    float gain = 0.85f;

    bool gate = false;
    float vel = 1.0f;

    float phase = 0.0f;
    float env = 0.0f;
    float envCoef = 0.0f;

    float targetFreq = 110.0f;
    float currentFreq = 110.0f;

    float lp = 0.0f;
};


class AcidSynthAudioProcessor : public juce::AudioProcessor
{
public:
    AcidSynthAudioProcessor();
    ~AcidSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

    juce::AudioProcessorValueTreeState apvts;

    // Used by the on-screen keyboard in the editor
    juce::MidiKeyboardState keyboardState;

    // Your existing synth voice (must exist somewhere in your project)
    // If you already had this as a member before, keep the same type.
    AcidVoice voice;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessor)
};
