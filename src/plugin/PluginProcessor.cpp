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

    p.push_back (std::make_unique<AudioParameterFloat> ("wave",   "Wave",
                                                        NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("cutoff", "Cutoff",
                                                        NormalisableRange<float>(20.0f, 18000.0f, 0.0f, 0.35f), 800.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("res",    "Resonance",
                                                        NormalisableRange<float>(0.0f, 0.995f, 0.0001f), 0.35f));

    p.push_back (std::make_unique<AudioParameterFloat> ("envmod", "Env Mod",
                                                        NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.75f));

    p.push_back (std::make_unique<AudioParameterFloat> ("decay",  "Decay",
                                                        NormalisableRange<float>(0.01f, 2.0f, 0.001f, 0.4f), 0.18f));

    p.push_back (std::make_unique<AudioParameterFloat> ("accent", "Accent",
                                                        NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.75f));

    p.push_back (std::make_unique<AudioParameterFloat> ("glide",  "Glide(ms)",
                                                        NormalisableRange<float>(0.0f, 500.0f, 0.1f, 0.5f), 80.0f));

    p.push_back (std::make_unique<AudioParameterFloat> ("drive",  "Drive",
                                                        NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));

    p.push_back (std::make_unique<AudioParameterFloat> ("gain",   "Gain",
                                                        NormalisableRange<float>(0.0f, 1.5f, 0.001f), 0.85f));

    return { p.begin(), p.end() };
}

bool AcidSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void AcidSynthAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    voice.prepare (sampleRate);
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
    const float envmod = getParam (apvts, "envmod");
    const float decay  = getParam (apvts, "decay");
    const float accent = getParam (apvts, "accent");
    const float glide  = getParam (apvts, "glide");
    const float drive  = getParam (apvts, "drive");
    const float gain   = getParam (apvts, "gain");

    voice.setParams (wave, cutoff, res, envmod, decay, accent, glide, drive, gain);

    // handle MIDI (mono)
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
            voice.noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            voice.noteOff (msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            voice.reset();
    }

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        const float s = voice.render();
        for (int ch = 0; ch < numCh; ++ch)
            buffer.setSample (ch, i, s);
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
