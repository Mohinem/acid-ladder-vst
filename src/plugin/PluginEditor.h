#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AcidSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AcidSynthAudioProcessorEditor (AcidSynthAudioProcessor&);
    ~AcidSynthAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& processor;

    // On-screen keyboard UI
    juce::MidiKeyboardComponent keyboard;

    // Sliders
    juce::Slider wave, cutoff, res, envmod, decay, accent, glide, drive, sat, sub, unison, gain;

    // Labels (real components, not painted text)
    juce::Label waveLabel, cutoffLabel, resLabel, envmodLabel, decayLabel, accentLabel, glideLabel,
        driveLabel, satLabel, subLabel, unisonLabel, gainLabel;

    // Top bar text
    juce::Label titleLabel;
    juce::Label readoutLabel;

    // Optional group headers (small polish)
    juce::Label groupLeft, groupMid, groupRight;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> aWave, aCutoff, aRes, aEnvmod, aDecay, aAccent, aGlide, aDrive,
        aSat, aSub, aUnison, aGain;

    // Internal helpers
    static void setupKnob (juce::Slider& s);
    void setupLabel (juce::Label& l, const juce::String& text);
    void wireReadout (juce::Slider& s, const juce::String& name);
    void updateReadout (const juce::String& name, const juce::Slider& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessorEditor)
};
