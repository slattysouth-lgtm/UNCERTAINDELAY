#include "PluginEditor.h"
#include <cmath>

UncertainVoiceEditor::UncertainVoiceEditor (UncertainVoiceProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    title.setText ("UNCERTAIN VOICE", juce::dontSendNotification);
    title.setFont (juce::Font (18.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, cText);
    addAndMakeVisible (title);

    keyLabel.setFont (juce::Font (26.0f, juce::Font::bold));
    keyLabel.setColour (juce::Label::textColourId, cText);
    addAndMakeVisible (keyLabel);

    keySub.setFont (juce::Font (11.0f));
    keySub.setColour (juce::Label::textColourId, cDim);
    addAndMakeVisible (keySub);

    forceLabel.setText ("FORCE", juce::dontSendNotification);
    forceLabel.setFont (juce::Font (10.0f));
    forceLabel.setJustificationType (juce::Justification::centred);
    forceLabel.setColour (juce::Label::textColourId, cDim);
    addAndMakeVisible (forceLabel);

    force.setRange (0.0, 100.0, 1.0);
    force.setValue (35.0);
    force.setColour (juce::Slider::rotarySliderFillColourId, cCta);
    force.setColour (juce::Slider::rotarySliderOutlineColourId, cLine);
    force.setColour (juce::Slider::thumbColourId, cText);
    addAndMakeVisible (force);

    setSize (420, 560);
    startTimerHz (30);
}

void UncertainVoiceEditor::resized()
{
    auto r = getLocalBounds().reduced (16);
    title.setBounds (r.removeFromTop (28));
    r.removeFromTop (8);

    auto keyArea = r.removeFromTop (70);
    keyLabel.setBounds (keyArea.removeFromTop (40));
    keySub.setBounds (keyArea);
    r.removeFromTop (8);

    r.removeFromBottom (10);
    auto knob = r.removeFromBottom (150);
    force.setBounds (knob.withSizeKeepingCentre (140, 140));
    forceLabel.setBounds (knob.getCentreX() - 40, knob.getCentreY() + 44, 80, 16);
}

void UncertainVoiceEditor::paint (juce::Graphics& g)
{
    g.fillAll (cBg);

    // gamme détectée live
    const auto k = proc.getKey();
    keyLabel.setText (k.nameFr(), juce::dontSendNotification);
    keySub.setText ("gamme detectee automatiquement  -  confiance "
                        + juce::String ((int) std::round (k.confidence * 100.0f)) + "%",
                    juce::dontSendNotification);

    // panneau trace
    auto full = getLocalBounds().reduced (16).toFloat();
    full.removeFromTop (28 + 8 + 70 + 8);
    auto traceArea = full.removeFromTop (200.0f);
    g.setColour (cPanel);
    g.fillRoundedRectangle (traceArea, 10.0f);
    drawTrace (g, traceArea.reduced (10.0f));

    // legende
    g.setFont (10.0f);
    g.setColour (cRaw); g.fillRect (traceArea.getX() + 12.0f, traceArea.getBottom() - 16.0f, 14.0f, 2.0f);
    g.setColour (cDim); g.drawText ("TA VOIX", (int) traceArea.getX() + 30, (int) traceArea.getBottom() - 22, 80, 12, juce::Justification::left);
}

void UncertainVoiceEditor::drawTrace (juce::Graphics& g, juce::Rectangle<float> area)
{
    const auto snap = proc.getTrace();
    const auto key  = proc.getKey();

    // plage midi observee
    float lo = 1.0e9f, hi = -1.0e9f;
    for (int i = 0; i < snap.count; ++i)
    {
        const float m = snap.midi[(size_t) i];
        if (! std::isnan (m)) { lo = juce::jmin (lo, m); hi = juce::jmax (hi, m); }
    }
    if (lo > hi) { g.setColour (cDim); g.setFont (12.0f);
        g.drawText ("En attente d'audio...", area, juce::Justification::centred); return; }

    lo = std::floor (lo) - 2.0f; hi = std::ceil (hi) + 2.0f;
    auto yOf = [&] (float m) { return area.getBottom() - (m - lo) / (hi - lo) * area.getHeight(); };
    auto xOf = [&] (int i)   { return area.getX() + (float) i / (float) (snap.count - 1) * area.getWidth(); };

    // lignes de gamme
    for (int m = (int) lo; m <= (int) hi; ++m)
    {
        int pc = m % 12; if (pc < 0) pc += 12;
        bool inScale = false, isRoot = (pc == key.root);
        for (int s : key.pcs) if (s == pc) inScale = true;

        g.setColour (isRoot ? cCta.withAlpha (0.35f)
                    : inScale ? juce::Colours::white.withAlpha (0.10f)
                              : juce::Colours::white.withAlpha (0.03f));
        const float y = yOf ((float) m);
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
    }

    // trace brute (ambre)
    juce::Path path; bool pen = false;
    for (int i = 0; i < snap.count; ++i)
    {
        const float m = snap.midi[(size_t) i];
        if (std::isnan (m)) { pen = false; continue; }
        const float x = xOf (i), y = yOf (m);
        if (! pen) { path.startNewSubPath (x, y); pen = true; }
        else        path.lineTo (x, y);
    }
    g.setColour (cRaw);
    g.strokePath (path, juce::PathStrokeType (2.0f));
}
