// src/plugin/PluginEditor.cpp
#include "PluginEditor.h"

void AcidSynthAudioProcessorEditor::Section::registerControl (juce::Component& c, const juce::String& debugName)
{
    editor.registerControl (c, debugName, this);
}

AcidSynthAudioProcessorEditor::TopBarSection::TopBarSection (AcidSynthAudioProcessorEditor& editorIn)
    : Section (editorIn)
{
    titleLabel.setText ("Acid Ladder VST", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::Font (15.0f).withStyle (juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.95f));
    titleLabel.setInterceptsMouseClicks (false, false);
    registerControl (titleLabel, "titleLabel");

    readoutLabel.setText ("", juce::dontSendNotification);
    readoutLabel.setJustificationType (juce::Justification::centredRight);
    readoutLabel.setFont (juce::Font (13.0f).withStyle (juce::Font::plain));
    readoutLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.85f));
    readoutLabel.setInterceptsMouseClicks (false, false);
    registerControl (readoutLabel, "readoutLabel");
}

void AcidSynthAudioProcessorEditor::TopBarSection::resized()
{
    auto area = getLocalBounds();
    titleLabel.setBounds (area.reduced (10, 0).removeFromLeft (300));
    readoutLabel.setBounds (area.reduced (10, 0));
}

AcidSynthAudioProcessorEditor::KeyboardSection::KeyboardSection (AcidSynthAudioProcessorEditor& editorIn,
                                                                juce::MidiKeyboardState& state)
    : Section (editorIn)
    , keyboard (state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    registerControl (keyboard, "keyboard");
}

void AcidSynthAudioProcessorEditor::KeyboardSection::resized()
{
    keyboard.setBounds (getLocalBounds().reduced (0, 8));
}

AcidSynthAudioProcessorEditor::KnobPanelSection::KnobPanelSection (AcidSynthAudioProcessorEditor& editorIn)
    : Section (editorIn)
{
    for (auto* slider : { &wave, &cutoff, &res, &envmod, &decay, &release, &accent, &glide, &drive, &sat, &sub, &unison,
                          &unisonSpread, &gain, &volume })
    {
        editor.setupKnob (*slider);
    }

    registerControl (wave, "wave");
    registerControl (cutoff, "cutoff");
    registerControl (res, "res");
    registerControl (envmod, "envmod");
    registerControl (decay, "decay");
    registerControl (release, "release");
    registerControl (accent, "accent");
    registerControl (glide, "glide");
    registerControl (drive, "drive");
    registerControl (sat, "sat");
    registerControl (sub, "sub");
    registerControl (unison, "unison");
    registerControl (unisonSpread, "unisonSpread");
    registerControl (gain, "gain");
    registerControl (volume, "volume");

    editor.setupCombo (filterChar);
    registerControl (filterChar, "filterChar");

    groupLeft.setText ("OSC", juce::dontSendNotification);
    groupMid.setText  ("FILTER", juce::dontSendNotification);
    groupRight.setText("AMP", juce::dontSendNotification);
    groupExtra.setText("MIX", juce::dontSendNotification);

    for (auto* gl : { &groupLeft, &groupMid, &groupRight, &groupExtra })
    {
        gl->setJustificationType (juce::Justification::centred);
        gl->setFont (juce::Font (11.0f).withStyle (juce::Font::bold));
        gl->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.55f));
        gl->setInterceptsMouseClicks (false, false);
        registerControl (*gl, gl->getText());
    }

    editor.setupLabel (waveLabel,   "WAVE");
    editor.setupLabel (cutoffLabel, "CUTOFF");
    editor.setupLabel (resLabel,    "RES");

    editor.setupLabel (envmodLabel, "ENVMOD");
    editor.setupLabel (decayLabel,  "DECAY");
    editor.setupLabel (releaseLabel, "RELEASE");
    editor.setupLabel (accentLabel, "ACCENT");

    editor.setupLabel (glideLabel,  "GLIDE");
    editor.setupLabel (driveLabel,  "DRIVE");
    editor.setupLabel (satLabel,    "SAT");
    editor.setupLabel (subLabel,    "SUB");
    editor.setupLabel (unisonLabel, "UNISON");
    editor.setupLabel (unisonSpreadLabel, "SPREAD");
    editor.setupLabel (gainLabel,   "GAIN");
    editor.setupLabel (volumeLabel, "VOLUME");
    editor.setupLabel (filterCharLabel, "FILTER CHAR");

    for (auto* label : { &waveLabel, &cutoffLabel, &resLabel, &envmodLabel, &decayLabel, &releaseLabel,
                         &accentLabel, &glideLabel, &driveLabel, &satLabel, &subLabel, &unisonLabel,
                         &unisonSpreadLabel, &gainLabel, &volumeLabel, &filterCharLabel })
        registerControl (*label, label->getText());

    editor.setupValueLabel (waveValueLabel);
    editor.setupValueLabel (cutoffValueLabel);
    editor.setupValueLabel (resValueLabel);
    editor.setupValueLabel (envmodValueLabel);
    editor.setupValueLabel (decayValueLabel);
    editor.setupValueLabel (releaseValueLabel);
    editor.setupValueLabel (accentValueLabel);
    editor.setupValueLabel (glideValueLabel);
    editor.setupValueLabel (driveValueLabel);
    editor.setupValueLabel (satValueLabel);
    editor.setupValueLabel (subValueLabel);
    editor.setupValueLabel (unisonValueLabel);
    editor.setupValueLabel (unisonSpreadValueLabel);
    editor.setupValueLabel (gainValueLabel);
    editor.setupValueLabel (volumeValueLabel);

    registerControl (waveValueLabel, "waveValueLabel");
    registerControl (cutoffValueLabel, "cutoffValueLabel");
    registerControl (resValueLabel, "resValueLabel");
    registerControl (envmodValueLabel, "envmodValueLabel");
    registerControl (decayValueLabel, "decayValueLabel");
    registerControl (releaseValueLabel, "releaseValueLabel");
    registerControl (accentValueLabel, "accentValueLabel");
    registerControl (glideValueLabel, "glideValueLabel");
    registerControl (driveValueLabel, "driveValueLabel");
    registerControl (satValueLabel, "satValueLabel");
    registerControl (subValueLabel, "subValueLabel");
    registerControl (unisonValueLabel, "unisonValueLabel");
    registerControl (unisonSpreadValueLabel, "unisonSpreadValueLabel");
    registerControl (gainValueLabel, "gainValueLabel");
    registerControl (volumeValueLabel, "volumeValueLabel");
}

