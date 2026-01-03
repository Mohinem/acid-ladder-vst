#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <algorithm>

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
        filterCharSmoothed.reset (sr, 0.01);
        filterCharSmoothed.setCurrentAndTargetValue (0.0f);
        accentSmoothed.reset (sr, 0.01);
        accentSmoothed.setCurrentAndTargetValue (0.0f);

        phase = 0.0f;
        phaseUnisonA = 0.0f;
        phaseUnisonB = 0.0f;
        phaseSub = 0.0f;
        lfo1Phase = 0.0f;
        lfo2Phase = 0.0f;
        env = 0.0f;
        envCoef = 0.0f;
        modEnv = 0.0f;
        modEnvCoef = 0.0f;
        releaseEnv = 0.0f;
        releaseCoef = 0.0f;

        targetFreq  = 110.0f;
        currentFreq = 110.0f;
        activeNote = -1;
        gate = false;
        glideActive = false;
        releaseActive = false;
        accentKick = 0.0f;
        accentKickCoef = std::exp (-1.0f / (sr * 0.012f));
        vel  = 0.0f;
        aftertouch = 0.0f;
        heldNotes.clear();

        // filter state
        filterL = {};
        filterR = {};
        filterCharSmoothed.setCurrentAndTargetValue (0.0f);

        // legacy lp not used anymore, but keep zeroed in case you referenced it elsewhere
        lp = 0.0f;
    }

    void setParams (float waveIn, float cutoffIn, float resIn, float envmodIn,
                    float decayIn, float releaseIn, float accentIn, float glideMsIn,
                    float driveIn, float satIn, float subMixIn,
                    float unisonIn, float unisonSpreadIn, float gainIn,
                    int filterCharIn)
    {
        wave    = waveIn;
        cutoff  = cutoffIn;     // Hz
        res     = resIn;        // 0..1
        envmod  = envmodIn;     // 0..1-ish
        decay   = decayIn;      // seconds
        release = releaseIn;    // seconds
        accent  = accentIn;     // 0..1
        glideMs = glideMsIn;    // ms
        drive   = driveIn;      // 0..1
        sat     = satIn;        // 0..1
        subMix  = subMixIn;     // 0..1
        unison  = unisonIn;     // 0..1
        unisonSpread = unisonSpreadIn; // 0..1
        gain    = gainIn;       // linear
        targetFilterChar = juce::jlimit (0, 4, filterCharIn);
        filterCharSmoothed.setTargetValue ((float) targetFilterChar);

        // simple exponential decay envelope coefficient
        const float d = juce::jmax (0.001f, decay);
        envCoef = std::exp (-1.0f / (sr * d));

        if (release <= 0.0001f)
            releaseCoef = 0.0f;
        else
            releaseCoef = std::exp (-1.0f / (sr * release));
    }

    void setModMatrix (int src1, int dst1, float amt1,
                       int src2, int dst2, float amt2,
                       int src3, int dst3, float amt3,
                       float lfo1RateIn, float lfo2RateIn,
                       float modEnvDecayIn)
    {
        slots[0] = { src1, dst1, amt1 };
        slots[1] = { src2, dst2, amt2 };
        slots[2] = { src3, dst3, amt3 };

        lfo1Rate = lfo1RateIn;
        lfo2Rate = lfo2RateIn;
        modEnvDecay = modEnvDecayIn;

        const float d = juce::jmax (0.01f, modEnvDecay);
        modEnvCoef = std::exp (-1.0f / (sr * d));
    }

    void setAftertouch (float pressure)
    {
        aftertouch = juce::jlimit (0.0f, 1.0f, pressure);
    }

    void noteOn (int midiNote, float velocity)
    {
        const float clampedVelocity = juce::jlimit (0.0f, 1.0f, velocity);
        const bool isAccented = (clampedVelocity > 0.7f) && (accent > 0.001f);

        heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                                         [midiNote] (const HeldNote& note) { return note.note == midiNote; }),
                         heldNotes.end());
        heldNotes.push_back ({ midiNote, clampedVelocity });

        const bool wasGate = gate;
        gate = true;
        vel = clampedVelocity;
        targetFreq = juce::MidiMessage::getMidiNoteInHertz (midiNote);
        activeNote = midiNote;
        glideActive = wasGate;
        releaseActive = false;
        releaseEnv = 1.0f;
        if (! wasGate)
        {
            currentFreq = targetFreq;
            phase = 0.0f;
            phaseUnisonA = 0.0f;
            phaseUnisonB = 0.0f;
            phaseSub = 0.0f;
            env = 1.0f; // instant attack for now
            modEnv = 1.0f;
        }
        else if (isAccented)
        {
            // 303-style accented legato "kick" without retriggering envelopes.
            accentKick = 1.0f;
        }
    }

    void noteOff (int midiNote)
    {
        heldNotes.erase (std::remove_if (heldNotes.begin(), heldNotes.end(),
                                         [midiNote] (const HeldNote& note) { return note.note == midiNote; }),
                         heldNotes.end());

        if (midiNote == activeNote)
        {
            if (! heldNotes.empty())
            {
                const auto& next = heldNotes.back();
                activeNote = next.note;
                targetFreq = juce::MidiMessage::getMidiNoteInHertz (next.note);
                vel = next.velocity;
                gate = true;
                glideActive = true;
            }
            else
            {
                gate = false;
                glideActive = false;
                activeNote = -1;
                releaseActive = true;
                releaseEnv = 1.0f;
                if (releaseCoef <= 0.0f)
                    releaseEnv = 0.0f;
            }
        }
        // let envelope decay naturally
    }

    void reset()
    {
        gate = false;
        glideActive = false;
        env = 0.0f;
        modEnv = 0.0f;
        releaseEnv = 0.0f;
        releaseActive = false;
        activeNote = -1;
        heldNotes.clear();
        accentKick = 0.0f;

        phase = 0.0f;
        phaseUnisonA = 0.0f;
        phaseUnisonB = 0.0f;
        phaseSub = 0.0f;

        filterL = {};
        filterR = {};
        filterCharSmoothed.setCurrentAndTargetValue ((float) targetFilterChar);
    }

    std::array<float, 2> renderStereo()
    {
        // --- glide (as in your original code) ---
        const float glideSec  = juce::jmax (0.0f, glideMs) * 0.001f;
        const float glideCoef = (glideSec <= 0.0f) ? 0.0f : std::exp (-1.0f / (sr * glideSec));
        if (glideActive)
            currentFreq = glideCoef * currentFreq + (1.0f - glideCoef) * targetFreq;
        else
            currentFreq = targetFreq;

        // --- modulation sources ---
        lfo1Phase += lfo1Rate / sr;
        lfo2Phase += lfo2Rate / sr;
        if (lfo1Phase >= 1.0f) lfo1Phase -= 1.0f;
        if (lfo2Phase >= 1.0f) lfo2Phase -= 1.0f;

        const float lfo1 = std::sin (juce::MathConstants<float>::twoPi * lfo1Phase);
        const float lfo2 = std::sin (juce::MathConstants<float>::twoPi * (lfo2Phase + 0.25f));

        // --- Accent macro (velocity + Accent parameter) ---
        // Accent shapes multiple targets like a classic 303: cutoff/res/envmod/drive/decay.
        const float accentGate = (vel > 0.7f) ? 1.0f : 0.0f;
        const float accentTarget = juce::jlimit (0.0f, 1.0f, accent * accentGate);
        accentSmoothed.setTargetValue (accentTarget);
        const float accentBase = accentSmoothed.getNextValue();
        const float accentShaped = std::pow (accentBase, 2.2f);

        const float accentKickValue = accentKick;
        accentKick *= accentKickCoef;
        if (accentKick < 1.0e-5f)
            accentKick = 0.0f;

        const float accentTotal = juce::jlimit (0.0f, 1.0f, accentShaped + 0.35f * accentKickValue);

        // --- envelope decay (accent slightly tightens decay times) ---
        const float decayScaled = juce::jmax (0.01f, decay * (1.0f - 0.25f * accentTotal));
        const float modDecayScaled = juce::jmax (0.01f, modEnvDecay * (1.0f - 0.15f * accentTotal));
        const float envCoefLocal = std::exp (-1.0f / (sr * decayScaled));
        const float modEnvCoefLocal = std::exp (-1.0f / (sr * modDecayScaled));
        env *= envCoefLocal;
        modEnv *= modEnvCoefLocal;

        if (gate)
        {
            releaseEnv = 1.0f;
            releaseActive = false;
        }
        else if (releaseActive)
        {
            if (releaseCoef <= 0.0f)
            {
                releaseEnv = 0.0f;
            }
            else
            {
                releaseEnv *= releaseCoef;
                if (releaseEnv < 1.0e-4f)
                    releaseEnv = 0.0f;
            }

            if (releaseEnv == 0.0f)
                releaseActive = false;
        }

        float modCutoff = 0.0f;
        float modPitch = 0.0f;
        float modDrive = 0.0f;
        float modGain = 0.0f;
        float modPan = 0.0f;

        for (const auto& slot : slots)
        {
            const float source = getSourceValue (slot.source, lfo1, lfo2);
            const float amount = slot.amount;

            switch (slot.dest)
            {
                case 1: // Cutoff
                    modCutoff += source * amount * 6000.0f;
                    break;
                case 2: // Pitch
                    modPitch += source * amount * 12.0f;
                    break;
                case 3: // Drive
                    modDrive += source * amount * 0.45f;
                    break;
                case 4: // Gain
                    modGain += source * amount * 0.5f;
                    break;
                case 5: // Pan
                    modPan += source * amount;
                    break;
                default:
                    break;
            }
        }

        const float pitchRatio = std::pow (2.0f, modPitch / 12.0f);
        const float modulatedFreq = currentFreq * pitchRatio;

        // --- oscillator: saw -> square morph (same idea as before) ---
        phase += modulatedFreq / sr;
        if (phase >= 1.0f) phase -= 1.0f;

        auto renderWave = [this] (float p)
        {
            float saw = 2.0f * p - 1.0f;
            float sq  = (p < 0.5f) ? 1.0f : -1.0f;
            return juce::jmap (wave, saw, sq);
        };

        float oscMain = renderWave (phase);

        const float unisonAmt = juce::jlimit (0.0f, 1.0f, unison);
        float oscA = 0.0f;
        float oscB = 0.0f;

        if (unisonAmt > 0.0001f)
        {
            const float detuneCents = 7.0f + 25.0f * unisonAmt;
            const float detuneRatio = std::pow (2.0f, detuneCents / 1200.0f);

            phaseUnisonA += (modulatedFreq * detuneRatio) / sr;
            phaseUnisonB += (modulatedFreq / detuneRatio) / sr;

            if (phaseUnisonA >= 1.0f) phaseUnisonA -= 1.0f;
            if (phaseUnisonB >= 1.0f) phaseUnisonB -= 1.0f;

            oscA = renderWave (phaseUnisonA);
            oscB = renderWave (phaseUnisonB);
        }

        const float subAmt = juce::jlimit (0.0f, 1.0f, subMix);
        if (subAmt > 0.0001f)
        {
            phaseSub += (modulatedFreq * 0.5f) / sr;
            if (phaseSub >= 1.0f) phaseSub -= 1.0f;
        }

        // --- cutoff with envelope modulation (accent opens cutoff and boosts env depth) ---
        // envmod maps to an added cutoff range.
        const float cutoffAccent = cutoff * (1.0f + 1.2f * accentTotal);
        const float envmodAccent = envmod * (1.0f + 0.6f * accentTotal);
        float fc = cutoffAccent + envmodAccent * 5000.0f * env + modCutoff;
        fc = juce::jlimit (20.0f, 16000.0f, fc);

        // --- Ladder-ish resonant 4-pole filter ---
        // Coefficient for one-pole stage: g = 1 - exp(-2*pi*fc/sr)
        const float g = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * fc / sr);
        const float gOs = 1.0f - std::exp (-2.0f * juce::MathConstants<float>::pi * fc / (sr * 2.0f));

        // Resonance amount. This is "by ear" scaling.
        // Make sure the full knob range is audibly effective.
        // Too high without clipping can explode, so we soft-clip the loop.
        const float resAccented = juce::jlimit (0.0f, 0.995f, res + 0.08f * accentTotal);
        const float resNorm = juce::jlimit (0.0f, 1.0f, resAccented);
        const float resCurve = std::pow (resNorm, 1.35f);
        float kBase = juce::jmap (resCurve, 0.0f, 1.0f, 0.0f, 4.8f);

        // Drive: pre-gain into the ladder core
        const float driveAccent = drive * (1.0f + 1.0f * accentTotal);
        float driveAmt = juce::jlimit (0.0f, 1.0f, driveAccent + modDrive);
        float pre = 1.0f + 6.0f * driveAmt;

        float oscLeft = 0.0f;
        float oscRight = 0.0f;

        const float basePan = juce::jlimit (-1.0f, 1.0f, modPan);
        const float spread = juce::jlimit (0.0f, 1.0f, unisonSpread) * (0.35f + 0.65f * unisonAmt);

        const float mainWeight = 1.0f - 0.35f * unisonAmt;
        const float sideWeight = 0.175f * unisonAmt;

        addPanned (oscLeft, oscRight, oscMain * mainWeight, basePan);

        if (unisonAmt > 0.0001f)
        {
            addPanned (oscLeft, oscRight, oscA * sideWeight, basePan - spread);
            addPanned (oscLeft, oscRight, oscB * sideWeight, basePan + spread);
        }

        if (subAmt > 0.0001f)
        {
            const float sub = std::sin (juce::MathConstants<float>::twoPi * phaseSub);
            addPanned (oscLeft, oscRight, subAmt * 0.8f * sub, basePan);
        }

        const float charIndex = filterCharSmoothed.getNextValue();
        const int idx0 = juce::jlimit (0, 4, (int) std::floor (charIndex));
        const int idx1 = juce::jlimit (0, 4, idx0 + 1);
        const float charMix = juce::jlimit (0.0f, 1.0f, charIndex - (float) idx0);

        const auto modeA = getFilterModeSettings (idx0);
        const auto modeB = getFilterModeSettings (idx1);

        auto lerp = [] (float a, float b, float t)
        {
            return a + (b - a) * t;
        };

        FilterModeSettings mode {};
        mode.kScale = lerp (modeA.kScale, modeB.kScale, charMix);
        mode.kGScale = lerp (modeA.kGScale, modeB.kGScale, charMix);
        mode.feedbackDrive = lerp (modeA.feedbackDrive, modeB.feedbackDrive, charMix);
        mode.stageClip = lerp (modeA.stageClip, modeB.stageClip, charMix);
        mode.asym = lerp (modeA.asym, modeB.asym, charMix);
        mode.resComp = lerp (modeA.resComp, modeB.resComp, charMix);
        mode.oversample = modeA.oversample || modeB.oversample;
        mode.clampStages = modeA.clampStages || modeB.clampStages;

        float left = processFilter (oscLeft * pre, kBase, g, gOs, mode, filterL);
        float right = processFilter (oscRight * pre, kBase, g, gOs, mode, filterR);

        // optional output saturation (kept from your original "drive coloration")
        left = std::tanh (left);
        right = std::tanh (right);

        // post-filter drive stage for extra power
        const float satAmt = juce::jlimit (0.0f, 1.0f, sat);
        if (satAmt > 0.0001f)
        {
            left = softClip (left * (1.0f + 8.0f * satAmt));
            right = softClip (right * (1.0f + 8.0f * satAmt));
        }

        // modest accent gain bump (the main accent impact is tone/drive/decay)
        const float acc = 1.0f + 0.2f * accentTotal;

        float outGain = juce::jlimit (0.0f, 2.0f, gain + modGain);

        const float ampEnv = env * releaseEnv;
        left *= ampEnv * acc * outGain;
        right *= ampEnv * acc * outGain;

        // avoid denormals
        if (std::abs (left) < 1e-12f) left = 0.0f;
        if (std::abs (right) < 1e-12f) right = 0.0f;

        return { left, right };
    }

