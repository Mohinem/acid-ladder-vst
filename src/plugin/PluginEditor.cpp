#include "PluginEditor.h"

AcidSynthAudioProcessorEditor::AcidSynthAudioProcessorEditor (AcidSynthAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
{
    auto& apvts = audioProcessor.apvts;

    setupKnob (wave,   waveL,   "Wave");
    setupKnob (cutoff, cutoffL, "Cutoff");
    setupKnob (res,    resL,    "Res");
    setupKnob (envmod, envmodL, "Env");
    setupKnob (decay,  decayL,  "Decay");
    setupKnob (accent, accentL, "Accent");
    setupKnob (glide,  glideL,  "Glide");
    setupKnob (drive,  driveL,  "Drive");
    setupKnob (gain,   gainL,   "Gain");

    waveA   = std::make_unique<SliderAttachment> (apvts, "wave",   wave);
    cutoffA = std::make_unique<SliderAttachment> (apvts, "cutoff", cutoff);
    resA    = std::make_unique<SliderAttachment> (apvts, "res",    res);
    envmodA = std::make_unique<SliderAttachment> (apvts, "envmod", envmod);
    decayA  = std::make_unique<SliderAttachment> (apvts, "decay",  decay);
    accentA = std::make_unique<SliderAttachment> (apvts, "accent", accent);
    glideA  = std::make_unique<SliderAttachment> (apvts, "glide",  glide);
    driveA  = std::make_unique<SliderAttachment> (apvts, "drive",  drive);
    gainA   = std::make_unique<SliderAttachment> (apvts, "gain",   gain);

    setSize (560, 260);
}

void AcidSynthAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (s);
    addAndMakeVisible (l);
}

void AcidSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawText ("Acid Synth (Ladder + Accent + Slide)", 10, 10, getWidth()-20, 20, juce::Justification::centredLeft);

    g.setColour (juce::Colours::darkgrey);
    g.drawRect (getLocalBounds().reduced (6));
}

void AcidSynthAudioProcessorEditor::resized()
{
    const int pad = 14;
    const int knobW = 92;
    const int knobH = 92;

    auto r = getLocalBounds().reduced (pad);
    r.removeFromTop (28);

    auto row1 = r.removeFromTop (knobH + 28);
    auto row2 = r.removeFromTop (knobH + 28);

    auto place = [&] (juce::Slider& s, juce::Label& l, juce::Rectangle<int> area)
    {
        l.setBounds (area.removeFromTop (18));
        s.setBounds (area);
    };

    auto c1 = row1.removeFromLeft (knobW);
    place (wave, waveL, c1);
    row1.removeFromLeft (pad);

    auto c2 = row1.removeFromLeft (knobW);
    place (cutoff, cutoffL, c2);
    row1.removeFromLeft (pad);

    auto c3 = row1.removeFromLeft (knobW);
    place (res, resL, c3);
    row1.removeFromLeft (pad);

    auto c4 = row1.removeFromLeft (knobW);
    place (envmod, envmodL, c4);
    row1.removeFromLeft (pad);

    auto c5 = row1.removeFromLeft (knobW);
    place (decay, decayL, c5);

    // row 2
    auto d1 = row2.removeFromLeft (knobW);
    place (accent, accentL, d1);
    row2.removeFromLeft (pad);

    auto d2 = row2.removeFromLeft (knobW);
    place (glide, glideL, d2);
    row2.removeFromLeft (pad);

    auto d3 = row2.removeFromLeft (knobW);
    place (drive, driveL, d3);
    row2.removeFromLeft (pad);

    auto d4 = row2.removeFromLeft (knobW);
    place (gain, gainL, d4);
}