void AcidSynthAudioProcessorEditor::KnobPanelSection::resized()
{
    auto gridArea = getLocalBounds().reduced (18, 14);
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

    groupLeft .setBounds (juce::Rectangle<int> (groupHeader.getX() + 0 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupMid  .setBounds (juce::Rectangle<int> (groupHeader.getX() + 1 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupRight.setBounds (juce::Rectangle<int> (groupHeader.getX() + 2 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));
    groupExtra.setBounds (juce::Rectangle<int> (groupHeader.getX() + 3 * (gridArea.getWidth() / cols), groupHeader.getY(),
                                               gridArea.getWidth() / cols, groupHeader.getHeight()));

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
}

AcidSynthAudioProcessorEditor::ModMatrixSection::ModMatrixSection (AcidSynthAudioProcessorEditor& editorIn)
    : Section (editorIn)
{
    modHeaderLabel.setText ("MOD MATRIX", juce::dontSendNotification);
    modHeaderLabel.setJustificationType (juce::Justification::centredLeft);
    modHeaderLabel.setFont (juce::Font (11.0f).withStyle (juce::Font::bold));
    modHeaderLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.6f));
    modHeaderLabel.setInterceptsMouseClicks (false, false);
    registerControl (modHeaderLabel, "modHeaderLabel");

    modSourceHeader.setText ("SOURCE", juce::dontSendNotification);
    modAmountHeader.setText ("AMOUNT", juce::dontSendNotification);
    modDestHeader.setText ("DEST", juce::dontSendNotification);

    for (auto* header : { &modSourceHeader, &modAmountHeader, &modDestHeader })
    {
        header->setJustificationType (juce::Justification::centred);
        header->setFont (juce::Font (10.0f).withStyle (juce::Font::bold));
        header->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.5f));
        header->setInterceptsMouseClicks (false, false);
        registerControl (*header, header->getText());
    }

    modRow1Label.setText ("MOD 1", juce::dontSendNotification);
    modRow2Label.setText ("MOD 2", juce::dontSendNotification);
    modRow3Label.setText ("MOD 3", juce::dontSendNotification);

    for (auto* rowLabel : { &modRow1Label, &modRow2Label, &modRow3Label })
    {
        rowLabel->setJustificationType (juce::Justification::centredLeft);
        rowLabel->setFont (juce::Font (11.0f).withStyle (juce::Font::bold));
        rowLabel->setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));
        rowLabel->setInterceptsMouseClicks (false, false);
        registerControl (*rowLabel, rowLabel->getText());
    }

    for (auto* combo : { &mod1Source, &mod1Dest, &mod2Source, &mod2Dest, &mod3Source, &mod3Dest })
    {
        editor.setupCombo (*combo);
        registerControl (*combo, "modCombo");
    }

    for (auto* slider : { &mod1Amount, &mod2Amount, &mod3Amount })
    {
        editor.setupKnob (*slider);
        registerControl (*slider, "modAmount");
    }

    editor.setupLabel (mod1AmountLabel, "AMOUNT");
    editor.setupLabel (mod2AmountLabel, "AMOUNT");
    editor.setupLabel (mod3AmountLabel, "AMOUNT");
    registerControl (mod1AmountLabel, "mod1AmountLabel");
    registerControl (mod2AmountLabel, "mod2AmountLabel");
    registerControl (mod3AmountLabel, "mod3AmountLabel");

    mod1AmountLabel.setVisible (false);
    mod2AmountLabel.setVisible (false);
    mod3AmountLabel.setVisible (false);

    editor.setupValueLabel (mod1AmountValueLabel);
    editor.setupValueLabel (mod2AmountValueLabel);
    editor.setupValueLabel (mod3AmountValueLabel);

    registerControl (mod1AmountValueLabel, "mod1AmountValueLabel");
    registerControl (mod2AmountValueLabel, "mod2AmountValueLabel");
    registerControl (mod3AmountValueLabel, "mod3AmountValueLabel");

    mod1AmountValueLabel.setVisible (false);
    mod2AmountValueLabel.setVisible (false);
    mod3AmountValueLabel.setVisible (false);
}

