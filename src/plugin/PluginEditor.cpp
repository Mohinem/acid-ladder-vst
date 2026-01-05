// src/plugin/PluginEditor.cpp
#include "PluginEditor.h"

namespace
{
    const juce::Colour kBgTop (0xff111419);
    const juce::Colour kBgBottom (0xff0a0b0d);
    const juce::Colour kPanelTop (0xff1b2128);
    const juce::Colour kPanelBottom (0xff14181d);
    const juce::Colour kPanelEdge (0xff2a323a);
    const juce::Colour kPanelInner (0xff222830);
    const juce::Colour kTextPrimary (0xffe6ecf2);
    const juce::Colour kTextSecondary (0xffc3cbd4);
    const juce::Colour kTextMuted (0xff8f98a3);
    const juce::Colour kAccent (0xff3db7ff);
    const juce::Colour kAccentGlow (0xff7fd4ff);
    const juce::Colour kControlBg (0xff171b20);
    const juce::Colour kControlEdge (0xff303740);

    constexpr float kPanelCorner = 12.0f;
}

AcidSynthAudioProcessorEditor::~AcidSynthAudioProcessorEditor()
{
    for (auto* s : { &wave, &cutoff, &res, &envmod, &decay, &release, &accent, &glide, &drive, &sat, &sub, &unison,
                     &unisonSpread, &gain, &volume, &lfo1Rate, &lfo2Rate, &modEnvDecay, &mod1Amount, &mod2Amount, &mod3Amount,
                     &fxDrive, &fxChorus, &fxDelay, &fxDelayTime, &fxReverb })
        s->setLookAndFeel (nullptr);
}

//==============================================================================
// Knob styling (minimal but more "pro" feeling)
void AcidSynthAudioProcessorEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::LinearHorizontal);

    // Remove the clunky numeric boxes (prototype look)
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    s.setColour (juce::Slider::backgroundColourId, kControlBg);
    s.setColour (juce::Slider::trackColourId, kAccent);
    s.setColour (juce::Slider::thumbColourId, kTextPrimary);
    s.setColour (juce::Slider::rotarySliderFillColourId, kAccent);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, kControlEdge);

    s.setLookAndFeel (&knobLookAndFeel);

    // Direct horizontal dragging for slider-style controls
    s.setVelocityBasedMode (false);
    s.setMouseDragSensitivity (110);

    // Mouse wheel tweaks
    s.setScrollWheelEnabled (true);

    // Hover/drag value popup for usability
    s.setPopupDisplayEnabled (true, true, nullptr);
}

void AcidSynthAudioProcessorEditor::KnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                                       float sliderPosProportional, float rotaryStartAngle,
                                                                       float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centre = bounds.getCentre();
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    auto rimBounds = bounds.reduced (1.5f);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.fillEllipse (rimBounds);

    auto faceBounds = rimBounds.reduced (5.5f);
    juce::ColourGradient faceGradient (kPanelTop, faceBounds.getCentreX(), faceBounds.getY(),
                                       kPanelBottom, faceBounds.getCentreX(), faceBounds.getBottom(), false);
    g.setGradientFill (faceGradient);
    g.fillEllipse (faceBounds);

    g.setColour (kPanelEdge);
    g.drawEllipse (rimBounds, 1.2f);

    auto arcRadius = radius - 5.0f;
    juce::Path backgroundArc;
    backgroundArc.addArc (centre.x - arcRadius, centre.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (kControlEdge);
    g.strokePath (backgroundArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addArc (centre.x - arcRadius, centre.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                     rotaryStartAngle, angle, true);
    auto valueColour = slider.isEnabled() ? kAccent : juce::Colours::grey;
    g.setColour (valueColour.withAlpha (0.35f));
    g.strokePath (valueArc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (valueColour);
    g.strokePath (valueArc, juce::PathStrokeType (3.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    auto pointerLength = arcRadius - 6.0f;
    auto pointerThickness = 2.4f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);

    g.setColour (kTextPrimary);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

    auto capBounds = juce::Rectangle<float> (centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f);
    g.setColour (kTextSecondary);
    g.fillEllipse (capBounds);
}

void AcidSynthAudioProcessorEditor::KnobLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                                       float sliderPos, float, float,
                                                                       const juce::Slider::SliderStyle style,
                                                                       juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0.0f, 0.0f, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
    auto trackHeight = juce::jmin (6.0f, bounds.getHeight() * 0.4f);
    auto track = bounds.withHeight (trackHeight).withCentre (bounds.getCentre());

    auto bgColour = slider.findColour (juce::Slider::backgroundColourId);
    auto trackColour = slider.findColour (juce::Slider::trackColourId);
    auto thumbColour = slider.findColour (juce::Slider::thumbColourId);

    g.setColour (bgColour);
    g.fillRoundedRectangle (track, trackHeight * 0.5f);

    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRoundedRectangle (track, trackHeight * 0.5f, 1.0f);

    auto filled = track.withWidth (juce::jlimit (0.0f, track.getWidth(), sliderPos - track.getX()));
    juce::ColourGradient fillGradient (trackColour.brighter (0.2f), filled.getX(), filled.getCentreY(),
                                       kAccentGlow, filled.getRight(), filled.getCentreY(), false);
    g.setGradientFill (fillGradient);
    g.fillRoundedRectangle (filled, trackHeight * 0.5f);

    auto thumbRadius = trackHeight * 0.9f;
    auto thumbCentre = juce::Point<float> (sliderPos, track.getCentreY());
    g.setColour (thumbColour);
    g.fillEllipse (juce::Rectangle<float> (thumbRadius * 2.0f, thumbRadius * 2.0f).withCentre (thumbCentre));
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.drawEllipse (juce::Rectangle<float> (thumbRadius * 2.0f, thumbRadius * 2.0f).withCentre (thumbCentre), 1.0f);
}

//==============================================================================
// Label styling
void AcidSynthAudioProcessorEditor::setupLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions (11.5f, juce::Font::bold)));
    l.setColour (juce::Label::textColourId, kTextSecondary);
    l.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (l);
}

