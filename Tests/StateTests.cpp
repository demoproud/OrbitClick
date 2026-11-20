#include "PluginProcessor.h"
#include <cmath>
#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInitialiser;
    OrbitClickProcessor original;
    auto set = [&] (const juce::String& key, float value)
    {
        auto* parameter = original.parameters.getParameter (key);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    };
    set ("layer1Units", 3.0f);
    set ("layer1Hits", 1.0f);
    set ("layer2Mute", 1.0f);
    set ("layer2Volume", 0.23f);

    juce::MemoryBlock bytes;
    original.getStateInformation (bytes);
    if (bytes.isEmpty()) return 1;
    OrbitClickProcessor restored;
    restored.setStateInformation (bytes.getData(), static_cast<int> (bytes.getSize()));
    auto value = [&] (const juce::String& key)
    {
        return restored.parameters.getRawParameterValue (key)->load();
    };
    if (restored.layer (0).units != 3 || restored.layer (0).hits != 1
        || value ("layer2Mute") < 0.5f
        || std::abs (value ("layer2Volume") - 0.23f) > 0.001f)
        return 2;
    std::cout << "state roundtrip passed\n";
}
