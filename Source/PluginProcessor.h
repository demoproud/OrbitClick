#pragma once
#include <JuceHeader.h>
#include "Timing.h"

class OrbitClickProcessor final : public juce::AudioProcessor
{
public:
    OrbitClickProcessor();
    void prepareToPlay (double sampleRate, int maximumBlockSize) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Orbit Click"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.08; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    struct Snapshot
    {
        double quarterPosition = 0.0;
        double bpm = 90.0;
        double seconds = 0.0;
        int numerator = 4;
        int denominator = 4;
        int64_t barCount = -1;
        double lastBarStart = -1.0;
        bool playing = false;
        bool hostPosition = false;
    };

    Snapshot snapshot() const noexcept;
    orbit::Layer layer (int index) const noexcept;
    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    struct Voice { double phase = 0.0; double envelope = 0.0; };
    Voice voices[2];
    struct ParameterPointers
    {
        std::atomic<float>* units = nullptr;
        std::atomic<float>* division = nullptr;
        std::atomic<float>* hits = nullptr;
        std::atomic<float>* volume = nullptr;
        std::atomic<float>* mute = nullptr;
    } parameterPointers[2];
    double currentSampleRate = 44100.0;
    double lastEndPosition = 0.0;
    bool wasPlaying = false;
    std::atomic<double> shownPosition { 0.0 }, shownBpm { 90.0 }, shownSeconds { 0.0 };
    std::atomic<int> shownNumerator { 4 }, shownDenominator { 4 };
    std::atomic<int64_t> shownBarCount { -1 };
    std::atomic<double> shownLastBarStart { -1.0 };
    std::atomic<bool> shownPlaying { false }, shownHostPosition { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OrbitClickProcessor)
};
