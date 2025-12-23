#pragma once
#include <JuceHeader.h>

// ------------------- Ladder Filter (Stilson/Smith-style with tanh) -------------------
struct TanhLadder
{
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        z1 = z2 = z3 = z4 = 0.0f;
        lastCut = 1000.0f;
        lastRes = 0.2f;
        updateCoeffs();
    }

    void reset()
    {
        z1 = z2 = z3 = z4 = 0.0f;
    }

    void setParams (float cutoffHz, float resonance01, float drive01)
    {
        // keep stable ranges
        cutoffHz = juce::jlimit (20.0f, 18000.0f, cutoffHz);
        resonance01 = juce::jlimit (0.0f, 0.995f, resonance01);
        drive01 = juce::jlimit (0.0f, 1.0f, drive01);

        cut = cutoffHz;
        res = resonance01;
        drive = drive01;

        if (std::abs (cut - lastCut) > 0.5f || std::abs (res - lastRes) > 1.0e-4f)
            updateCoeffs();
    }

    float process (float x)
    {
        // input drive into soft saturation
        const float inDrive = 1.0f + 10.0f * drive;      // 1..11
        const float sat = std::tanh (x * inDrive);

        // resonance feedback from last stage output
        float u = sat - k * z4;

        // 4 one-pole stages with tanh in each stage (gives "squelch")
        float y1 = stage (u, z1);
        float y2 = stage (y1, z2);
        float y3 = stage (y2, z3);
        float y4 = stage (y3, z4);

        return y4;
    }

private:
    float sr = 44100.0f;
    float cut = 1000.0f, res = 0.2f, drive = 0.0f;

    float g = 0.0f;      // integrator coeff
    float k = 0.0f;      // resonance amount (scaled)
    float z1 = 0.0f, z2 = 0.0f, z3 = 0.0f, z4 = 0.0f;

    float lastCut = 1000.0f, lastRes = 0.2f;

    void updateCoeffs()
    {
        // TPT-ish one-pole coefficient (simple & stable)
        const float wc = 2.0f * juce::MathConstants<float>::pi * cut;
        const float T  = 1.0f / sr;
        const float wa = (2.0f / T) * std::tan (wc * T * 0.5f);
        g = wa * T * 0.5f;

        // resonance scaling: 303-like "near self-osc but not exploding"
        // 0..0.995 -> 0..~4
        k = 4.0f * res;
        lastCut = cut;
        lastRes = res;
    }

    float stage (float x, float& z)
    {
        // one-pole: y = (x + z) * g / (1 + g), with nonlinearity in feedback path
        const float v = (x - z) * (g / (1.0f + g));
        const float y = v + z;
        z = y + v; // integrator update

        // soft clip stage output
        return std::tanh (y);
    }
};

// ------------------- Simple Osc (Saw/Square) -------------------
struct AcidOsc
{
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        phase = 0.0f;
        phaseInc = 0.0f;
    }

    void setFrequency (float hz)
    {
        freq = hz;
        phaseInc = freq / sr;
    }

    void setWave (float wave01) // 0 = saw, 1 = square
    {
        wave = wave01;
    }

    float process()
    {
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;

        // naive but fine for acid; can add polyBLEP later
        const float saw = 2.0f * phase - 1.0f;
        const float sq  = (phase < 0.5f) ? 1.0f : -1.0f;

        // smooth morph (optional, but fun)
        return juce::jmap (wave, saw, sq);
    }

private:
    float sr = 44100.0f;
    float freq = 110.0f;
    float wave = 0.0f;
    float phase = 0.0f;
    float phaseInc = 0.0f;
};

// ------------------- Decay-only Env (303-ish) -------------------
struct DecayEnv
{
    void prepare (double sampleRate)
    {
        sr = (float) sampleRate;
        level = 0.0f;
        coeff = 0.9995f;
    }

    void setDecaySeconds (float sec)
    {
        sec = juce::jlimit (0.01f, 2.0f, sec);
        // exponential decay to ~ -60 dB
        coeff = std::exp (std::log (0.001f) / (sec * sr));
    }

    void trigger (float accentGain)
    {
        // accent pushes the starting level higher
        level = juce::jlimit (0.0f, 2.0f, 1.0f * accentGain);
        active = true;
    }

