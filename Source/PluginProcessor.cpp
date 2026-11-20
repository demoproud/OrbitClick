#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <array>
#include <cmath>

namespace
{
juce::String id (int layer, const char* suffix)
{
    return "layer" + juce::String (layer + 1) + suffix;
}

}

juce::AudioProcessorValueTreeState::ParameterLayout OrbitClickProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 0; i < 2; ++i)
    {
        const int defaultHits = i == 0 ? 3 : 2;
        layout.add (std::make_unique<juce::AudioParameterInt> (id (i, "Units"),
                    "Layer " + juce::String (i + 1) + " cycle notes", 1, 32, 8));
        layout.add (std::make_unique<juce::AudioParameterChoice> (id (i, "Division"),
                    "Layer " + juce::String (i + 1) + " note value",
                    juce::StringArray { "Half", "Quarter", "Eighth", "Sixteenth" }, 2));
        layout.add (std::make_unique<juce::AudioParameterInt> (id (i, "Hits"),
                    "Layer " + juce::String (i + 1) + " hits", 1, 16, defaultHits));
        layout.add (std::make_unique<juce::AudioParameterFloat> (id (i, "Volume"),
                    "Layer " + juce::String (i + 1) + " volume", 0.0f, 1.0f, i == 0 ? 0.7f : 0.55f));
        layout.add (std::make_unique<juce::AudioParameterBool> (id (i, "Mute"),
                    "Layer " + juce::String (i + 1) + " mute", false));
    }
    return layout;
}

OrbitClickProcessor::OrbitClickProcessor()
    : juce::AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "OrbitClickState", createLayout())
{
    for (int i = 0; i < 2; ++i)
    {
        auto& p = parameterPointers[i];
        p.units = parameters.getRawParameterValue (id (i, "Units"));
        p.division = parameters.getRawParameterValue (id (i, "Division"));
        p.hits = parameters.getRawParameterValue (id (i, "Hits"));
        p.volume = parameters.getRawParameterValue (id (i, "Volume"));
        p.mute = parameters.getRawParameterValue (id (i, "Mute"));
    }
}

void OrbitClickProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    for (auto& voice : voices) voice = {};
    wasPlaying = false;
}

orbit::Layer OrbitClickProcessor::layer (int index) const noexcept
{
    const auto& p = parameterPointers[index];
    const int choice = juce::jlimit (0, 3, static_cast<int> (p.division->load (std::memory_order_relaxed)));
    return { static_cast<int> (p.units->load (std::memory_order_relaxed)),
             orbit::divisions[choice],
             static_cast<int> (p.hits->load (std::memory_order_relaxed)) };
}

OrbitClickProcessor::Snapshot OrbitClickProcessor::snapshot() const noexcept
{
    return { shownPosition.load(), shownBpm.load(), shownSeconds.load(), shownNumerator.load(),
             shownDenominator.load(), shownBarCount.load(), shownLastBarStart.load(),
             shownPlaying.load(), shownHostPosition.load() };
}

void OrbitClickProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    const auto* hostPlayHead = getPlayHead();
    if (hostPlayHead == nullptr)
    {
        shownHostPosition.store (false);
        shownPlaying.store (false);
        wasPlaying = false;
        return;
    }
    const auto pos = hostPlayHead->getPosition();
    if (! pos.hasValue())
    {
        shownHostPosition.store (false);
        shownPlaying.store (false);
        wasPlaying = false;
        return;
    }

    const auto bpm = pos->getBpm().orFallback (90.0);
    const auto ppq = pos->getPpqPosition();
    const auto playing = pos->getIsPlaying() && ppq.hasValue() && bpm > 0.0;
    auto signature = pos->getTimeSignature().orFallback (juce::AudioPlayHead::TimeSignature { 4, 4 });
    const auto loop = pos->getLoopPoints();
    const bool validLoop = pos->getIsLooping() && loop.hasValue() && loop->ppqEnd > loop->ppqStart;
    const double quarterPerSample = bpm / (60.0 * currentSampleRate);
    const auto layers = std::array<orbit::Layer, 2> { layer (0), layer (1) };
    const auto gains = std::array<double, 2> {
        parameterPointers[0].volume->load() * (parameterPointers[0].mute->load() < 0.5f ? 1.0 : 0.0),
        parameterPointers[1].volume->load() * (parameterPointers[1].mute->load() < 0.5f ? 1.0 : 0.0) };
    const auto decays = std::array<double, 2> {
        std::exp (-1.0 / (0.009 * currentSampleRate)),
        std::exp (-1.0 / (0.018 * currentSampleRate)) };

    shownBpm.store (bpm);
    shownNumerator.store (signature.numerator);
    shownDenominator.store (signature.denominator);
    shownBarCount.store (pos->getBarCount().orFallback (-1));
    shownLastBarStart.store (pos->getPpqPositionOfLastBarStart().orFallback (-1.0));
    shownHostPosition.store (ppq.hasValue());
    shownPlaying.store (playing);
    shownPosition.store (ppq.orFallback (0.0));
    shownSeconds.store (pos->getTimeInSeconds().orFallback (0.0));
    if (! playing)
    {
        wasPlaying = false;
        for (auto& voice : voices) voice = {};
        return;
    }
    if (! wasPlaying || std::abs (*ppq - lastEndPosition) > quarterPerSample * 2.0)
        for (auto& voice : voices) voice = {};
    wasPlaying = true;

    const auto samples = buffer.getNumSamples();
    const auto channels = buffer.getNumChannels();
    for (int sample = 0; sample < samples; ++sample)
    {
        double quarter = *ppq + sample * quarterPerSample;
        const bool crossingLoop = validLoop
                               && quarter < loop->ppqEnd && quarter + quarterPerSample > loop->ppqEnd;
        if (validLoop)
            quarter = orbit::wrapLoop (quarter, loop->ppqStart, loop->ppqEnd);
        double mixed = 0.0;
        for (int i = 0; i < 2; ++i)
        {
            auto& voice = voices[i];
            const bool hit = orbit::isHitAtSample (quarter, crossingLoop ? loop->ppqEnd - quarter : quarterPerSample, layers[i])
                || (crossingLoop && orbit::isHitAtSample (loop->ppqStart,
                                    quarter + quarterPerSample - loop->ppqEnd, layers[i]));
            if (gains[i] > 0.0 && hit)
            {
                voice.phase = 0.0;
                voice.envelope = 1.0;
            }
            const double frequency = i == 0 ? 1420.0 : 690.0;
            const double tone = std::sin (voice.phase) * voice.envelope;
            const double attack = voice.envelope > 0.93 ? (voice.envelope - 0.93) * 2.0 : 0.0;
            mixed += gains[i] * (tone + attack) * 0.33;
            voice.phase += juce::MathConstants<double>::twoPi * frequency / currentSampleRate;
            if (voice.phase > juce::MathConstants<double>::twoPi)
                voice.phase -= juce::MathConstants<double>::twoPi;
            voice.envelope *= decays[i];
            if (voice.envelope < 1.0e-6) voice.envelope = 0.0;
        }
        const auto output = static_cast<float> (juce::jlimit (-1.0, 1.0, mixed));
        for (int channel = 0; channel < channels; ++channel)
            buffer.setSample (channel, sample, output);
    }
    double end = *ppq + samples * quarterPerSample;
    if (validLoop)
        end = orbit::wrapLoop (end, loop->ppqStart, loop->ppqEnd);
    shownPosition.store (end);
    shownSeconds.store (pos->getTimeInSeconds().orFallback (0.0) + samples / currentSampleRate);
    lastEndPosition = end;
}

void OrbitClickProcessor::getStateInformation (juce::MemoryBlock& data)
{
    auto state = parameters.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, data);
}

void OrbitClickProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* OrbitClickProcessor::createEditor()
{
    return new OrbitClickEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OrbitClickProcessor();
}