void AcidSynthAudioProcessorEditor::ModMatrixSection::resized()
{
    auto modArea = getLocalBounds();

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
}

AcidSynthAudioProcessorEditor::FxSection::FxSection (AcidSynthAudioProcessorEditor& editorIn)
    : Section (editorIn)
{
    fxHeaderLabel.setText ("FX + MOD SOURCES", juce::dontSendNotification);
    fxHeaderLabel.setJustificationType (juce::Justification::centredLeft);
    fxHeaderLabel.setFont (juce::Font (11.0f).withStyle (juce::Font::bold));
    fxHeaderLabel.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.6f));
    fxHeaderLabel.setInterceptsMouseClicks (false, false);
    registerControl (fxHeaderLabel, "fxHeaderLabel");

    for (auto* slider : { &lfo1Rate, &lfo2Rate, &modEnvDecay, &fxDrive, &fxChorus, &fxDelay, &fxDelayTime, &fxReverb })
    {
        editor.setupKnob (*slider);
        registerControl (*slider, "fxSlider");
    }

    for (auto* combo : { &lfo1Mode, &lfo1Sync, &lfo2Mode, &lfo2Sync })
    {
        editor.setupCombo (*combo);
        registerControl (*combo, "lfoCombo");
    }

    editor.setupLabel (lfo1RateLabel, "LFO 1 RATE");
    editor.setupLabel (lfo2RateLabel, "LFO 2 RATE");
    editor.setupLabel (modEnvDecayLabel, "MOD ENV");
    editor.setupLabel (lfo1ModeLabel, "MODE");
    editor.setupLabel (lfo1SyncLabel, "SYNC");
    editor.setupLabel (lfo2ModeLabel, "MODE");
    editor.setupLabel (lfo2SyncLabel, "SYNC");
    editor.setupLabel (fxDriveLabel, "DRIVE");
    editor.setupLabel (fxChorusLabel, "CHORUS");
    editor.setupLabel (fxDelayLabel, "DELAY");
    editor.setupLabel (fxDelayTimeLabel, "TIME");
    editor.setupLabel (fxReverbLabel, "REVERB");

    for (auto* label : { &lfo1RateLabel, &lfo2RateLabel, &modEnvDecayLabel, &lfo1ModeLabel, &lfo1SyncLabel,
                         &lfo2ModeLabel, &lfo2SyncLabel, &fxDriveLabel, &fxChorusLabel, &fxDelayLabel,
                         &fxDelayTimeLabel, &fxReverbLabel })
        registerControl (*label, label->getText());

    editor.setupValueLabel (lfo1RateValueLabel);
    editor.setupValueLabel (lfo2RateValueLabel);
    editor.setupValueLabel (modEnvDecayValueLabel);
    editor.setupValueLabel (fxDriveValueLabel);
    editor.setupValueLabel (fxChorusValueLabel);
    editor.setupValueLabel (fxDelayValueLabel);
    editor.setupValueLabel (fxDelayTimeValueLabel);
    editor.setupValueLabel (fxReverbValueLabel);

    registerControl (lfo1RateValueLabel, "lfo1RateValueLabel");
    registerControl (lfo2RateValueLabel, "lfo2RateValueLabel");
    registerControl (modEnvDecayValueLabel, "modEnvDecayValueLabel");
    registerControl (fxDriveValueLabel, "fxDriveValueLabel");
    registerControl (fxChorusValueLabel, "fxChorusValueLabel");
    registerControl (fxDelayValueLabel, "fxDelayValueLabel");
    registerControl (fxDelayTimeValueLabel, "fxDelayTimeValueLabel");
    registerControl (fxReverbValueLabel, "fxReverbValueLabel");
}

