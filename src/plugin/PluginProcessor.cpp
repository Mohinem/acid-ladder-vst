#include "PluginProcessor.h"
#include "PluginEditor.h"

static float getParam (juce::AudioProcessorValueTreeState& apvts, const char* id)
{
    if (auto* v = apvts.getRawParameterValue (id))
        return v->load();

    jassertfalse; // param id typo
    return 0.0f;
}

AcidSynthAudioProcessor::AcidSynthAudioProcessor()
    : juce::AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this, nullptr, "PARAMS", createParams())
{
}

AcidSynthAudioProcessor::~AcidSynthAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout AcidSynthAudioProcessor::createParams()
{
    using namespace juce;

    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto modSources = StringArray { "Off", "LFO 1", "LFO 2", "Mod Env", "Velocity", "Aftertouch" };
    auto modDests = StringArray { "Off", "Cutoff", "Pitch", "Drive", "Gain", "Pan" };

    p.push_back (std::make_unique<AudioParameterFloat> ("wave",   "Wave",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("cutoff", "Cutoff",
                                                        NormalisableRange<float>(20.0f, 18000.0f, 0.0f, 0.35f), 800.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("res",    "Resonance",
                                                        NormalisableRange<float>(0.0f, 0.995f), 0.3f));

    p.push_back (std::make_unique<AudioParameterChoice> ("filterChar", "Filter Character",
                                                         StringArray { "Classic 303", "Clean Ladder", "Aggressive", "Modern", "Screech" }, 0));

    p.push_back (std::make_unique<AudioParameterFloat> ("envmod", "Env Mod",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.55f));

    p.push_back (std::make_unique<AudioParameterFloat> ("decay",  "Decay",
                                                        NormalisableRange<float>(0.01f, 2.0f, 0.0f, 0.4f), 0.18f));

    p.push_back (std::make_unique<AudioParameterFloat> ("accent", "Accent",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.75f));

    p.push_back (std::make_unique<AudioParameterFloat> ("glide",  "Glide(ms)",
                                                        NormalisableRange<float>(0.0f, 500.0f, 0.0f, 0.5f), 80.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("drive",  "Drive",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.2f));

    p.push_back (std::make_unique<AudioParameterFloat> ("sat",    "Saturation",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.1f));

    p.push_back (std::make_unique<AudioParameterFloat> ("sub",    "Sub Mix",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.1f));

    p.push_back (std::make_unique<AudioParameterFloat> ("unison", "Unison",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("unisonSpread", "Unison Spread",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.2f));

    p.push_back (std::make_unique<AudioParameterFloat> ("gain",   "Gain",
                                                        NormalisableRange<float>(0.0f, 1.5f), 0.85f));

    p.push_back (std::make_unique<AudioParameterFloat> ("lfo1Rate", "LFO 1 Rate",
                                                        NormalisableRange<float>(0.0f, 15.0f, 0.0f, 0.5f), 2.2f));

    p.push_back (std::make_unique<AudioParameterFloat> ("lfo2Rate", "LFO 2 Rate",
                                                        NormalisableRange<float>(0.0f, 15.0f, 0.0f, 0.5f), 4.8f));

    p.push_back (std::make_unique<AudioParameterFloat> ("modEnvDecay", "Mod Env Decay",
                                                        NormalisableRange<float>(0.0f, 2.5f, 0.0f, 0.4f), 0.55f));

    p.push_back (std::make_unique<AudioParameterChoice> ("mod1Source", "Mod 1 Source", modSources, 0));
    p.push_back (std::make_unique<AudioParameterChoice> ("mod1Dest", "Mod 1 Dest", modDests, 0));
    p.push_back (std::make_unique<AudioParameterFloat> ("mod1Amount", "Mod 1 Amount",
                                                        NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<AudioParameterChoice> ("mod2Source", "Mod 2 Source", modSources, 0));
    p.push_back (std::make_unique<AudioParameterChoice> ("mod2Dest", "Mod 2 Dest", modDests, 0));
    p.push_back (std::make_unique<AudioParameterFloat> ("mod2Amount", "Mod 2 Amount",
                                                        NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<AudioParameterChoice> ("mod3Source", "Mod 3 Source", modSources, 0));
    p.push_back (std::make_unique<AudioParameterChoice> ("mod3Dest", "Mod 3 Dest", modDests, 0));
    p.push_back (std::make_unique<AudioParameterFloat> ("mod3Amount", "Mod 3 Amount",
                                                        NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("fxDrive", "FX Drive",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.15f));

    p.push_back (std::make_unique<AudioParameterFloat> ("fxChorus", "Chorus",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.18f));

    p.push_back (std::make_unique<AudioParameterFloat> ("fxDelay", "Delay",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.2f));

    p.push_back (std::make_unique<AudioParameterFloat> ("fxDelayTime", "Delay Time",
                                                        NormalisableRange<float>(5.0f, 700.0f, 0.0f, 0.5f), 260.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("fxReverb", "Reverb",
                                                        NormalisableRange<float>(0.0f, 1.0f), 0.25f));

    return { p.begin(), p.end() };
}

bool AcidSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void AcidSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voice.prepare (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (juce::jmax (1, getTotalNumOutputChannels()));

    chorus.prepare (spec);
    chorus.setCentreDelay (12.0f);
    chorus.setFeedback (0.18f);

    delayLineL.reset();
    delayLineR.reset();
    delayLineL.prepare (spec);
    delayLineR.prepare (spec);

    reverb.reset();
}

void AcidSynthAudioProcessor::releaseResources()
{
}

void AcidSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // ✅ THIS is the critical line that makes the on-screen keyboard generate MIDI:
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);

    // parameters
    const float wave   = getParam (apvts, "wave");
    const float cutoff = getParam (apvts, "cutoff");
    const float res    = getParam (apvts, "res");
    const int filterChar = (int) getParam (apvts, "filterChar");
    const float envmod = getParam (apvts, "envmod");
    const float decay  = getParam (apvts, "decay");
    const float accent = getParam (apvts, "accent");
    const float glide  = getParam (apvts, "glide");
    const float drive  = getParam (apvts, "drive");
    const float sat    = getParam (apvts, "sat");
    const float subMix = getParam (apvts, "sub");
    const float unison = getParam (apvts, "unison");
    const float unisonSpread = getParam (apvts, "unisonSpread");
    const float gain   = getParam (apvts, "gain");

    const float lfo1Rate = getParam (apvts, "lfo1Rate");
    const float lfo2Rate = getParam (apvts, "lfo2Rate");
    const float modEnvDecay = getParam (apvts, "modEnvDecay");

    const int mod1Source = (int) getParam (apvts, "mod1Source");
    const int mod1Dest = (int) getParam (apvts, "mod1Dest");
    const float mod1Amount = getParam (apvts, "mod1Amount");

    const int mod2Source = (int) getParam (apvts, "mod2Source");
    const int mod2Dest = (int) getParam (apvts, "mod2Dest");
    const float mod2Amount = getParam (apvts, "mod2Amount");

    const int mod3Source = (int) getParam (apvts, "mod3Source");
    const int mod3Dest = (int) getParam (apvts, "mod3Dest");
    const float mod3Amount = getParam (apvts, "mod3Amount");

    const float fxDrive = getParam (apvts, "fxDrive");
    const float fxChorus = getParam (apvts, "fxChorus");
    const float fxDelay = getParam (apvts, "fxDelay");
    const float fxDelayTime = getParam (apvts, "fxDelayTime");
    const float fxReverb = getParam (apvts, "fxReverb");

    voice.setParams (wave, cutoff, res, envmod, decay, accent, glide, drive, sat, subMix, unison, unisonSpread, gain, filterChar);
    voice.setModMatrix (mod1Source, mod1Dest, mod1Amount,
                        mod2Source, mod2Dest, mod2Amount,
                        mod3Source, mod3Dest, mod3Amount,
                        lfo1Rate, lfo2Rate, modEnvDecay);

    // handle MIDI (mono)
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
            voice.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            voice.noteOff (msg.getNoteNumber());
        else if (msg.isChannelPressure())
            currentAftertouch = msg.getChannelPressureValue() / 127.0f;
        else if (msg.isAftertouch())
            currentAftertouch = msg.getAfterTouchValue() / 127.0f;
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            voice.reset();
    }

    voice.setAftertouch (currentAftertouch);

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    if (numCh == 0 || numSamples == 0)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto s = voice.renderStereo();
        if (numCh == 1)
            buffer.setSample (0, i, 0.5f * (s[0] + s[1]));
        else
        {
            buffer.setSample (0, i, s[0]);
            buffer.setSample (1, i, s[1]);
        }
    }

    const float distortionMix = juce::jlimit (0.0f, 1.0f, fxDrive);
    if (distortionMix > 0.0001f)
    {
        const float driveAmt = 1.0f + 10.0f * distortionMix;
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* data = buffer.getWritePointer (ch);
            for (int i = 0; i < numSamples; ++i)
            {
                const float dry = data[i];
                const float wet = std::tanh (dry * driveAmt);
                data[i] = dry + (wet - dry) * distortionMix;
            }
        }
    }

    const float chorusMix = juce::jlimit (0.0f, 1.0f, fxChorus);
    if (chorusMix > 0.0001f)
    {
        chorus.setRate (0.25f + 3.2f * chorusMix);
        chorus.setDepth (0.25f + 0.55f * chorusMix);
        chorus.setMix (chorusMix);

        auto block = juce::dsp::AudioBlock<float> (buffer);
        auto context = juce::dsp::ProcessContextReplacing<float> (block);
        chorus.process (context);
    }

    const float delayMix = juce::jlimit (0.0f, 1.0f, fxDelay);
    if (delayMix > 0.0001f)
    {
        const float delaySamples = (float) getSampleRate() * (fxDelayTime * 0.001f);
        delayLineL.setDelay (delaySamples);
        delayLineR.setDelay (delaySamples);

        const float feedback = 0.35f + 0.25f * delayMix;
        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = buffer.getSample (0, i);
            const float inR = (numCh > 1) ? buffer.getSample (1, i) : inL;

            const float dl = delayLineL.popSample (0);
            const float dr = delayLineR.popSample (0);

            delayLineL.pushSample (0, inL + dl * feedback);
            delayLineR.pushSample (0, inR + dr * feedback);

            buffer.setSample (0, i, inL + dl * delayMix);
            if (numCh > 1)
                buffer.setSample (1, i, inR + dr * delayMix);
        }
    }

    const float reverbMix = juce::jlimit (0.0f, 1.0f, fxReverb);
    if (reverbMix > 0.0001f)
    {
        juce::Reverb::Parameters params;
        params.roomSize = 0.2f + 0.7f * reverbMix;
        params.damping = 0.45f + 0.35f * reverbMix;
        params.wetLevel = 0.12f + 0.35f * reverbMix;
        params.dryLevel = 1.0f - 0.2f * reverbMix;
        params.width = 1.0f;
        reverb.setParameters (params);

        if (numCh > 1)
        {
            reverb.processStereo (buffer.getWritePointer (0),
                                  buffer.getWritePointer (1),
                                  numSamples);
        }
        else
        {
            reverb.processMono (buffer.getWritePointer (0), numSamples);
        }
    }
}

bool AcidSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AcidSynthAudioProcessor::createEditor()
{
    return new AcidSynthAudioProcessorEditor (*this);
}

void AcidSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AcidSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidSynthAudioProcessor();
}
