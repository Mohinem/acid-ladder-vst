#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AcidSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AcidSynthAudioProcessorEditor (AcidSynthAudioProcessor&);
    ~AcidSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& processor;

    struct KnobLookAndFeel : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider& slider) override;
    };

    KnobLookAndFeel knobLookAndFeel;

    // On-screen keyboard UI
    juce::MidiKeyboardComponent keyboard;

    // Sliders
    juce::Slider wave, cutoff, res, envmod, decay, accent, glide, drive, sat, sub, unison, gain;

    // Labels (real components, not painted text)
    juce::Label waveLabel, cutoffLabel, resLabel, envmodLabel, decayLabel, accentLabel, glideLabel,
        driveLabel, satLabel, subLabel, unisonLabel, gainLabel;
    juce::Label waveValueLabel, cutoffValueLabel, resValueLabel, envmodValueLabel, decayValueLabel,
        accentValueLabel, glideValueLabel, driveValueLabel, satValueLabel, subValueLabel,
        unisonValueLabel, gainValueLabel;

    // Top bar text
    juce::Label titleLabel;
    juce::Label readoutLabel;

    // Optional group headers (small polish)
    juce::Label groupLeft, groupMid, groupRight;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> aWave, aCutoff, aRes, aEnvmod, aDecay, aAccent, aGlide, aDrive,
        aSat, aSub, aUnison, aGain;

    // Internal helpers
    void setupKnob (juce::Slider& s);
    void setupLabel (juce::Label& l, const juce::String& text);
    void setupValueLabel (juce::Label& l);
    void wireReadout (juce::Slider& s, const juce::String& name, juce::Label& valueLabel);
    void updateReadout (const juce::String& name, const juce::Slider& s);
    static juce::String formatValue (const juce::Slider& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessorEditor)
};