void AcidSynthAudioProcessorEditor::setupValueLabel (juce::Label& l)
{
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::plain)));
    l.setColour (juce::Label::textColourId, kTextMuted);
    l.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (l);
}

//==============================================================================
// Readout formatting
void AcidSynthAudioProcessorEditor::updateReadout (const juce::String& name, const juce::Slider& s)
{
    readoutLabel.setText (name + ": " + formatValue (s), juce::dontSendNotification);
}

juce::String AcidSynthAudioProcessorEditor::formatValue (const juce::Slider& s)
{
    const auto v = s.getValue();

    if (std::abs (v) >= 1000.0)
        return juce::String (v, 1);
    if (std::abs (v) >= 100.0)
        return juce::String (v, 1);
    if (std::abs (v) >= 10.0)
        return juce::String (v, 2);
    return juce::String (v, 3);
}

//==============================================================================
// Connect slider interactions to top readout
void AcidSynthAudioProcessorEditor::wireReadout (juce::Slider& s, const juce::String& name, juce::Label& valueLabel)
{
    s.setTooltip (name);

    s.onDragStart = [this, &s, name]
    {
        updateReadout (name, s);
    };

    s.onValueChange = [this, &s, name, &valueLabel]
    {
        updateReadout (name, s);
        valueLabel.setText (formatValue (s), juce::dontSendNotification);
    };
}

