#pragma once
#include "PluginProcessor.h"

class OrbitClickEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OrbitClickEditor (OrbitClickProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override { repaint(); }
    void drawClock (juce::Graphics&, int, juce::Point<float>, float);
    struct Controls
    {
        juce::Slider units, hits, volume;
        juce::ComboBox division;
        juce::ToggleButton mute;
        juce::Label unitsLabel, hitsLabel, volumeLabel, divisionLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unitsLink, hitsLink, volumeLink;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionLink;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteLink;
    } controls[2];
    OrbitClickProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitClickEditor)
};