void AcidSynthAudioProcessorEditor::FxSection::resized()
{
    auto fxArea = getLocalBounds();
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

AcidSynthAudioProcessorEditor::~AcidSynthAudioProcessorEditor()
{
    for (auto* s : { &knobPanel.wave, &knobPanel.cutoff, &knobPanel.res, &knobPanel.envmod, &knobPanel.decay,
                     &knobPanel.release, &knobPanel.accent, &knobPanel.glide, &knobPanel.drive, &knobPanel.sat,
                     &knobPanel.sub, &knobPanel.unison, &knobPanel.unisonSpread, &knobPanel.gain, &knobPanel.volume,
                     &fxPanel.lfo1Rate, &fxPanel.lfo2Rate, &fxPanel.modEnvDecay, &modMatrixPanel.mod1Amount,
                     &modMatrixPanel.mod2Amount, &modMatrixPanel.mod3Amount, &fxPanel.fxDrive, &fxPanel.fxChorus,
                     &fxPanel.fxDelay, &fxPanel.fxDelayTime, &fxPanel.fxReverb })
        s->setLookAndFeel (nullptr);
}

//==============================================================================
// Knob styling (minimal but more "pro" feeling)
void AcidSynthAudioProcessorEditor::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::LinearHorizontal);

    // Remove the clunky numeric boxes (prototype look)
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

    s.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff1b1b1b));
    s.setColour (juce::Slider::trackColourId, juce::Colour (0xff48c6ff));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff3f3f3));

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
    juce::ColourGradient faceGradient (juce::Colour (0xff3a3a3a), faceBounds.getCentreX(), faceBounds.getY(),
                                       juce::Colour (0xff0c0c0c), faceBounds.getCentreX(), faceBounds.getBottom(), false);
    g.setGradientFill (faceGradient);
    g.fillEllipse (faceBounds);

    g.setColour (juce::Colour (0xff5f5f5f));
    g.drawEllipse (rimBounds, 1.2f);

    auto arcRadius = radius - 5.0f;
    juce::Path backgroundArc;
    backgroundArc.addArc (centre.x - arcRadius, centre.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff2f2f2f));
    g.strokePath (backgroundArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addArc (centre.x - arcRadius, centre.y - arcRadius, arcRadius * 2.0f, arcRadius * 2.0f,
                     rotaryStartAngle, angle, true);
    auto valueColour = slider.isEnabled() ? juce::Colour (0xff48c6ff) : juce::Colours::grey;
    g.setColour (valueColour.withAlpha (0.35f));
    g.strokePath (valueArc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (valueColour);
    g.strokePath (valueArc, juce::PathStrokeType (3.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    auto pointerLength = arcRadius - 6.0f;
    auto pointerThickness = 2.4f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength, 1.0f);

    g.setColour (juce::Colour (0xfff3f3f3));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

    auto capBounds = juce::Rectangle<float> (centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f);
    g.setColour (juce::Colour (0xffd9d9d9));
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

    auto filled = track.withWidth (juce::jlimit (0.0f, track.getWidth(), sliderPos - track.getX()));
    g.setColour (trackColour);
    g.fillRoundedRectangle (filled, trackHeight * 0.5f);

    auto thumbRadius = trackHeight * 0.9f;
    auto thumbCentre = juce::Point<float> (sliderPos, track.getCentreY());
    g.setColour (thumbColour);
    g.fillEllipse (juce::Rectangle<float> (thumbRadius * 2.0f, thumbRadius * 2.0f).withCentre (thumbCentre));
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
}

void AcidSynthAudioProcessorEditor::setupValueLabel (juce::Label& l)
{
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (11.0f).withStyle (juce::Font::plain));
    l.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));
    l.setInterceptsMouseClicks (false, false);
}

void AcidSynthAudioProcessorEditor::setupCombo (juce::ComboBox& combo)
{
    combo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1b1b1b));
    combo.setColour (juce::ComboBox::textColourId, juce::Colours::white.withAlpha (0.9f));
    combo.setColour (juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha (0.2f));
}