//==============================================================================
// ctor
AcidSynthAudioProcessorEditor::AcidSynthAudioProcessorEditor (AcidSynthAudioProcessor& p)
    : juce::AudioProcessorEditor (&p)
    , processor (p)
    , keyboard (processor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // --- Sliders
    setupKnob (wave);
    setupKnob (cutoff);
    setupKnob (res);
    setupKnob (envmod);
    setupKnob (decay);
    setupKnob (release);
    setupKnob (accent);
    setupKnob (glide);
    setupKnob (drive);
    setupKnob (sat);
    setupKnob (sub);
    setupKnob (unison);
    setupKnob (unisonSpread);
    setupKnob (gain);
    setupKnob (volume);
    setupKnob (lfo1Rate);
    setupKnob (lfo2Rate);
    setupKnob (modEnvDecay);
    setupKnob (mod1Amount);
    setupKnob (mod2Amount);
    setupKnob (mod3Amount);
    setupKnob (fxDrive);
    setupKnob (fxChorus);
    setupKnob (fxDelay);
    setupKnob (fxDelayTime);
    setupKnob (fxReverb);

    addAndMakeVisible (wave);
    addAndMakeVisible (cutoff);
    addAndMakeVisible (res);
    addAndMakeVisible (envmod);
    addAndMakeVisible (decay);
    addAndMakeVisible (release);
    addAndMakeVisible (accent);
    addAndMakeVisible (glide);
    addAndMakeVisible (drive);
    addAndMakeVisible (sat);
    addAndMakeVisible (sub);
    addAndMakeVisible (unison);
    addAndMakeVisible (unisonSpread);
    addAndMakeVisible (gain);
    addAndMakeVisible (volume);
    addAndMakeVisible (lfo1Rate);
    addAndMakeVisible (lfo2Rate);
    addAndMakeVisible (modEnvDecay);
    addAndMakeVisible (mod1Amount);
    addAndMakeVisible (mod2Amount);
    addAndMakeVisible (mod3Amount);
    addAndMakeVisible (fxDrive);
    addAndMakeVisible (fxChorus);
    addAndMakeVisible (fxDelay);
    addAndMakeVisible (fxDelayTime);
    addAndMakeVisible (fxReverb);

    for (auto* combo : { &lfo1Mode, &lfo1Sync, &lfo2Mode, &lfo2Sync })
    {
        combo->setColour (juce::ComboBox::backgroundColourId, kControlBg);
        combo->setColour (juce::ComboBox::textColourId, kTextPrimary);
        combo->setColour (juce::ComboBox::outlineColourId, kControlEdge);
        combo->setColour (juce::ComboBox::focusedOutlineColourId, kAccent);
        combo->setColour (juce::ComboBox::arrowColourId, kTextSecondary);
        addAndMakeVisible (*combo);
    }

    for (auto* combo : { &mod1Source, &mod1Dest, &mod2Source, &mod2Dest, &mod3Source, &mod3Dest })
    {
        combo->setColour (juce::ComboBox::backgroundColourId, kControlBg);
        combo->setColour (juce::ComboBox::textColourId, kTextPrimary);
        combo->setColour (juce::ComboBox::outlineColourId, kControlEdge);
        combo->setColour (juce::ComboBox::focusedOutlineColourId, kAccent);
        combo->setColour (juce::ComboBox::arrowColourId, kTextSecondary);
        addAndMakeVisible (*combo);
    }

    filterChar.setColour (juce::ComboBox::backgroundColourId, kControlBg);
    filterChar.setColour (juce::ComboBox::textColourId, kTextPrimary);
    filterChar.setColour (juce::ComboBox::outlineColourId, kControlEdge);
    filterChar.setColour (juce::ComboBox::focusedOutlineColourId, kAccent);
    filterChar.setColour (juce::ComboBox::arrowColourId, kTextSecondary);
    addAndMakeVisible (filterChar);

    auto addModItems = [] (juce::ComboBox& combo, const juce::StringArray& items)
    {
        for (int i = 0; i < items.size(); ++i)
            combo.addItem (items[i], i + 1);
    };

    const juce::StringArray modSources { "Off", "LFO 1", "LFO 2", "Mod Env", "Velocity", "Aftertouch" };
    const juce::StringArray modDests { "Off", "Cutoff", "Pitch", "Drive", "Gain", "Pan" };
    const juce::StringArray filterChars { "Classic 303", "Clean Ladder", "Aggressive", "Modern", "Screech" };
    const juce::StringArray lfoModes { "Free", "Sync" };
    const juce::StringArray lfoSyncs { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/8D", "1/16D", "1/8T", "1/16T" };

    addModItems (mod1Source, modSources);
    addModItems (mod2Source, modSources);
    addModItems (mod3Source, modSources);

    addModItems (mod1Dest, modDests);
    addModItems (mod2Dest, modDests);
    addModItems (mod3Dest, modDests);

    addModItems (filterChar, filterChars);
    addModItems (lfo1Mode, lfoModes);
    addModItems (lfo2Mode, lfoModes);
    addModItems (lfo1Sync, lfoSyncs);
    addModItems (lfo2Sync, lfoSyncs);

    mod1Source.setTooltip ("Mod 1 Source");
    mod1Dest.setTooltip ("Mod 1 Destination");
    mod2Source.setTooltip ("Mod 2 Source");
    mod2Dest.setTooltip ("Mod 2 Destination");
    mod3Source.setTooltip ("Mod 3 Source");
    mod3Dest.setTooltip ("Mod 3 Destination");
    filterChar.setTooltip ("Filter Character");
    lfo1Mode.setTooltip ("LFO 1 Mode");
    lfo1Sync.setTooltip ("LFO 1 Sync");
    lfo2Mode.setTooltip ("LFO 2 Mode");
    lfo2Sync.setTooltip ("LFO 2 Sync");

    // --- Keyboard
    addAndMakeVisible (keyboard);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffe1e6ec));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff1a1f25));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, kPanelEdge);
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, kTextMuted);
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, kAccent.withAlpha (0.18f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, kAccent.withAlpha (0.45f));

    // --- Top bar labels
    titleLabel.setText ("Acid Ladder VST", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, kTextPrimary);
    titleLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (titleLabel);

    readoutLabel.setText ("", juce::dontSendNotification);
    readoutLabel.setJustificationType (juce::Justification::centredRight);
    readoutLabel.setFont (juce::Font (juce::FontOptions (12.5f, juce::Font::plain)));
    readoutLabel.setColour (juce::Label::textColourId, kAccentGlow);
    readoutLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (readoutLabel);

    // --- Group headers
    groupLeft.setText ("OSC", juce::dontSendNotification);
    groupMid.setText  ("FILTER", juce::dontSendNotification);
    groupRight.setText("AMP", juce::dontSendNotification);
    groupExtra.setText("MIX", juce::dontSendNotification);

    for (auto* gl : { &groupLeft, &groupMid, &groupRight, &groupExtra })
    {
        gl->setJustificationType (juce::Justification::centred);
        gl->setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
        gl->setColour (juce::Label::textColourId, kTextMuted);
        gl->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*gl);
    }

    modHeaderLabel.setText ("MOD MATRIX", juce::dontSendNotification);
    modHeaderLabel.setJustificationType (juce::Justification::centredLeft);
    modHeaderLabel.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    modHeaderLabel.setColour (juce::Label::textColourId, kTextSecondary);
    modHeaderLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (modHeaderLabel);

    fxHeaderLabel.setText ("FX + MOD SOURCES", juce::dontSendNotification);
    fxHeaderLabel.setJustificationType (juce::Justification::centredLeft);
    fxHeaderLabel.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    fxHeaderLabel.setColour (juce::Label::textColourId, kTextSecondary);
    fxHeaderLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (fxHeaderLabel);

    modSourceHeader.setText ("SOURCE", juce::dontSendNotification);
    modAmountHeader.setText ("AMOUNT", juce::dontSendNotification);
    modDestHeader.setText ("DEST", juce::dontSendNotification);

    for (auto* header : { &modSourceHeader, &modAmountHeader, &modDestHeader })
    {
        header->setJustificationType (juce::Justification::centred);
        header->setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
        header->setColour (juce::Label::textColourId, kTextMuted);
        header->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*header);
    }

    modRow1Label.setText ("MOD 1", juce::dontSendNotification);
    modRow2Label.setText ("MOD 2", juce::dontSendNotification);
    modRow3Label.setText ("MOD 3", juce::dontSendNotification);

    for (auto* rowLabel : { &modRow1Label, &modRow2Label, &modRow3Label })
    {
        rowLabel->setJustificationType (juce::Justification::centredLeft);
        rowLabel->setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
        rowLabel->setColour (juce::Label::textColourId, kTextSecondary);
        rowLabel->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*rowLabel);
    }

    // --- Knob labels
    setupLabel (waveLabel,   "WAVE");
    setupLabel (cutoffLabel, "CUTOFF");
    setupLabel (resLabel,    "RES");

    setupLabel (envmodLabel, "ENVMOD");
    setupLabel (decayLabel,  "DECAY");
    setupLabel (releaseLabel, "RELEASE");
    setupLabel (accentLabel, "ACCENT");

    setupLabel (glideLabel,  "GLIDE");
    setupLabel (driveLabel,  "DRIVE");
    setupLabel (satLabel,    "SAT");
    setupLabel (subLabel,    "SUB");
    setupLabel (unisonLabel, "UNISON");
    setupLabel (unisonSpreadLabel, "SPREAD");
    setupLabel (gainLabel,   "GAIN");
    setupLabel (volumeLabel, "VOLUME");
    setupLabel (filterCharLabel, "FILTER CHAR");
    setupLabel (lfo1RateLabel, "LFO 1 RATE");
    setupLabel (lfo2RateLabel, "LFO 2 RATE");
    setupLabel (modEnvDecayLabel, "MOD ENV");
    setupLabel (lfo1ModeLabel, "MODE");
    setupLabel (lfo1SyncLabel, "SYNC");
    setupLabel (lfo2ModeLabel, "MODE");
    setupLabel (lfo2SyncLabel, "SYNC");
    setupLabel (mod1AmountLabel, "AMOUNT");
    setupLabel (mod2AmountLabel, "AMOUNT");
    setupLabel (mod3AmountLabel, "AMOUNT");
    setupLabel (fxDriveLabel, "DRIVE");
    setupLabel (fxChorusLabel, "CHORUS");
    setupLabel (fxDelayLabel, "DELAY");
    setupLabel (fxDelayTimeLabel, "TIME");
    setupLabel (fxReverbLabel, "REVERB");

    mod1AmountLabel.setVisible (false);
    mod2AmountLabel.setVisible (false);
    mod3AmountLabel.setVisible (false);

    for (auto* l : { &waveValueLabel, &cutoffValueLabel, &resValueLabel, &envmodValueLabel, &decayValueLabel,
                     &releaseValueLabel,
                     &accentValueLabel, &glideValueLabel, &driveValueLabel, &satValueLabel, &subValueLabel,
                     &unisonValueLabel, &unisonSpreadValueLabel, &gainValueLabel, &volumeValueLabel, &lfo1RateValueLabel,
                     &lfo2RateValueLabel, &modEnvDecayValueLabel, &mod1AmountValueLabel, &mod2AmountValueLabel,
                     &mod3AmountValueLabel, &fxDriveValueLabel, &fxChorusValueLabel, &fxDelayValueLabel,
                     &fxDelayTimeValueLabel, &fxReverbValueLabel })
        setupValueLabel (*l);

    mod1AmountValueLabel.setVisible (false);
    mod2AmountValueLabel.setVisible (false);
    mod3AmountValueLabel.setVisible (false);

    // --- Attachments (unchanged)
    auto& apvts = processor.apvts;
    aWave   = std::make_unique<Attachment> (apvts, "wave",   wave);
    aCutoff = std::make_unique<Attachment> (apvts, "cutoff", cutoff);
    aRes    = std::make_unique<Attachment> (apvts, "res",    res);
    aEnvmod = std::make_unique<Attachment> (apvts, "envmod", envmod);
    aDecay  = std::make_unique<Attachment> (apvts, "decay",  decay);
    aRelease = std::make_unique<Attachment> (apvts, "release", release);
    aAccent = std::make_unique<Attachment> (apvts, "accent", accent);
    aGlide  = std::make_unique<Attachment> (apvts, "glide",  glide);
    aDrive  = std::make_unique<Attachment> (apvts, "drive",  drive);
    aSat    = std::make_unique<Attachment> (apvts, "sat",    sat);
    aSub    = std::make_unique<Attachment> (apvts, "sub",    sub);
    aUnison = std::make_unique<Attachment> (apvts, "unison", unison);
    aUnisonSpread = std::make_unique<Attachment> (apvts, "unisonSpread", unisonSpread);
    aGain   = std::make_unique<Attachment> (apvts, "gain",   gain);
    aVolume = std::make_unique<Attachment> (apvts, "volume", volume);

    aLfo1Rate = std::make_unique<Attachment> (apvts, "lfo1Rate", lfo1Rate);
    aLfo2Rate = std::make_unique<Attachment> (apvts, "lfo2Rate", lfo2Rate);
    aModEnvDecay = std::make_unique<Attachment> (apvts, "modEnvDecay", modEnvDecay);
    aLfo1Mode = std::make_unique<ComboAttachment> (apvts, "lfo1Mode", lfo1Mode);
    aLfo1Sync = std::make_unique<ComboAttachment> (apvts, "lfo1Sync", lfo1Sync);
    aLfo2Mode = std::make_unique<ComboAttachment> (apvts, "lfo2Mode", lfo2Mode);
    aLfo2Sync = std::make_unique<ComboAttachment> (apvts, "lfo2Sync", lfo2Sync);

    aMod1Amount = std::make_unique<Attachment> (apvts, "mod1Amount", mod1Amount);
    aMod2Amount = std::make_unique<Attachment> (apvts, "mod2Amount", mod2Amount);
    aMod3Amount = std::make_unique<Attachment> (apvts, "mod3Amount", mod3Amount);

    aFxDrive = std::make_unique<Attachment> (apvts, "fxDrive", fxDrive);
    aFxChorus = std::make_unique<Attachment> (apvts, "fxChorus", fxChorus);
    aFxDelay = std::make_unique<Attachment> (apvts, "fxDelay", fxDelay);
    aFxDelayTime = std::make_unique<Attachment> (apvts, "fxDelayTime", fxDelayTime);
    aFxReverb = std::make_unique<Attachment> (apvts, "fxReverb", fxReverb);

    aMod1Source = std::make_unique<ComboAttachment> (apvts, "mod1Source", mod1Source);
    aMod1Dest = std::make_unique<ComboAttachment> (apvts, "mod1Dest", mod1Dest);
    aMod2Source = std::make_unique<ComboAttachment> (apvts, "mod2Source", mod2Source);
    aMod2Dest = std::make_unique<ComboAttachment> (apvts, "mod2Dest", mod2Dest);
    aMod3Source = std::make_unique<ComboAttachment> (apvts, "mod3Source", mod3Source);
    aMod3Dest = std::make_unique<ComboAttachment> (apvts, "mod3Dest", mod3Dest);
    aFilterChar = std::make_unique<ComboAttachment> (apvts, "filterChar", filterChar);

    auto updateLfoUi = [this]
    {
        const bool lfo1SyncMode = (lfo1Mode.getSelectedItemIndex() == 1);
        lfo1Rate.setEnabled (! lfo1SyncMode);
        lfo1RateLabel.setEnabled (! lfo1SyncMode);
        lfo1RateValueLabel.setEnabled (! lfo1SyncMode);
        lfo1Rate.setVisible (! lfo1SyncMode);
        lfo1RateLabel.setVisible (! lfo1SyncMode);
        lfo1RateValueLabel.setVisible (! lfo1SyncMode);
        lfo1Sync.setVisible (lfo1SyncMode);
        lfo1SyncLabel.setVisible (lfo1SyncMode);
        lfo1ModeLabel.setVisible (true);

        const bool lfo2SyncMode = (lfo2Mode.getSelectedItemIndex() == 1);
        lfo2Rate.setEnabled (! lfo2SyncMode);
        lfo2RateLabel.setEnabled (! lfo2SyncMode);
        lfo2RateValueLabel.setEnabled (! lfo2SyncMode);
        lfo2Rate.setVisible (! lfo2SyncMode);
        lfo2RateLabel.setVisible (! lfo2SyncMode);
        lfo2RateValueLabel.setVisible (! lfo2SyncMode);
        lfo2Sync.setVisible (lfo2SyncMode);
        lfo2SyncLabel.setVisible (lfo2SyncMode);
        lfo2ModeLabel.setVisible (true);
    };

    lfo1Mode.onChange = updateLfoUi;
    lfo2Mode.onChange = updateLfoUi;
    updateLfoUi();

    // --- Double-click reset
    for (auto* s : { &wave, &cutoff, &res, &envmod, &decay, &release, &accent, &glide, &drive, &sat, &sub, &unison,
                     &unisonSpread, &gain, &volume, &lfo1Rate, &lfo2Rate, &modEnvDecay, &mod1Amount, &mod2Amount, &mod3Amount,
                     &fxDrive, &fxChorus, &fxDelay, &fxDelayTime, &fxReverb })
        s->setDoubleClickReturnValue (true, s->getValue());

    // --- Readout wiring
    waveValueLabel.setText (formatValue (wave), juce::dontSendNotification);
    cutoffValueLabel.setText (formatValue (cutoff), juce::dontSendNotification);
    resValueLabel.setText (formatValue (res), juce::dontSendNotification);
    envmodValueLabel.setText (formatValue (envmod), juce::dontSendNotification);
    decayValueLabel.setText (formatValue (decay), juce::dontSendNotification);
    releaseValueLabel.setText (formatValue (release), juce::dontSendNotification);
    accentValueLabel.setText (formatValue (accent), juce::dontSendNotification);
    glideValueLabel.setText (formatValue (glide), juce::dontSendNotification);
    driveValueLabel.setText (formatValue (drive), juce::dontSendNotification);
    satValueLabel.setText (formatValue (sat), juce::dontSendNotification);
    subValueLabel.setText (formatValue (sub), juce::dontSendNotification);
    unisonValueLabel.setText (formatValue (unison), juce::dontSendNotification);
    unisonSpreadValueLabel.setText (formatValue (unisonSpread), juce::dontSendNotification);
    gainValueLabel.setText (formatValue (gain), juce::dontSendNotification);
    volumeValueLabel.setText (formatValue (volume), juce::dontSendNotification);
    lfo1RateValueLabel.setText (formatValue (lfo1Rate), juce::dontSendNotification);
    lfo2RateValueLabel.setText (formatValue (lfo2Rate), juce::dontSendNotification);
    modEnvDecayValueLabel.setText (formatValue (modEnvDecay), juce::dontSendNotification);
    mod1AmountValueLabel.setText (formatValue (mod1Amount), juce::dontSendNotification);
    mod2AmountValueLabel.setText (formatValue (mod2Amount), juce::dontSendNotification);
    mod3AmountValueLabel.setText (formatValue (mod3Amount), juce::dontSendNotification);
    fxDriveValueLabel.setText (formatValue (fxDrive), juce::dontSendNotification);
    fxChorusValueLabel.setText (formatValue (fxChorus), juce::dontSendNotification);
    fxDelayValueLabel.setText (formatValue (fxDelay), juce::dontSendNotification);
    fxDelayTimeValueLabel.setText (formatValue (fxDelayTime), juce::dontSendNotification);
    fxReverbValueLabel.setText (formatValue (fxReverb), juce::dontSendNotification);

    wireReadout (wave,   "WAVE",   waveValueLabel);
    wireReadout (cutoff, "CUTOFF", cutoffValueLabel);
    wireReadout (res,    "RES",    resValueLabel);
    wireReadout (envmod, "ENVMOD", envmodValueLabel);
    wireReadout (decay,  "DECAY",  decayValueLabel);
    wireReadout (release, "RELEASE", releaseValueLabel);
    wireReadout (accent, "ACCENT", accentValueLabel);
    wireReadout (glide,  "GLIDE",  glideValueLabel);
    wireReadout (drive,  "DRIVE",  driveValueLabel);
    wireReadout (sat,    "SAT",    satValueLabel);
    wireReadout (sub,    "SUB",    subValueLabel);
    wireReadout (unison, "UNISON", unisonValueLabel);
    wireReadout (unisonSpread, "SPREAD", unisonSpreadValueLabel);
    wireReadout (gain,   "GAIN",   gainValueLabel);
    wireReadout (volume, "VOLUME", volumeValueLabel);
    wireReadout (lfo1Rate, "LFO 1 RATE", lfo1RateValueLabel);
    wireReadout (lfo2Rate, "LFO 2 RATE", lfo2RateValueLabel);
    wireReadout (modEnvDecay, "MOD ENV", modEnvDecayValueLabel);
    wireReadout (mod1Amount, "MOD 1 AMT", mod1AmountValueLabel);
    wireReadout (mod2Amount, "MOD 2 AMT", mod2AmountValueLabel);
    wireReadout (mod3Amount, "MOD 3 AMT", mod3AmountValueLabel);
    wireReadout (fxDrive, "FX DRIVE", fxDriveValueLabel);
    wireReadout (fxChorus, "CHORUS", fxChorusValueLabel);
    wireReadout (fxDelay, "DELAY", fxDelayValueLabel);
    wireReadout (fxDelayTime, "DELAY TIME", fxDelayTimeValueLabel);
    wireReadout (fxReverb, "REVERB", fxReverbValueLabel);

    updateReadout ("CUTOFF", cutoff);

    setSize (1020, 900);
}

