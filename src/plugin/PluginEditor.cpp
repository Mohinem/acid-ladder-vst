// src/plugin/PluginEditor.cpp
#include "PluginEditor.h"

//==============================================================================
// Knob styling (minimal but more "pro" feeling)
void AcidSynthAudioProcessorEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalDrag);

    // Remove the clunky numeric boxes (prototype look)
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    // Better feel: velocity mode makes rotary control smoother/more "plugin-like"
    s.setVelocityBasedMode (true);
    s.setVelocityModeParameters (0.7, 1, 0.05, true);

    // Longer drag distance for finer control on mouse/trackpad
    s.setMouseDragSensitivity (200);

    // Mouse wheel tweaks
    s.setScrollWheelEnabled (true);
}

//==============================================================================
// Label styling
void AcidSynthAudioProcessorEditor::setupLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (12.0f).withStyle (juce::Font::bold));
    l.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.90f));
    l.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (l);
}

//==============================================================================
// Readout formatting
void AcidSynthAudioProcessorEditor::updateReadout (const juce::String& name, const juce::Slider& s)
{
    juce::String value;
    const auto v = s.getValue();

    if (std::abs (v) >= 1000.0)
        value = juce::String (v, 1);
    else if (std::abs (v) >= 100.0)
        value = juce::String (v, 1);
    else
        value = juce::String (v, 3);

    readoutLabel.setText (name + ": " + value, juce::dontSendNotification);
}

//==============================================================================
// Connect slider interactions to top readout
void AcidSynthAudioProcessorEditor::wireReadout (juce::Slider& s, const juce::String& name)
{
    s.setTooltip (name);

    s.onDragStart = [this, &s, name]
    {
        updateReadout (name, s);
    };

    s.onValueChange = [this, &s, name]
    {
        updateReadout (name, s);
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
    setupKnob (accent);
    setupKnob (glide);
    setupKnob (drive);
    setupKnob (gain);

    addAndMakeVisible (wave);
    addAndMakeVisible (cutoff);
    addAndMakeVisible (res);
    addAndMakeVisible (envmod);
    addAndMakeVisible (decay);
    addAndMakeVisible (accent);
    addAndMakeVisible (glide);
    addAndMakeVisible (drive);
    addAndMakeVisible (gain);

    // --- Keyboard
    addAndMakeVisible (keyboard);

    // --- Top bar labels
    titleLabel.setText ("Acid Ladder VST", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (15.0f).withStyle (juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.95f));
    titleLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (titleLabel);

    readoutLabel.setText ("", juce::dontSendNotification);
    readoutLabel.setJustificationType (juce::Justification::centredRight);
    readoutLabel.setFont (juce::Font (13.0f).withStyle (juce::Font::plain));
    readoutLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.85f));
    readoutLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (readoutLabel);

    // --- Group headers
    groupLeft.setText ("OSC", juce::dontSendNotification);
    groupMid.setText  ("FILTER", juce::dontSendNotification);
    groupRight.setText("AMP", juce::dontSendNotification);

    for (auto* gl : { &groupLeft, &groupMid, &groupRight })
    {
        gl->setJustificationType (juce::Justification::centred);
        gl->setFont (juce::Font (11.0f).withStyle (juce::Font::bold));
        gl->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.55f));
        gl->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*gl);
    }

    // --- Knob labels
    setupLabel (waveLabel,   "WAVE");
    setupLabel (cutoffLabel, "CUTOFF");
    setupLabel (resLabel,    "RES");

    setupLabel (envmodLabel, "ENVMOD");
    setupLabel (decayLabel,  "DECAY");
    setupLabel (accentLabel, "ACCENT");

    setupLabel (glideLabel,  "GLIDE");
    setupLabel (driveLabel,  "DRIVE");
    setupLabel (gainLabel,   "GAIN");

    // --- Attachments (unchanged)
    auto& apvts = processor.apvts;
    aWave   = std::make_unique<Attachment> (apvts, "wave",   wave);
    aCutoff = std::make_unique<Attachment> (apvts, "cutoff", cutoff);
    aRes    = std::make_unique<Attachment> (apvts, "res",    res);
    aEnvmod = std::make_unique<Attachment> (apvts, "envmod", envmod);
    aDecay  = std::make_unique<Attachment> (apvts, "decay",  decay);
    aAccent = std::make_unique<Attachment> (apvts, "accent", accent);
    aGlide  = std::make_unique<Attachment> (apvts, "glide",  glide);
    aDrive  = std::make_unique<Attachment> (apvts, "drive",  drive);
    aGain   = std::make_unique<Attachment> (apvts, "gain",   gain);

    // --- Double-click reset
    for (auto* s : { &wave, &cutoff, &res, &envmod, &decay, &accent, &glide, &drive, &gain })
        s->setDoubleClickReturnValue (true, s->getValue());

    // --- Readout wiring
    wireReadout (wave,   "WAVE");
    wireReadout (cutoff, "CUTOFF");
    wireReadout (res,    "RES");
    wireReadout (envmod, "ENVMOD");
    wireReadout (decay,  "DECAY");
    wireReadout (accent, "ACCENT");
    wireReadout (glide,  "GLIDE");
    wireReadout (drive,  "DRIVE");
    wireReadout (gain,   "GAIN");

    updateReadout ("CUTOFF", cutoff);

    setSize (760, 420);
}