void AcidSynthAudioProcessorEditor::registerControl (juce::Component& c, const juce::String& debugName,
                                                     juce::Component* parentOverride)
{
    c.setName (debugName);
    if (parentOverride != nullptr)
        parentOverride->addAndMakeVisible (c);
    else
        addAndMakeVisible (c);
    registeredControls.push_back (&c);
}

void AcidSynthAudioProcessorEditor::layoutGuard()
{
#if JUCE_DEBUG
    for (auto* control : registeredControls)
    {
        if (control == nullptr)
            continue;

        if (control->isVisible() && control->getParentComponent() == nullptr)
        {
            DBG ("LayoutGuard: " + control->getName() + " has no parent component");
            jassertfalse;
        }

        if (control->isVisible() && (control->getWidth() < 4 || control->getHeight() < 4))
        {
            DBG ("LayoutGuard: " + control->getName() + " has tiny bounds " + control->getBounds().toString());
            jassertfalse;
        }
    }
#endif
}

void AcidSynthAudioProcessorEditor::addSliderAttachment (const juce::String& paramId, juce::Slider& slider)
{
    attachments.sliders.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, paramId, slider));
}

void AcidSynthAudioProcessorEditor::addComboAttachment (const juce::String& paramId, juce::ComboBox& combo)
{
    attachments.combos.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, paramId, combo));
}

