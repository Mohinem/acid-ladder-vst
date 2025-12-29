#pragma once
#include <JuceHeader.h>

//==============================================================================
// Minimal mono "acid voice" with ladder-ish resonant 4-pole filter.
// Uses: cutoff (Hz), res (0..1), envmod (0..1-ish), drive (0..1), etc.
// Still keeps your simple decay envelope and saw->square morph osc.
class AcidVoice
{
public:
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;

        phase = 0.0f;
        env = 0.0f;
        envCoef = 0.0f;

        targetFreq  = 110.0f;
        currentFreq = 110.0f;
        gate = false;
        vel  = 0.0f;

        // filter state
        z1 = z2 = z3 = z4 = 0.0f;
        lastY = 0.0f;

        // legacy lp not used anymore, but keep zeroed in case you referenced it elsewhere
        lp = 0.0f;
    }

    void setParams (float waveIn, float cutoffIn, float resIn, float envmodIn,
                    float decayIn, float accentIn, float glideMsIn,
                    float driveIn, float gainIn)
    {
        wave    = waveIn;
        cutoff  = cutoffIn;     // Hz
        res     = resIn;        // 0..1
        envmod  = envmodIn;     // 0..1-ish
        decay   = decayIn;      // seconds
        accent  = accentIn;     // 0..1
        glideMs = glideMsIn;    // ms
        drive   = driveIn;      // 0..1
        gain    = gainIn;       // linear

        // simple exponential decay envelope coefficient
        const float d = juce::jmax (0.001f, decay);
        envCoef = std::exp (-1.0f / (sr * d));
    }

    void noteOn (int midiNote, float velocity)
    {
        gate = true;
        vel = juce::jlimit (0.0f, 1.0f, velocity);
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

        z1 = z2 = z3 = z4 = 0.0f;
        lastY = 0.0f;
    }

    float render()
    {
        // --- glide (as in your original code) ---
        const float glideSec  = juce::jmax (0.0f, glideMs) * 0.001f;
        const float glideCoef = (glideSec <= 0.0f) ? 0.0f : std::exp (-1.0f / (sr * glideSec));
        currentFreq = glideCoef * currentFreq + (1.0f - glideCoef) * targetFreq;

        // --- oscillator: saw -> square morph (same idea as before) ---
        phase += currentFreq / sr;
        if (phase >= 1.0f) phase -= 1.0f;

        float saw = 2.0f * phase - 1.0f;
        float sq  = (phase < 0.5f) ? 1.0f : -1.0f;
        float osc = juce::jmap (wave, saw, sq);

        // --- envelope decay ---
        env *= envCoef;

        // --- cutoff with envelope modulation (keep your behavior) ---
        // envmod maps to an added cutoff range. Tune this later if you want.
        float fc = cutoff + envmod * 8000.0f * env;
        fc = juce::jlimit (20.0f, 18000.0f, fc);

        // --- Ladder-ish resonant 4-pole filter ---
        // Coefficient for one-pole stage: g = 1 - exp(-2*pi*fc/sr)
        const float g = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * fc / sr);

        // Resonance amount. This is "by ear" scaling.
        // Typical usable range: res 0..1 => k about 0..~4.
        // Too high without clipping can explode, so we soft-clip the loop.
        float k = juce::jlimit (0.0f, 1.0f, res);
        k *= 4.0f;

        // Drive: pre-gain into the ladder core
        const float pre = 1.0f + 6.0f * juce::jlimit (0.0f, 1.0f, drive);
        float u = osc * pre;

        // feedback (use last output)
        u -= k * lastY;

        // soft clip in the loop for stability + character
        u = softClip (u);

        // 4 cascaded one-poles (24 dB-ish)
        z1 += g * (u  - z1);
        z2 += g * (z1 - z2);
        z3 += g * (z2 - z3);
        z4 += g * (z3 - z4);

        float y = z4;
        lastY = y;

        // optional output saturation (kept from your original "drive coloration")
        y = std::tanh (y);

        // accent boosts volume a bit based on velocity (same as before)
        const float acc = 1.0f + accent * 0.6f * (vel > 0.7f ? 1.0f : 0.0f);

        // final amp
        y *= env * acc * gain;

        // avoid denormals
        if (std::abs (y) < 1e-12f) y = 0.0f;

        return y;
    }

private:
    // Cheap, stable soft clip (faster than tanh in the feedback loop)
    static inline float softClip (float v)
    {
        const float a = 0.8f;
        return v / (1.0f + a * std::abs (v));
    }

    // --- common voice state ---
    float sr = 44100.0f;

    float phase = 0.0f;

    float env = 0.0f;
    float envCoef = 0.0f;

    float targetFreq  = 110.0f;
    float currentFreq = 110.0f;

    bool  gate = false;
    float vel  = 0.0f;

    // --- params (set via setParams) ---
    float wave    = 0.0f;     // 0..1
    float cutoff  = 800.0f;   // Hz
    float res     = 0.0f;     // 0..1
    float envmod  = 0.5f;     // 0..1-ish
    float decay   = 0.2f;     // seconds
    float accent  = 0.0f;     // 0..1
    float glideMs = 0.0f;     // ms
    float drive   = 0.0f;     // 0..1
    float gain    = 0.2f;     // linear

    // --- filter state (ladder-ish) ---
    float z1 = 0.0f, z2 = 0.0f, z3 = 0.0f, z4 = 0.0f;
    float lastY = 0.0f;

    // kept from your original code (not used now, but harmless)
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