//==============================================================================
// Paint: structured panels + subtle separators
void AcidSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto bounds = getLocalBounds();

    // Top bar background
    auto topBar = bounds.removeFromTop (36);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.fillRect (topBar);

    // Divider line
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawLine (0.0f, (float) topBar.getBottom(), (float) getWidth(), (float) topBar.getBottom(), 1.0f);

    // Knob panel background
    auto content = bounds.reduced (10);
    auto keyboardStrip = content.removeFromBottom (100);

    auto knobPanel = content;
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.fillRoundedRectangle (knobPanel.toFloat(), 10.0f);

    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawRoundedRectangle (knobPanel.toFloat(), 10.0f, 1.0f);

    // Keyboard strip divider
    g.setColour (juce::Colours::white.withAlpha (0.12f));
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
    auto keyboardArea = content.removeFromBottom (100);
    keyboard.setBounds (keyboardArea.reduced (0, 8));

    // Knob panel area
    auto gridArea = content;

    // Inner padding for the panel
    gridArea = gridArea.reduced (18, 14);

    // Reserve header strip above the 3 columns for group labels
    auto groupHeader = gridArea.removeFromTop (16);

    // 3 columns
    const int cols = 3;
    const int rows = 3;

    const int cellW = gridArea.getWidth() / cols;
    const int cellH = gridArea.getHeight() / rows;

    auto cellRect = [&](int col, int row)
    {
        return juce::Rectangle<int> (gridArea.getX() + col * cellW,
                                     gridArea.getY() + row * cellH,
                                     cellW, cellH).reduced (10, 8);
    };

    // Group header labels aligned with columns
    groupLeft .setBounds (juce::Rectangle<int> (groupHeader.getX() + 0 * (gridArea.getWidth() / 3), groupHeader.getY(),
                                               gridArea.getWidth() / 3, groupHeader.getHeight()));
    groupMid  .setBounds (juce::Rectangle<int> (groupHeader.getX() + 1 * (gridArea.getWidth() / 3), groupHeader.getY(),
                                               gridArea.getWidth() / 3, groupHeader.getHeight()));
    groupRight.setBounds (juce::Rectangle<int> (groupHeader.getX() + 2 * (gridArea.getWidth() / 3), groupHeader.getY(),
                                               gridArea.getWidth() / 3, groupHeader.getHeight()));

    // Helper: place a label + knob inside a cell
    auto place = [&](juce::Label& lbl, juce::Slider& s, int col, int row)
    {
        auto cell = cellRect (col, row);

        // label strip
        auto labelArea = cell.removeFromTop (16);
        lbl.setBounds (labelArea);

        // knob gets the rest
        s.setBounds (cell);
    };

    // 3x3 placements
    place (waveLabel,   wave,   0, 0);
    place (cutoffLabel, cutoff, 1, 0);
    place (resLabel,    res,    2, 0);

    place (envmodLabel, envmod, 0, 1);
    place (decayLabel,  decay,  1, 1);
    place (accentLabel, accent, 2, 1);

    place (glideLabel,  glide,  0, 2);
    place (driveLabel,  drive,  1, 2);
    place (gainLabel,   gain,   2, 2);
}