    float process()
    {
        if (! active) return 0.0f;
        const float out = level;
        level *= coeff;
        if (level < 1.0e-5f) { level = 0.0f; active = false; }
        return out;
    }

    bool isActive() const { return active; }

private:
    float sr = 44100.0f;
    float level = 0.0f;
    float coeff = 0.9995f;
    bool active = false;
};

// ------------------- Mono Acid Voice -------------------
struct AcidVoice
{
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        osc.prepare (sampleRate);
        env.prepare (sampleRate);
        filter.prepare (sampleRate);
        currentHz = targetHz = 110.0f;
        glideCoef = 0.0f;
        lastNote = -1;
        gate = false;
    }

    void reset()
    {
        filter.reset();
        currentHz = targetHz = 110.0f;
        lastNote = -1;
        gate = false;
    }

    void setParams (float wave01,
                    float cutoffHz, float res01,
                    float envMod01, float decaySec,
                    float accentAmt01, float glideMs,
                    float drive01, float outGain)
    {
        wave = wave01;
        cutoff = cutoffHz;
        resonance = res01;
        envMod = envMod01;
        decay = decaySec;
        accentAmt = accentAmt01;
        drive = drive01;
        gain = outGain;

        osc.setWave (wave);
        env.setDecaySeconds (decay);

        // glide coefficient (one-pole smoothing toward targetHz)
        glideMs = juce::jlimit (0.0f, 500.0f, glideMs);
        if (glideMs <= 0.0f)
            glideCoef = 1.0f;
        else
        {
            const double t = glideMs / 1000.0;
            glideCoef = (float) (1.0 - std::exp (-1.0 / (t * sr)));
        }

        filter.setParams (cutoff, resonance, drive);
    }

    void noteOn (int midiNote, float velocity01)
    {
        const float hz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);

        // Accent: velocity OR accentAmt boosts env + cutoff mod + amp
        const bool isAccent = (velocity01 > 0.80f); // simple rule; tweak later
        accent = isAccent ? (1.0f + 1.5f * accentAmt) : 1.0f;

        // Slide: if a note is already held, we glide instead of retriggering env fully
        if (gate)
        {
            targetHz = hz;
            // still small retrigger gives classic "bump"
            env.trigger (0.7f * accent);
        }
        else
        {
            currentHz = targetHz = hz;
            env.trigger (1.0f * accent);
            gate = true;
        }

        lastNote = midiNote;
        velocity = velocity01;
    }

    void noteOff (int midiNote)
    {
        // mono: release only if the note-off matches last note
        if (midiNote == lastNote)
            gate = false;
    }

    float render()
    {
        // glide toward target
        currentHz += (targetHz - currentHz) * glideCoef;
        osc.setFrequency (currentHz);

        const float e = env.process();

        // 303-ish: envelope modulates cutoff strongly; accent increases modulation
        const float envCut = cutoff * (1.0f + (2.5f * envMod * e * accent));
        filter.setParams (juce::jlimit (20.0f, 18000.0f, envCut), resonance, drive);

        float s = osc.process();

        // amp: env also affects amp a bit, accent boosts
        const float amp = (0.25f + 0.75f * e) * accent * (0.5f + 0.5f * velocity);
        s *= amp;

        s = filter.process (s);

        // extra output drive/clip
        s = std::tanh (s * (1.0f + 6.0f * drive));

        return s * gain;
    }

private:
    double sr = 44100.0;

    AcidOsc osc;
    DecayEnv env;
    TanhLadder filter;

    float wave = 0.0f;
    float cutoff = 800.0f;
    float resonance = 0.3f;
    float envMod = 0.7f;
    float decay = 0.2f;
    float accentAmt = 0.7f;
    float drive = 0.3f;
    float gain = 0.8f;

    float currentHz = 110.0f, targetHz = 110.0f;
    float glideCoef = 1.0f;

    float accent = 1.0f;
    float velocity = 1.0f;

    int lastNote = -1;
    bool gate = false;
};

// ------------------- Audio Processor -------------------
class AcidSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    AcidSynthAudioProcessor();
    ~AcidSynthAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

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

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();

private:
    AcidVoice voice;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessor)
};