private:
    // Cheap, stable soft clip (faster than tanh in the feedback loop)
    static inline float softClip (float v, float a = 0.8f)
    {
        return v / (1.0f + a * std::abs (v));
    }

    static inline float softClipAsym (float v, float a, float asym)
    {
        const float drive = v * ((v >= 0.0f) ? (1.0f + asym) : (1.0f - asym));
        return softClip (drive, a);
    }

    struct FilterState
    {
        float z1 = 0.0f;
        float z2 = 0.0f;
        float z3 = 0.0f;
        float z4 = 0.0f;
        float lastY = 0.0f;
    };

    struct ModSlot
    {
        int source = 0;
        int dest = 0;
        float amount = 0.0f;
    };

    struct HeldNote
    {
        int note = -1;
        float velocity = 0.0f;
    };

    float getSourceValue (int sourceId, float lfo1, float lfo2) const
    {
        switch (sourceId)
        {
            case 1: return lfo1;
            case 2: return lfo2;
            case 3: return modEnv;
            case 4: return vel;
            case 5: return aftertouch;
            default: return 0.0f;
        }
    }

    static inline void addPanned (float& left, float& right, float value, float pan)
    {
        const float clamped = juce::jlimit (-1.0f, 1.0f, pan);
        const float angle = (clamped + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        const float gL = std::cos (angle);
        const float gR = std::sin (angle);
        left += value * gL;
        right += value * gR;
    }

    struct FilterModeSettings
    {
        float kScale = 1.0f;
        float kGScale = 0.25f;
        float feedbackDrive = 1.0f;
        float stageClip = 0.8f;
        float asym = 0.0f;
        float resComp = 0.0f;
        float inputDrive = 1.0f;
        float outputGain = 1.0f;
        bool oversample = false;
        bool clampStages = false;
    };

    static inline FilterModeSettings getFilterModeSettings (int mode)
    {
        FilterModeSettings settings {};
        switch (mode)
        {
            case 1: // Clean Ladder
                settings.kScale = 0.95f;
                settings.kGScale = 0.35f;
                settings.feedbackDrive = 0.6f;
                settings.stageClip = 0.4f;
                settings.resComp = 0.22f;
                settings.inputDrive = 0.85f;
                settings.outputGain = 1.02f;
                break;
            case 2: // Aggressive
                settings.kScale = 1.1f;
                settings.kGScale = 0.2f;
                settings.feedbackDrive = 1.1f;
                settings.stageClip = 0.9f;
                settings.asym = 0.18f;
                settings.resComp = 0.12f;
                settings.inputDrive = 1.1f;
                settings.outputGain = 0.98f;
                break;
            case 3: // Modern
                settings.kScale = 1.02f;
                settings.kGScale = 0.25f;
                settings.feedbackDrive = 0.85f;
                settings.stageClip = 0.55f;
                settings.resComp = 0.2f;
                settings.inputDrive = 1.0f;
                settings.oversample = true;
                break;
            case 4: // Screech
                settings.kScale = 1.2f;
                settings.kGScale = 0.18f;
                settings.feedbackDrive = 1.6f;
                settings.stageClip = 1.1f;
                settings.asym = 0.32f;
                settings.resComp = 0.08f;
                settings.inputDrive = 1.2f;
                settings.outputGain = 0.95f;
                settings.clampStages = true;
                break;
            case 0: // Classic 303
            default:
                settings.kScale = 1.0f;
                settings.kGScale = 0.35f;
                settings.feedbackDrive = 0.85f;
                settings.stageClip = 0.7f;
                settings.resComp = 0.22f;
                settings.inputDrive = 1.0f;
                settings.outputGain = 0.98f;
                settings.oversample = true;
                break;
        }
        return settings;
    }

    static inline float processFilter (float input, float kBase, float g, float gOs,
                                       const FilterModeSettings& mode, FilterState& state)
    {
        float k = kBase * mode.kScale;
        k *= (1.0f - mode.kGScale * g);

        k = juce::jlimit (0.0f, 4.9f, k);

        const float inputSample = softClip (input * mode.inputDrive, 0.6f);
        auto processSample = [&](float inSample, float gSample)
        {
            float u = inSample;
            u -= k * state.lastY;

            if (mode.asym > 0.001f)
                u = softClipAsym (u * mode.feedbackDrive, mode.stageClip, mode.asym);
            else
                u = softClip (u * mode.feedbackDrive, mode.stageClip);

            state.z1 += gSample * (u  - state.z1);
            state.z2 += gSample * (state.z1 - state.z2);
            state.z3 += gSample * (state.z2 - state.z3);
            state.z4 += gSample * (state.z3 - state.z4);

            if (mode.clampStages)
            {
                state.z1 = juce::jlimit (-3.0f, 3.0f, state.z1);
                state.z2 = juce::jlimit (-3.0f, 3.0f, state.z2);
                state.z3 = juce::jlimit (-3.0f, 3.0f, state.z3);
                state.z4 = juce::jlimit (-3.0f, 3.0f, state.z4);
            }
            else
            {
                if (std::abs (state.z1) < 1e-12f) state.z1 = 0.0f;
                if (std::abs (state.z2) < 1e-12f) state.z2 = 0.0f;
                if (std::abs (state.z3) < 1e-12f) state.z3 = 0.0f;
                if (std::abs (state.z4) < 1e-12f) state.z4 = 0.0f;
            }

            float y = state.z4;
            if (mode.resComp > 0.0f)
                y += (mode.resComp * (1.0f - g)) * (inSample - y);

            if (! std::isfinite (y))
            {
                state = {};
                return 0.0f;
            }

            state.lastY = y;
            return y;
        };

        float y = 0.0f;
        if (mode.oversample)
        {
            y = processSample (inputSample, gOs);
            y = processSample (inputSample, gOs);
        }
        else
        {
            y = processSample (inputSample, g);
        }

        if (std::abs (y) < 1e-12f)
            y = 0.0f;
        return y * mode.outputGain;
    }

    // --- common voice state ---
    float sr = 44100.0f;

    float phase = 0.0f;
    float phaseUnisonA = 0.0f;
    float phaseUnisonB = 0.0f;
    float phaseSub = 0.0f;
    float lfo1Phase = 0.0f;
    float lfo2Phase = 0.0f;

    float env = 0.0f;
    float envCoef = 0.0f;
    float modEnv = 0.0f;
    float modEnvCoef = 0.0f;
    float releaseEnv = 0.0f;
    float releaseCoef = 0.0f;

    float targetFreq  = 110.0f;
    float currentFreq = 110.0f;

    int activeNote = -1;
    bool  gate = false;
    bool glideActive = false;
    bool releaseActive = false;
    float vel  = 0.0f;
    float aftertouch = 0.0f;
    std::vector<HeldNote> heldNotes;
    float accentKick = 0.0f;
    float accentKickCoef = 0.0f;

    // --- params (set via setParams) ---
    float wave    = 0.0f;     // 0..1
    float cutoff  = 800.0f;   // Hz
    float res     = 0.0f;     // 0..1
    float envmod  = 0.5f;     // 0..1-ish
    float decay   = 0.2f;     // seconds
    float release = 0.15f;    // seconds
    float accent  = 0.0f;     // 0..1
    float glideMs = 0.0f;     // ms
    float drive   = 0.0f;     // 0..1
    float sat     = 0.0f;     // 0..1
    float subMix  = 0.0f;     // 0..1
    float unison  = 0.0f;     // 0..1
    float unisonSpread = 0.0f; // 0..1
    float gain    = 0.2f;     // linear
    int targetFilterChar = 0;
    juce::SmoothedValue<float> filterCharSmoothed;
    juce::SmoothedValue<float> accentSmoothed;

    float lfo1Rate = 0.5f;
    float lfo2Rate = 1.25f;
    float modEnvDecay = 0.3f;
    std::array<ModSlot, 3> slots {};

    // --- filter state (ladder-ish) ---
    FilterState filterL;
    FilterState filterR;

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
    juce::dsp::Chorus<float> chorus;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineL { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLineR { 192000 };
    juce::Reverb reverb;
    float currentAftertouch = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessor)
};
