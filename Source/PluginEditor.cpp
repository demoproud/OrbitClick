#include "PluginEditor.h"
#include <array>
#include <cmath>

namespace
{
const auto background = juce::Colour (0xff101724);
const auto muted = juce::Colour (0xff8794a9);
const auto colors = std::array<juce::Colour, 2> { juce::Colour (0xff54e0c4), juce::Colour (0xffffb968) };

juce::String id (int layer, const char* suffix)
{
    return "layer" + juce::String (layer + 1) + suffix;
}

void setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, muted);
    label.setJustificationType (juce::Justification::centredLeft);
}
}

OrbitClickEditor::OrbitClickEditor (OrbitClickProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setSize (880, 620);
    for (int i = 0; i < 2; ++i)
    {
        auto& c = controls[i];
        for (auto* slider : { &c.units, &c.hits, &c.volume })
        {
            slider->setSliderStyle (juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 22);
            slider->setColour (juce::Slider::thumbColourId, colors[i]);
            slider->setColour (juce::Slider::trackColourId, colors[i].withAlpha (0.7f));
            addAndMakeVisible (*slider);
        }
        c.units.setRange (1, 32, 1);
        c.hits.setRange (1, 16, 1);
        c.volume.setRange (0.0, 1.0, 0.01);
        c.volume.setTextValueSuffix (" ");
        c.division.addItemList ({ "Half", "Quarter", "Eighth", "Sixteenth" }, 1);
        addAndMakeVisible (c.division);
        c.mute.setButtonText ("Mute");
        addAndMakeVisible (c.mute);
        setupLabel (c.unitsLabel, "Cycle notes");
        setupLabel (c.divisionLabel, "Note value");
        setupLabel (c.hitsLabel, "Hits per cycle");
        setupLabel (c.volumeLabel, "Volume");
        for (auto* label : { &c.unitsLabel, &c.divisionLabel, &c.hitsLabel, &c.volumeLabel })
            addAndMakeVisible (*label);
        c.unitsLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.parameters, id (i, "Units"), c.units);
        c.divisionLink = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.parameters, id (i, "Division"), c.division);
        c.hitsLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.parameters, id (i, "Hits"), c.hits);
        c.volumeLink = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.parameters, id (i, "Volume"), c.volume);
        c.muteLink = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.parameters, id (i, "Mute"), c.mute);
    }
    startTimerHz (30);
}

void OrbitClickEditor::resized()
{
    for (int i = 0; i < 2; ++i)
    {
        const int x = 55 + i * 440;
        auto& c = controls[i];
        const int y = 370;
        c.unitsLabel.setBounds (x, y, 130, 24);
        c.units.setBounds (x + 145, y, 230, 24);
        c.divisionLabel.setBounds (x, y + 42, 130, 24);
        c.division.setBounds (x + 145, y + 42, 230, 24);
        c.hitsLabel.setBounds (x, y + 84, 130, 24);
        c.hits.setBounds (x + 145, y + 84, 230, 24);
        c.volumeLabel.setBounds (x, y + 126, 130, 24);
        c.volume.setBounds (x + 145, y + 126, 230, 24);
        c.mute.setBounds (x + 145, y + 168, 110, 26);
    }
}