//==============================================================================
// Readout formatting
void AcidSynthAudioProcessorEditor::updateReadout (const juce::String& name, const juce::Slider& s)
{
    topBar.readoutLabel.setText (name + ": " + formatValue (s), juce::dontSendNotification);
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
    , topBar (*this)
    , keyboardSection (*this, processor.keyboardState)
    , knobPanel (*this)
    , modMatrixPanel (*this)
    , fxPanel (*this)
{
    addAndMakeVisible (topBar);
    addAndMakeVisible (keyboardSection);
    addAndMakeVisible (knobPanel);
    addAndMakeVisible (modMatrixPanel);
    addAndMakeVisible (fxPanel);

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

    addModItems (modMatrixPanel.mod1Source, modSources);
    addModItems (modMatrixPanel.mod2Source, modSources);
    addModItems (modMatrixPanel.mod3Source, modSources);

    addModItems (modMatrixPanel.mod1Dest, modDests);
    addModItems (modMatrixPanel.mod2Dest, modDests);
    addModItems (modMatrixPanel.mod3Dest, modDests);

    addModItems (knobPanel.filterChar, filterChars);
    addModItems (fxPanel.lfo1Mode, lfoModes);
    addModItems (fxPanel.lfo2Mode, lfoModes);
    addModItems (fxPanel.lfo1Sync, lfoSyncs);
    addModItems (fxPanel.lfo2Sync, lfoSyncs);

    modMatrixPanel.mod1Source.setTooltip ("Mod 1 Source");
    modMatrixPanel.mod1Dest.setTooltip ("Mod 1 Destination");
    modMatrixPanel.mod2Source.setTooltip ("Mod 2 Source");
    modMatrixPanel.mod2Dest.setTooltip ("Mod 2 Destination");
    modMatrixPanel.mod3Source.setTooltip ("Mod 3 Source");
    modMatrixPanel.mod3Dest.setTooltip ("Mod 3 Destination");
    knobPanel.filterChar.setTooltip ("Filter Character");
    fxPanel.lfo1Mode.setTooltip ("LFO 1 Mode");
    fxPanel.lfo1Sync.setTooltip ("LFO 1 Sync");
    fxPanel.lfo2Mode.setTooltip ("LFO 2 Mode");
    fxPanel.lfo2Sync.setTooltip ("LFO 2 Sync");

    // Inventory (control -> parameter ID)
    // wave -> "wave"
    // cutoff -> "cutoff"
    // res -> "res"
    // envmod -> "envmod"
    // decay -> "decay"
    // release -> "release"
    // accent -> "accent"
    // glide -> "glide"
    // drive -> "drive"
    // sat -> "sat"
    // sub -> "sub"
    // unison -> "unison"
    // unisonSpread -> "unisonSpread"
    // gain -> "gain"
    // volume -> "volume"
    // filterChar -> "filterChar"
    // lfo1Rate -> "lfo1Rate"
    // lfo2Rate -> "lfo2Rate"
    // modEnvDecay -> "modEnvDecay"
    // lfo1Mode -> "lfo1Mode"
    // lfo1Sync -> "lfo1Sync"
    // lfo2Mode -> "lfo2Mode"
    // lfo2Sync -> "lfo2Sync"
    // mod1Amount -> "mod1Amount"
    // mod2Amount -> "mod2Amount"
    // mod3Amount -> "mod3Amount"
    // fxDrive -> "fxDrive"
    // fxChorus -> "fxChorus"
    // fxDelay -> "fxDelay"
    // fxDelayTime -> "fxDelayTime"
    // fxReverb -> "fxReverb"
    // mod1Source -> "mod1Source"
    // mod1Dest -> "mod1Dest"
    // mod2Source -> "mod2Source"
    // mod2Dest -> "mod2Dest"
    // mod3Source -> "mod3Source"
    // mod3Dest -> "mod3Dest"

    addSliderAttachment ("wave",   knobPanel.wave);
    addSliderAttachment ("cutoff", knobPanel.cutoff);
    addSliderAttachment ("res",    knobPanel.res);
    addSliderAttachment ("envmod", knobPanel.envmod);
    addSliderAttachment ("decay",  knobPanel.decay);
    addSliderAttachment ("release", knobPanel.release);
    addSliderAttachment ("accent", knobPanel.accent);
    addSliderAttachment ("glide",  knobPanel.glide);
    addSliderAttachment ("drive",  knobPanel.drive);
    addSliderAttachment ("sat",    knobPanel.sat);
    addSliderAttachment ("sub",    knobPanel.sub);
    addSliderAttachment ("unison", knobPanel.unison);
    addSliderAttachment ("unisonSpread", knobPanel.unisonSpread);
    addSliderAttachment ("gain",   knobPanel.gain);
    addSliderAttachment ("volume", knobPanel.volume);

    addSliderAttachment ("lfo1Rate", fxPanel.lfo1Rate);
    addSliderAttachment ("lfo2Rate", fxPanel.lfo2Rate);
    addSliderAttachment ("modEnvDecay", fxPanel.modEnvDecay);
    addComboAttachment ("lfo1Mode", fxPanel.lfo1Mode);
    addComboAttachment ("lfo1Sync", fxPanel.lfo1Sync);
    addComboAttachment ("lfo2Mode", fxPanel.lfo2Mode);
    addComboAttachment ("lfo2Sync", fxPanel.lfo2Sync);

    addSliderAttachment ("mod1Amount", modMatrixPanel.mod1Amount);
    addSliderAttachment ("mod2Amount", modMatrixPanel.mod2Amount);
    addSliderAttachment ("mod3Amount", modMatrixPanel.mod3Amount);

    addSliderAttachment ("fxDrive", fxPanel.fxDrive);
    addSliderAttachment ("fxChorus", fxPanel.fxChorus);
    addSliderAttachment ("fxDelay", fxPanel.fxDelay);
    addSliderAttachment ("fxDelayTime", fxPanel.fxDelayTime);
    addSliderAttachment ("fxReverb", fxPanel.fxReverb);

    addComboAttachment ("mod1Source", modMatrixPanel.mod1Source);
    addComboAttachment ("mod1Dest", modMatrixPanel.mod1Dest);
    addComboAttachment ("mod2Source", modMatrixPanel.mod2Source);
    addComboAttachment ("mod2Dest", modMatrixPanel.mod2Dest);
    addComboAttachment ("mod3Source", modMatrixPanel.mod3Source);
    addComboAttachment ("mod3Dest", modMatrixPanel.mod3Dest);
    addComboAttachment ("filterChar", knobPanel.filterChar);

    auto updateLfoUi = [this]
    {
        const bool lfo1SyncMode = (fxPanel.lfo1Mode.getSelectedItemIndex() == 1);
        fxPanel.lfo1Rate.setEnabled (! lfo1SyncMode);
        fxPanel.lfo1RateLabel.setEnabled (! lfo1SyncMode);
        fxPanel.lfo1RateValueLabel.setEnabled (! lfo1SyncMode);
        fxPanel.lfo1Rate.setVisible (! lfo1SyncMode);
        fxPanel.lfo1RateLabel.setVisible (! lfo1SyncMode);
        fxPanel.lfo1RateValueLabel.setVisible (! lfo1SyncMode);
        fxPanel.lfo1Sync.setVisible (lfo1SyncMode);
        fxPanel.lfo1SyncLabel.setVisible (lfo1SyncMode);
        fxPanel.lfo1ModeLabel.setVisible (true);

        const bool lfo2SyncMode = (fxPanel.lfo2Mode.getSelectedItemIndex() == 1);
        fxPanel.lfo2Rate.setEnabled (! lfo2SyncMode);
        fxPanel.lfo2RateLabel.setEnabled (! lfo2SyncMode);
        fxPanel.lfo2RateValueLabel.setEnabled (! lfo2SyncMode);
        fxPanel.lfo2Rate.setVisible (! lfo2SyncMode);
        fxPanel.lfo2RateLabel.setVisible (! lfo2SyncMode);
        fxPanel.lfo2RateValueLabel.setVisible (! lfo2SyncMode);
        fxPanel.lfo2Sync.setVisible (lfo2SyncMode);
        fxPanel.lfo2SyncLabel.setVisible (lfo2SyncMode);
        fxPanel.lfo2ModeLabel.setVisible (true);
    };

    fxPanel.lfo1Mode.onChange = updateLfoUi;
    fxPanel.lfo2Mode.onChange = updateLfoUi;
    updateLfoUi();

    // --- Double-click reset
    for (auto* s : { &knobPanel.wave, &knobPanel.cutoff, &knobPanel.res, &knobPanel.envmod, &knobPanel.decay,
                     &knobPanel.release, &knobPanel.accent, &knobPanel.glide, &knobPanel.drive, &knobPanel.sat,
                     &knobPanel.sub, &knobPanel.unison, &knobPanel.unisonSpread, &knobPanel.gain, &knobPanel.volume,
                     &fxPanel.lfo1Rate, &fxPanel.lfo2Rate, &fxPanel.modEnvDecay, &modMatrixPanel.mod1Amount,
                     &modMatrixPanel.mod2Amount, &modMatrixPanel.mod3Amount, &fxPanel.fxDrive, &fxPanel.fxChorus,
                     &fxPanel.fxDelay, &fxPanel.fxDelayTime, &fxPanel.fxReverb })
        s->setDoubleClickReturnValue (true, s->getValue());

    // --- Readout wiring
    knobPanel.waveValueLabel.setText (formatValue (knobPanel.wave), juce::dontSendNotification);
    knobPanel.cutoffValueLabel.setText (formatValue (knobPanel.cutoff), juce::dontSendNotification);
    knobPanel.resValueLabel.setText (formatValue (knobPanel.res), juce::dontSendNotification);
    knobPanel.envmodValueLabel.setText (formatValue (knobPanel.envmod), juce::dontSendNotification);
    knobPanel.decayValueLabel.setText (formatValue (knobPanel.decay), juce::dontSendNotification);
    knobPanel.releaseValueLabel.setText (formatValue (knobPanel.release), juce::dontSendNotification);
    knobPanel.accentValueLabel.setText (formatValue (knobPanel.accent), juce::dontSendNotification);
    knobPanel.glideValueLabel.setText (formatValue (knobPanel.glide), juce::dontSendNotification);
    knobPanel.driveValueLabel.setText (formatValue (knobPanel.drive), juce::dontSendNotification);
    knobPanel.satValueLabel.setText (formatValue (knobPanel.sat), juce::dontSendNotification);
    knobPanel.subValueLabel.setText (formatValue (knobPanel.sub), juce::dontSendNotification);
    knobPanel.unisonValueLabel.setText (formatValue (knobPanel.unison), juce::dontSendNotification);
    knobPanel.unisonSpreadValueLabel.setText (formatValue (knobPanel.unisonSpread), juce::dontSendNotification);
    knobPanel.gainValueLabel.setText (formatValue (knobPanel.gain), juce::dontSendNotification);
    knobPanel.volumeValueLabel.setText (formatValue (knobPanel.volume), juce::dontSendNotification);
    fxPanel.lfo1RateValueLabel.setText (formatValue (fxPanel.lfo1Rate), juce::dontSendNotification);
    fxPanel.lfo2RateValueLabel.setText (formatValue (fxPanel.lfo2Rate), juce::dontSendNotification);
    fxPanel.modEnvDecayValueLabel.setText (formatValue (fxPanel.modEnvDecay), juce::dontSendNotification);
    modMatrixPanel.mod1AmountValueLabel.setText (formatValue (modMatrixPanel.mod1Amount), juce::dontSendNotification);
    modMatrixPanel.mod2AmountValueLabel.setText (formatValue (modMatrixPanel.mod2Amount), juce::dontSendNotification);
    modMatrixPanel.mod3AmountValueLabel.setText (formatValue (modMatrixPanel.mod3Amount), juce::dontSendNotification);
    fxPanel.fxDriveValueLabel.setText (formatValue (fxPanel.fxDrive), juce::dontSendNotification);
    fxPanel.fxChorusValueLabel.setText (formatValue (fxPanel.fxChorus), juce::dontSendNotification);
    fxPanel.fxDelayValueLabel.setText (formatValue (fxPanel.fxDelay), juce::dontSendNotification);
    fxPanel.fxDelayTimeValueLabel.setText (formatValue (fxPanel.fxDelayTime), juce::dontSendNotification);
    fxPanel.fxReverbValueLabel.setText (formatValue (fxPanel.fxReverb), juce::dontSendNotification);

    wireReadout (knobPanel.wave,   "WAVE",   knobPanel.waveValueLabel);
    wireReadout (knobPanel.cutoff, "CUTOFF", knobPanel.cutoffValueLabel);
    wireReadout (knobPanel.res,    "RES",    knobPanel.resValueLabel);
    wireReadout (knobPanel.envmod, "ENVMOD", knobPanel.envmodValueLabel);
    wireReadout (knobPanel.decay,  "DECAY",  knobPanel.decayValueLabel);
    wireReadout (knobPanel.release, "RELEASE", knobPanel.releaseValueLabel);
    wireReadout (knobPanel.accent, "ACCENT", knobPanel.accentValueLabel);
    wireReadout (knobPanel.glide,  "GLIDE",  knobPanel.glideValueLabel);
    wireReadout (knobPanel.drive,  "DRIVE",  knobPanel.driveValueLabel);
    wireReadout (knobPanel.sat,    "SAT",    knobPanel.satValueLabel);
    wireReadout (knobPanel.sub,    "SUB",    knobPanel.subValueLabel);
    wireReadout (knobPanel.unison, "UNISON", knobPanel.unisonValueLabel);
    wireReadout (knobPanel.unisonSpread, "SPREAD", knobPanel.unisonSpreadValueLabel);
    wireReadout (knobPanel.gain,   "GAIN",   knobPanel.gainValueLabel);
    wireReadout (knobPanel.volume, "VOLUME", knobPanel.volumeValueLabel);
    wireReadout (fxPanel.lfo1Rate, "LFO 1 RATE", fxPanel.lfo1RateValueLabel);
    wireReadout (fxPanel.lfo2Rate, "LFO 2 RATE", fxPanel.lfo2RateValueLabel);
    wireReadout (fxPanel.modEnvDecay, "MOD ENV", fxPanel.modEnvDecayValueLabel);
    wireReadout (modMatrixPanel.mod1Amount, "MOD 1 AMT", modMatrixPanel.mod1AmountValueLabel);
    wireReadout (modMatrixPanel.mod2Amount, "MOD 2 AMT", modMatrixPanel.mod2AmountValueLabel);
    wireReadout (modMatrixPanel.mod3Amount, "MOD 3 AMT", modMatrixPanel.mod3AmountValueLabel);
    wireReadout (fxPanel.fxDrive, "FX DRIVE", fxPanel.fxDriveValueLabel);
    wireReadout (fxPanel.fxChorus, "CHORUS", fxPanel.fxChorusValueLabel);
    wireReadout (fxPanel.fxDelay, "DELAY", fxPanel.fxDelayValueLabel);
    wireReadout (fxPanel.fxDelayTime, "DELAY TIME", fxPanel.fxDelayTimeValueLabel);
    wireReadout (fxPanel.fxReverb, "REVERB", fxPanel.fxReverbValueLabel);

    updateReadout ("CUTOFF", knobPanel.cutoff);

    setSize (1020, 900);
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

    // Panels
    auto content = bounds.reduced (10);
    auto keyboardStrip = content.removeFromBottom (100);
    auto modFxPanel = content.removeFromBottom (240);
    auto knobPanel = content;

    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.fillRoundedRectangle (knobPanel.toFloat(), 10.0f);
    g.fillRoundedRectangle (modFxPanel.toFloat(), 10.0f);

    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawRoundedRectangle (knobPanel.toFloat(), 10.0f, 1.0f);
    g.drawRoundedRectangle (modFxPanel.toFloat(), 10.0f, 1.0f);

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
    auto topBarArea = area.removeFromTop (36);
    topBar.setBounds (topBarArea);

    // Main content
    auto content = area.reduced (10);

    // Keyboard strip
    const int keyboardHeight = juce::jlimit (70, 100, (int) (content.getHeight() * 0.16f));
    auto keyboardArea = content.removeFromBottom (keyboardHeight);
    keyboardSection.setBounds (keyboardArea);

    // Mod/FX panel area
    const int modFxHeight = juce::jlimit (260, 380, (int) (content.getHeight() * 0.45f));
    auto modFxPanel = content.removeFromBottom (modFxHeight);

    // Knob panel area
    knobPanel.setBounds (content);

    auto modFxArea = modFxPanel.reduced (18, 12);
    auto modArea = modFxArea.removeFromLeft (int (modFxArea.getWidth() * 0.62f));
    auto fxArea = modFxArea;
    modMatrixPanel.setBounds (modArea);
    fxPanel.setBounds (fxArea);

    layoutGuard();
}