//==============================================================================
// Paint: structured panels + subtle separators
void AcidSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto bgBounds = bounds.toFloat();

    juce::ColourGradient bgGradient (kBgTop, 0.0f, 0.0f, kBgBottom, 0.0f, (float) getHeight(), false);
    g.setGradientFill (bgGradient);
    g.fillRect (bgBounds);

    // Top bar background
    auto topBar = bounds.removeFromTop (36);
    g.setColour (kPanelBottom);
    g.fillRect (topBar);

    // Divider line
    g.setColour (kPanelEdge);
    g.drawLine (0.0f, (float) topBar.getBottom(), (float) getWidth(), (float) topBar.getBottom(), 1.0f);
    g.setColour (kAccent.withAlpha (0.5f));
    g.drawLine (0.0f, (float) topBar.getBottom() + 1.0f, (float) getWidth(), (float) topBar.getBottom() + 1.0f, 1.5f);

    // Panels
    auto content = bounds.reduced (10);
    const int keyboardHeight = juce::jlimit (70, 100, (int) (content.getHeight() * 0.16f));
    auto keyboardStrip = content.removeFromBottom (keyboardHeight);
    const int modFxHeight = juce::jlimit (260, 380, (int) (content.getHeight() * 0.45f));
    auto modFxPanel = content.removeFromBottom (modFxHeight);
    auto knobPanel = content;

    auto drawPanel = [&](juce::Rectangle<int> panel)
    {
        auto panelF = panel.toFloat();
        juce::DropShadow shadow (juce::Colours::black.withAlpha (0.35f), 12, { 0, 4 });
        shadow.drawForRectangle (g, panel);

        juce::ColourGradient panelGradient (kPanelTop, panelF.getX(), panelF.getY(),
                                            kPanelBottom, panelF.getX(), panelF.getBottom(), false);
        g.setGradientFill (panelGradient);
        g.fillRoundedRectangle (panelF, kPanelCorner);

        g.setColour (kPanelEdge);
        g.drawRoundedRectangle (panelF, kPanelCorner, 1.0f);

        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawRoundedRectangle (panelF.reduced (1.0f), kPanelCorner - 1.0f, 1.0f);
    };

    drawPanel (knobPanel);
    drawPanel (modFxPanel);

    auto keyboardPanel = keyboardStrip.reduced (0, 6);
    if (keyboardPanel.getHeight() > 0)
        drawPanel (keyboardPanel);

    // Keyboard strip divider
    g.setColour (kPanelEdge);
    g.drawLine ((float) keyboardStrip.getX(),
                (float) keyboardStrip.getY(),
                (float) keyboardStrip.getRight(),
                (float) keyboardStrip.getY(),
                1.0f);
}