void OrbitClickEditor::drawClock (juce::Graphics& g, int index, juce::Point<float> centre, float radius)
{
    const auto layer = processor.layer (index);
    const auto position = processor.snapshot();
    const auto color = colors[index];
    const auto ring = juce::Rectangle<float> (centre.x - radius, centre.y - radius, 2 * radius, 2 * radius);
    g.setColour (juce::Colour (0xff2b3a4e));
    g.drawEllipse (ring, 3.0f);
    for (int hit = 0; hit < layer.hits; ++hit)
    {
        const float angle = juce::MathConstants<float>::twoPi * hit / layer.hits - juce::MathConstants<float>::halfPi;
        const auto point = centre + juce::Point<float> (std::cos (angle) * radius, std::sin (angle) * radius);
        g.setColour (color);
        g.fillEllipse (juce::Rectangle<float> (10, 10).withCentre (point));
    }
    const float angle = juce::MathConstants<float>::twoPi * static_cast<float> (orbit::phase (position.quarterPosition, layer))
                      - juce::MathConstants<float>::halfPi;
    const auto tip = centre + juce::Point<float> (std::cos (angle) * (radius - 15), std::sin (angle) * (radius - 15));
    g.setColour (color.withAlpha (position.playing ? 1.0f : 0.4f));
    g.drawLine (juce::Line<float> (centre, tip), 4.0f);
    g.fillEllipse (juce::Rectangle<float> (12, 12).withCentre (centre));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("LAYER " + juce::String (index + 1), juce::Rectangle<int> (static_cast<int> (centre.x) - 100, 66, 200, 28),
                juce::Justification::centred);
    g.setColour (muted);
    g.setFont (juce::FontOptions (14.0f));
    const auto text = juce::String (layer.hits) + " hits / " + juce::String (layer.units) + " × 1/" + juce::String (layer.division);
    g.drawText (text, juce::Rectangle<int> (static_cast<int> (centre.x) - 135, 318, 270, 24), juce::Justification::centred);
}

void OrbitClickEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("ORBIT CLICK", 28, 14, 280, 40, juce::Justification::centredLeft);
    g.setColour (muted);
    g.setFont (juce::FontOptions (13.0f));
    g.drawText ("HOST SYNCED RHYTHM CLOCKS", 620, 20, 230, 30, juce::Justification::centredRight);
    drawClock (g, 0, { 220.0f, 202.0f }, 105.0f);
    drawClock (g, 1, { 660.0f, 202.0f }, 105.0f);

    const auto s = processor.snapshot();
    const auto a = processor.layer (0);
    const auto b = processor.layer (1);
    const double alignment = static_cast<double> (orbit::realignmentTicks (a, b)) / orbit::ticksPerQuarter;
    const double phase = orbit::positiveModulo (s.quarterPosition, alignment);
    const double remaining = phase < 1.0e-7 ? 0.0 : alignment - phase;
    const double quartersPerBar = s.numerator * 4.0 / juce::jmax (1, s.denominator);
    const int bar = s.barCount >= 0 ? static_cast<int> (s.barCount + 1)
                                     : 1 + static_cast<int> (std::floor (juce::jmax (0.0, s.quarterPosition) / quartersPerBar));
    const double barOffset = s.lastBarStart >= 0.0 ? s.quarterPosition - s.lastBarStart
                                                      : orbit::positiveModulo (s.quarterPosition, quartersPerBar);
    const int beat = 1 + static_cast<int> (std::floor (juce::jmax (0.0, barOffset) * s.denominator / 4.0));
    const int minutes = static_cast<int> (s.seconds) / 60;
    const int seconds = static_cast<int> (s.seconds) % 60;
    g.setColour (juce::Colour (0xff1c2939));
    g.fillRoundedRectangle (20.0f, 573.0f, 840.0f, 32.0f, 8.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    const auto status = s.hostPosition ? (s.playing ? "PLAY" : "STOP") : "WAITING FOR HOST POSITION";
    const auto timing = "Bar " + juce::String (bar) + " beat " + juce::String (beat)
        + "  ·  " + juce::String (minutes) + ":" + juce::String (seconds).paddedLeft ('0', 2)
        + "  ·  " + juce::String (s.bpm, 1) + " BPM";
    g.drawText (juce::String (status) + "   " + timing, 32, 575, 560, 28, juce::Justification::centredLeft);
    g.setColour (colors[0]);
    g.drawText ("Realign in " + juce::String (remaining, 2) + " beats", 600, 575, 245, 28,
                juce::Justification::centredRight);
}
