#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AcidSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    explicit AcidSynthAudioProcessorEditor (AcidSynthAudioProcessor&);
    ~AcidSynthAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider wave, cutoff, res, envmod, decay, accent, glide, drive, gain;
    juce::Label  waveL, cutoffL, resL, envmodL, decayL, accentL, glideL, driveL, gainL;

    std::unique_ptr<SliderAttachment> waveA, cutoffA, resA, envmodA, decayA, accentA, glideA, driveA, gainA;

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessorEditor)
};