//==============================================================================
// Layout: consistent spacing, reserved label space, clean grid
void AcidSynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Top bar
    auto topBar = area.removeFromTop (36);
    titleLabel.setBounds (topBar.reduced (10, 0).removeFromLeft (300));
    readoutLabel.setBounds (topBar.reduced (10, 0));

    // Main content
    auto content = area.reduced (10);

    // Keyboard strip
    const int keyboardHeight = juce::jlimit (70, 100, (int) (content.getHeight() * 0.16f));
    auto keyboardArea = content.removeFromBottom (keyboardHeight);
    keyboard.setBounds (keyboardArea.reduced (0, 8));

    // Mod/FX panel area
    const int modFxHeight = juce::jlimit (260, 380, (int) (content.getHeight() * 0.45f));
    auto modFxPanel = content.removeFromBottom (modFxHeight);

    // Knob panel area
    auto gridArea = content.reduced (18, 14);
    auto groupHeader = gridArea.removeFromTop (16);

    const int cols = 4;
    const int rows = 4;

    const int cellW = gridArea.getWidth() / cols;
    const int cellH = gridArea.getHeight() / rows;

    auto cellRect = [&](int col, int row)
    {
        return juce::Rectangle<int> (gridArea.getX() + col * cellW,
                                     gridArea.getY() + row * cellH,
                                     cellW, cellH).reduced (10, 8);
    };

    // Group header labels aligned with columns
    groupLeft .setBounds (juce::Rectangle<int> (groupHeader.getX() + 0 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupMid  .setBounds (juce::Rectangle<int> (groupHeader.getX() + 1 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupRight.setBounds (juce::Rectangle<int> (groupHeader.getX() + 2 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupExtra.setBounds (juce::Rectangle<int> (groupHeader.getX() + 3 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));

    // Helper: place a label + knob inside a cell
    auto place = [&](juce::Label& lbl, juce::Slider& s, juce::Label& valueLabel, int col, int row)
    {
        auto cell = cellRect (col, row);

        auto labelArea = cell.removeFromTop (14);
        lbl.setBounds (labelArea);

        auto valueArea = cell.removeFromBottom (14);
        valueLabel.setBounds (valueArea);

        s.setBounds (cell);
    };

    auto placeCombo = [&](juce::Label& lbl, juce::ComboBox& combo, int col, int row)
    {
        auto cell = cellRect (col, row);
        auto labelArea = cell.removeFromTop (14);
        lbl.setBounds (labelArea);
        combo.setBounds (cell.withHeight (22).withCentre (cell.getCentre()));
    };

    // 4x4 placements
    place (waveLabel,   wave,   waveValueLabel,   0, 0);
    place (cutoffLabel, cutoff, cutoffValueLabel, 1, 0);
    place (resLabel,    res,    resValueLabel,    2, 0);
    place (envmodLabel, envmod, envmodValueLabel, 3, 0);

    place (decayLabel,  decay,  decayValueLabel,  0, 1);
    place (accentLabel, accent, accentValueLabel, 1, 1);
    place (glideLabel,  glide,  glideValueLabel,  2, 1);
    place (driveLabel,  drive,  driveValueLabel,  3, 1);

    place (satLabel,    sat,    satValueLabel,    0, 2);
    place (subLabel,    sub,    subValueLabel,    1, 2);
    place (unisonLabel, unison, unisonValueLabel, 2, 2);
    place (unisonSpreadLabel, unisonSpread, unisonSpreadValueLabel, 3, 2);

    place (gainLabel,   gain,   gainValueLabel,   0, 3);
    placeCombo (filterCharLabel, filterChar, 1, 3);
    place (releaseLabel, release, releaseValueLabel, 2, 3);
    place (volumeLabel, volume, volumeValueLabel, 3, 3);

    // Mod/FX panel layout
    auto modFxArea = modFxPanel.reduced (18, 12);
    auto modArea = modFxArea.removeFromLeft (int (modFxArea.getWidth() * 0.62f));
    auto fxArea = modFxArea;

    auto modHeader = modArea.removeFromTop (18);
    modHeaderLabel.setBounds (modHeader);

    auto modColumnHeader = modArea.removeFromTop (14);

    const int rowLabelWidth = 52;
    const int gap = 8;
    const int available = modArea.getWidth() - rowLabelWidth - gap * 3;
    const int sourceWidth = (int) (available * 0.38f);
    const int amountWidth = (int) (available * 0.24f);
    const int destWidth = available - sourceWidth - amountWidth;

    modSourceHeader.setBounds (modColumnHeader.getX() + rowLabelWidth + gap,
                               modColumnHeader.getY(),
                               sourceWidth,
                               modColumnHeader.getHeight());
    modAmountHeader.setBounds (modSourceHeader.getRight() + gap,
                               modColumnHeader.getY(),
                               amountWidth,
                               modColumnHeader.getHeight());
    modDestHeader.setBounds (modAmountHeader.getRight() + gap,
                             modColumnHeader.getY(),
                             destWidth,
                             modColumnHeader.getHeight());

    const int modRowHeight = modArea.getHeight() / 3;
    auto modRow = [&] (juce::Label& rowLabel, juce::ComboBox& source, juce::Slider& amount, juce::ComboBox& dest)
    {
        auto rowArea = modArea.removeFromTop (modRowHeight).reduced (0, 6);
        rowLabel.setBounds (rowArea.removeFromLeft (rowLabelWidth));
        rowArea.removeFromLeft (gap);

        source.setBounds (rowArea.removeFromLeft (sourceWidth));
        rowArea.removeFromLeft (gap);

        amount.setBounds (rowArea.removeFromLeft (amountWidth));
        rowArea.removeFromLeft (gap);

        dest.setBounds (rowArea.removeFromLeft (destWidth));
    };

    modRow (modRow1Label, mod1Source, mod1Amount, mod1Dest);
    modRow (modRow2Label, mod2Source, mod2Amount, mod2Dest);
    modRow (modRow3Label, mod3Source, mod3Amount, mod3Dest);

    auto fxHeader = fxArea.removeFromTop (18);
    fxHeaderLabel.setBounds (fxHeader);

    int fxSourcesHeight = juce::jlimit (90, 160, (int) (fxArea.getHeight() * 0.5f));
    const int minFxControlsHeight = 130;
    if (fxArea.getHeight() - fxSourcesHeight < minFxControlsHeight)
        fxSourcesHeight = juce::jmax (60, fxArea.getHeight() - minFxControlsHeight);
    fxSourcesHeight = juce::jmax (60, fxSourcesHeight);

    auto fxSourcesArea = fxArea.removeFromTop (fxSourcesHeight);
    fxSourcesArea = fxSourcesArea.reduced (6, 4);
    const int fxSourceCellW = fxSourcesArea.getWidth() / 3;
    const int fxLabelHeight = 12;
    const int fxValueHeight = 12;
    auto fxSourceCell = [&](juce::Label& lbl, juce::Slider& s, juce::Label& valueLabel, int col)
    {
        auto cell = juce::Rectangle<int> (fxSourcesArea.getX() + col * fxSourceCellW,
                                          fxSourcesArea.getY(),
                                          fxSourceCellW,
                                          fxSourcesArea.getHeight()).reduced (6, 0);
        auto labelArea = cell.removeFromTop (fxLabelHeight);
        lbl.setBounds (labelArea);
        auto valueArea = cell.removeFromBottom (fxValueHeight);
        valueLabel.setBounds (valueArea);
        s.setBounds (cell);
    };

    auto fxLfoCell = [&](juce::Label& rateLabel, juce::Slider& rateSlider, juce::Label& valueLabel,
                         juce::Label& modeLabel, juce::ComboBox& mode,
                         juce::Label& syncLabel, juce::ComboBox& sync,
                         int col)
    {
        auto cell = juce::Rectangle<int> (fxSourcesArea.getX() + col * fxSourceCellW,
                                          fxSourcesArea.getY(),
                                          fxSourceCellW,
                                          fxSourcesArea.getHeight()).reduced (6, 0);
        auto labelArea = cell.removeFromTop (fxLabelHeight);
        rateLabel.setBounds (labelArea);
        auto valueArea = cell.removeFromBottom (fxValueHeight);
        valueLabel.setBounds (valueArea);

        auto sliderArea = cell.removeFromTop (26);
        rateSlider.setBounds (sliderArea);

        auto modeLabelArea = cell.removeFromTop (12);
        modeLabel.setBounds (modeLabelArea);
        auto modeArea = cell.removeFromTop (18);
        mode.setBounds (modeArea.withHeight (18).withCentre (modeArea.getCentre()));

        auto syncLabelArea = cell.removeFromTop (12);
        syncLabel.setBounds (syncLabelArea);
        auto syncArea = cell.removeFromTop (18);
        sync.setBounds (syncArea.withHeight (18).withCentre (syncArea.getCentre()));
    };

    fxLfoCell (lfo1RateLabel, lfo1Rate, lfo1RateValueLabel,
               lfo1ModeLabel, lfo1Mode, lfo1SyncLabel, lfo1Sync, 0);
    fxLfoCell (lfo2RateLabel, lfo2Rate, lfo2RateValueLabel,
               lfo2ModeLabel, lfo2Mode, lfo2SyncLabel, lfo2Sync, 1);
    fxSourceCell (modEnvDecayLabel, modEnvDecay, modEnvDecayValueLabel, 2);

    auto fxControlsArea = fxArea.reduced (6, 8);
    const int fxCols = 3;
    const int fxRows = 2;
    const int fxCellW = fxControlsArea.getWidth() / fxCols;
    const int fxCellH = fxControlsArea.getHeight() / fxRows;

    auto fxCell = [&](juce::Label& lbl, juce::Slider& s, juce::Label& valueLabel, int col, int row)
    {
        auto cell = juce::Rectangle<int> (fxControlsArea.getX() + col * fxCellW,
                                          fxControlsArea.getY() + row * fxCellH,
                                          fxCellW, fxCellH).reduced (8, 6);
        auto labelArea = cell.removeFromTop (fxLabelHeight);
        lbl.setBounds (labelArea);
        auto valueArea = cell.removeFromBottom (fxValueHeight);
        valueLabel.setBounds (valueArea);
        s.setBounds (cell);
    };

    fxCell (fxDriveLabel, fxDrive, fxDriveValueLabel, 0, 0);
    fxCell (fxChorusLabel, fxChorus, fxChorusValueLabel, 1, 0);
    fxCell (fxDelayLabel, fxDelay, fxDelayValueLabel, 2, 0);
    fxCell (fxDelayTimeLabel, fxDelayTime, fxDelayTimeValueLabel, 0, 1);
    fxCell (fxReverbLabel, fxReverb, fxReverbValueLabel, 1, 1);
}
