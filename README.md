# Orbit Click

<img width="1109" height="852" alt="image" src="https://github.com/user-attachments/assets/176d8ab5-bc78-4b14-bb4d-3a88408a5541" />

Orbit Click is a Windows VST3 instrument with two audible rhythm clocks. Each layer has a cycle length (number of notes × note value), evenly spaced hits per cycle, volume, and mute. The defaults make a 3:2 rhythm over one 4/4 bar. For a three-eighth-note cycle against a 4/4 bar, set layer 1 to **3 × Eighth, 1 hit** and layer 2 to **8 × Eighth, 1 hit**. The clocks use the host's quarter-note position, and the status line shows bar, beat, elapsed time, BPM, and the next cycle realignment. A host should supply bar count and elapsed time for exact labels; otherwise the bar label is estimated from the current meter.

## Build on Windows x64

Install Visual Studio 2026 with the **Desktop development with C++** workload, the x64 MSVC build tools, a Windows SDK, and C++ CMake tools. Use CMake 4.2 or newer for the Visual Studio 2026 generator. JUCE 8.0.12 is fetched during configuration unless `JUCE_DIR` points to a local checkout. This machine's default `cmake` is 4.1, so use Visual Studio's bundled CMake from PowerShell:

```powershell
$vsCmake = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin'
& "$vsCmake\cmake.exe" -G "Visual Studio 18 2026" -S . -B build-2026 -A x64
& "$vsCmake\cmake.exe" --build build-2026 --config Release --target OrbitClick_VST3 orbit_timing_tests orbit_state_tests -- /m:1
& "$vsCmake\ctest.exe" --test-dir build-2026 -C Release --output-on-failure
```

The VST3 bundle is produced under `build-2026/OrbitClick_artefacts/Release/VST3/Orbit Click.vst3`. Copy that complete folder to `%CommonProgramFiles%\VST3`, rescan plugins in Mixcraft, and load it as an instrument. Keep Mixcraft's transport playing to hear the clicks.

## Timing design

Every audio block starts at the host's reported quarter-note position. The plugin calculates event positions per sample at the current host BPM, so starting mid-song, seeks, transport stops, and BPM changes between blocks do not depend on an accumulated internal clock. If the host provides loop points, the timeline wraps inside a block as well. Visuals read the position published by audio processing. The two short synthesized tones differ in pitch and decay. The audio callback uses no locks or heap allocation.

The realignment display is the least common multiple of the two cycle durations in 960-tick quarter-note units, anchored to song quarter note zero. Hit phase also anchors there. A loop with a start that is not on the rhythm grid follows the song timeline rather than restarting the pattern at the loop boundary.

## Scope and verification

Manual rhythm entry is supported. MIDI import and automatic cycle detection are later work. The existing MIDI analysis files and source MIDI are not read or modified. `Tests/TimingTests.cpp` covers equal-cycle 3:2, different-length cycles, starting mid-song, off-grid seeking, and loop wrapping. `Tests/StateTests.cpp` checks a parameter save and restore roundtrip. Mixcraft 10.6 Pro Studio playback, plugin scanning, session restore, and loop behavior require testing in the host.

## JUCE licence

JUCE 8 is available under AGPLv3 or a commercial JUCE licence. This source is provided for local evaluation. Before distributing a built plugin, choose and comply with the applicable JUCE licence and the licence requirements for your own code. See [JUCE's licence](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md) and [JUCE 8 terms](https://juce.com/legal/juce-8-licence/).
