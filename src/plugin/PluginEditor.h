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
    juce::Slider wave, cutoff, res, envmod, decay, release, accent, glide, drive, sat, sub, unison, unisonSpread, gain;
    juce::Slider lfo1Rate, lfo2Rate, modEnvDecay;
    juce::Slider mod1Amount, mod2Amount, mod3Amount;
    juce::Slider fxDrive, fxChorus, fxDelay, fxDelayTime, fxReverb;
    juce::ComboBox filterChar;

    // Labels (real components, not painted text)
    juce::Label waveLabel, cutoffLabel, resLabel, envmodLabel, decayLabel, releaseLabel, accentLabel, glideLabel,
        driveLabel, satLabel, subLabel, unisonLabel, unisonSpreadLabel, gainLabel;
    juce::Label filterCharLabel;
    juce::Label lfo1RateLabel, lfo2RateLabel, modEnvDecayLabel;
    juce::Label mod1AmountLabel, mod2AmountLabel, mod3AmountLabel;
    juce::Label fxDriveLabel, fxChorusLabel, fxDelayLabel, fxDelayTimeLabel, fxReverbLabel;
    juce::Label waveValueLabel, cutoffValueLabel, resValueLabel, envmodValueLabel, decayValueLabel, releaseValueLabel,
        accentValueLabel, glideValueLabel, driveValueLabel, satValueLabel, subValueLabel,
        unisonValueLabel, unisonSpreadValueLabel, gainValueLabel;
    juce::Label lfo1RateValueLabel, lfo2RateValueLabel, modEnvDecayValueLabel;
    juce::Label mod1AmountValueLabel, mod2AmountValueLabel, mod3AmountValueLabel;
    juce::Label fxDriveValueLabel, fxChorusValueLabel, fxDelayValueLabel, fxDelayTimeValueLabel, fxReverbValueLabel;

    // Top bar text
    juce::Label titleLabel;
    juce::Label readoutLabel;

    // Optional group headers (small polish)
    juce::Label groupLeft, groupMid, groupRight, groupExtra;
    juce::Label modHeaderLabel, fxHeaderLabel, modSourceHeader, modAmountHeader, modDestHeader;
    juce::Label modRow1Label, modRow2Label, modRow3Label;

    juce::ComboBox mod1Source, mod1Dest, mod2Source, mod2Dest, mod3Source, mod3Dest;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<Attachment> aWave, aCutoff, aRes, aEnvmod, aDecay, aRelease, aAccent, aGlide, aDrive,
        aSat, aSub, aUnison, aUnisonSpread, aGain;
    std::unique_ptr<Attachment> aLfo1Rate, aLfo2Rate, aModEnvDecay;
    std::unique_ptr<Attachment> aMod1Amount, aMod2Amount, aMod3Amount;
    std::unique_ptr<Attachment> aFxDrive, aFxChorus, aFxDelay, aFxDelayTime, aFxReverb;
    std::unique_ptr<ComboAttachment> aMod1Source, aMod1Dest, aMod2Source, aMod2Dest, aMod3Source, aMod3Dest;
    std::unique_ptr<ComboAttachment> aFilterChar;

    // Internal helpers
    void setupKnob (juce::Slider& s);
    void setupLabel (juce::Label& l, const juce::String& text);
    void setupValueLabel (juce::Label& l);
    void wireReadout (juce::Slider& s, const juce::String& name, juce::Label& valueLabel);
    void updateReadout (const juce::String& name, const juce::Slider& s);
    static juce::String formatValue (const juce::Slider& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessorEditor)
};
