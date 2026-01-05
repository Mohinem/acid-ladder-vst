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

    struct AttachmentBank
    {
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliders;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> combos;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttons;
    };

    struct KnobLookAndFeel : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider& slider) override;
        void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               const juce::Slider::SliderStyle, juce::Slider& slider) override;
    };

    KnobLookAndFeel knobLookAndFeel;

    struct Section : public juce::Component
    {
        explicit Section (AcidSynthAudioProcessorEditor& editorIn) : editor (editorIn) {}
        void registerControl (juce::Component& c, const juce::String& debugName);

    protected:
        AcidSynthAudioProcessorEditor& editor;
    };

    struct TopBarSection : public Section
    {
        explicit TopBarSection (AcidSynthAudioProcessorEditor& editorIn);
        void resized() override;

        juce::Label titleLabel;
        juce::Label readoutLabel;
    };

    struct KeyboardSection : public Section
    {
        KeyboardSection (AcidSynthAudioProcessorEditor& editorIn, juce::MidiKeyboardState& state);
        void resized() override;

        juce::MidiKeyboardComponent keyboard;
    };

    struct KnobPanelSection : public Section
    {
        explicit KnobPanelSection (AcidSynthAudioProcessorEditor& editorIn);
        void resized() override;

        // Sliders
        juce::Slider wave, cutoff, res, envmod, decay, release, accent, glide, drive, sat, sub, unison, unisonSpread, gain, volume;

        // Combos
        juce::ComboBox filterChar;

        // Labels
        juce::Label waveLabel, cutoffLabel, resLabel, envmodLabel, decayLabel, releaseLabel, accentLabel, glideLabel,
            driveLabel, satLabel, subLabel, unisonLabel, unisonSpreadLabel, gainLabel, volumeLabel;
        juce::Label filterCharLabel;
        juce::Label waveValueLabel, cutoffValueLabel, resValueLabel, envmodValueLabel, decayValueLabel, releaseValueLabel,
            accentValueLabel, glideValueLabel, driveValueLabel, satValueLabel, subValueLabel,
            unisonValueLabel, unisonSpreadValueLabel, gainValueLabel, volumeValueLabel;

        // Group headers
        juce::Label groupLeft, groupMid, groupRight, groupExtra;
    };

    struct ModMatrixSection : public Section
    {
        explicit ModMatrixSection (AcidSynthAudioProcessorEditor& editorIn);
        void resized() override;

        juce::Label modHeaderLabel, modSourceHeader, modAmountHeader, modDestHeader;
        juce::Label modRow1Label, modRow2Label, modRow3Label;
        juce::ComboBox mod1Source, mod1Dest, mod2Source, mod2Dest, mod3Source, mod3Dest;
        juce::Slider mod1Amount, mod2Amount, mod3Amount;
        juce::Label mod1AmountLabel, mod2AmountLabel, mod3AmountLabel;
        juce::Label mod1AmountValueLabel, mod2AmountValueLabel, mod3AmountValueLabel;
    };

    struct FxSection : public Section
    {
        explicit FxSection (AcidSynthAudioProcessorEditor& editorIn);
        void resized() override;

        juce::Label fxHeaderLabel;
        juce::Slider lfo1Rate, lfo2Rate, modEnvDecay;
        juce::ComboBox lfo1Mode, lfo1Sync, lfo2Mode, lfo2Sync;
        juce::Label lfo1RateLabel, lfo2RateLabel, modEnvDecayLabel;
        juce::Label lfo1ModeLabel, lfo1SyncLabel, lfo2ModeLabel, lfo2SyncLabel;
        juce::Label lfo1RateValueLabel, lfo2RateValueLabel, modEnvDecayValueLabel;

        juce::Slider fxDrive, fxChorus, fxDelay, fxDelayTime, fxReverb;
        juce::Label fxDriveLabel, fxChorusLabel, fxDelayLabel, fxDelayTimeLabel, fxReverbLabel;
        juce::Label fxDriveValueLabel, fxChorusValueLabel, fxDelayValueLabel, fxDelayTimeValueLabel, fxReverbValueLabel;
    };

    TopBarSection topBar;
    KeyboardSection keyboardSection;
    KnobPanelSection knobPanel;
    ModMatrixSection modMatrixPanel;
    FxSection fxPanel;

    // Internal helpers
    void setupKnob (juce::Slider& s);
    void setupLabel (juce::Label& l, const juce::String& text);
    void setupValueLabel (juce::Label& l);
    void setupCombo (juce::ComboBox& combo);
    void wireReadout (juce::Slider& s, const juce::String& name, juce::Label& valueLabel);
    void updateReadout (const juce::String& name, const juce::Slider& s);
    static juce::String formatValue (const juce::Slider& s);
    void registerControl (juce::Component& c, const juce::String& debugName, juce::Component* parentOverride = nullptr);
    void layoutGuard();
    void addSliderAttachment (const juce::String& paramId, juce::Slider& slider);
    void addComboAttachment (const juce::String& paramId, juce::ComboBox& combo);

    std::vector<juce::Component*> registeredControls;
    AttachmentBank attachments;

    friend struct Section;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AcidSynthAudioProcessorEditor)
};
