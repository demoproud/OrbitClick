# Orbit Click

<img width="1109" height="852" alt="Orbit Click plugin showing two circular rhythm clocks" src="https://github.com/user-attachments/assets/176d8ab5-bc78-4b14-bb4d-3a88408a5541" />

Orbit Click is a free, open-source Windows 64-bit VST3 instrument for hearing and seeing two rhythms at once. It is intended for DAWs that support Windows x64 VST3 instruments. It makes its own short click sounds; no samples or MIDI files are needed. Load it on an instrument track and start your DAW's transport.

## What it does

| Feature | Description |
| --- | --- |
| Two rhythm layers | Set each cycle's length as a number of notes and a note value, then choose evenly spaced hits per cycle. |
| Audible clicks | The layers use different pitches and decay times, with separate volume and mute controls. |
| Rhythm clocks | Circular hit markers show each layer's pattern; moving hands follow the audio timeline. |
| Realignment display | Shows the number of quarter-note beats until both cycles start together again. |
| Host sync | Follows the DAW's tempo and transport position, including starts in the middle of a song, seeking, and loops when the host supplies loop points. |
| Saved settings | Layer settings are stored with the DAW project. |

**Try 3:2 over one 4/4 bar:** keep both cycles at **8 × Eighth**, and set the hits to **3** and **2**. This is the default.

**Try a three-eighth-note cycle against 4/4:** set layer 1 to **3 × Eighth, 1 hit** and layer 2 to **8 × Eighth, 1 hit**. The cycle starts realign every three bars of 4/4.

The status line shows bar, beat, elapsed time, BPM, and next realignment. It uses the host's bar count when available; otherwise the bar label is estimated from the current meter.

## Download and install on Windows x64

Download [OrbitClick-0.1.0-win64.zip](dist/OrbitClick-0.1.0-win64.zip) and extract it. Copy the **entire** `Orbit Click.vst3` folder into `%CommonProgramFiles%\VST3`, then rescan plugins in your DAW and load Orbit Click as an instrument. Copying only the file inside the bundle will not install it correctly. The ZIP includes the licence and third-party notices. You can verify the download with [SHA256SUMS.txt](dist/SHA256SUMS.txt).

## Build on Windows x64

Install Visual Studio 2026 with the **Desktop development with C++** workload, the x64 MSVC build tools, a Windows SDK, and C++ CMake tools. Use CMake 4.2 or newer for the Visual Studio 2026 generator. JUCE 8.0.17-security is fetched from its official GitHub repository during configuration unless `JUCE_DIR` points to a local checkout. This project uses JUCE commit `dabd5ad13cd869cba41046947fffc8e5ce5cd662`. If your default `cmake` is older than 4.2, use Visual Studio's bundled CMake from PowerShell:

```powershell
$vsCmake = 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin'
& "$vsCmake\cmake.exe" -G "Visual Studio 18 2026" -S . -B build-2026 -A x64
& "$vsCmake\cmake.exe" --build build-2026 --config Release --target OrbitClick_VST3 orbit_timing_tests orbit_state_tests -- /m:1
& "$vsCmake\ctest.exe" --test-dir build-2026 -C Release --output-on-failure
```

The VST3 bundle is produced under `build-2026/OrbitClick_artefacts/Release/VST3/Orbit Click.vst3`. Keep your DAW's transport playing to hear the clicks. Visual Studio 2022 is also supported by changing the generator to `Visual Studio 17 2022` and using CMake 3.22 or newer.

## Timing design

Every audio block starts at the host's reported quarter-note position. The plugin calculates event positions per sample at the current host BPM, so starting mid-song, seeks, transport stops, and BPM changes between blocks do not depend on an accumulated internal clock. If the host provides loop points, the timeline wraps inside a block as well. Visuals read the position published by audio processing. The two short synthesized tones differ in pitch and decay. The audio callback uses no locks or heap allocation.

The realignment display is the least common multiple of the two cycle durations in 960-tick quarter-note units, anchored to song quarter note zero. Hit phase also anchors there. A loop with a start that is not on the rhythm grid follows the song timeline rather than restarting the pattern at the loop boundary.

## Scope and verification

Manual rhythm entry is supported. MIDI import and automatic cycle detection are later work. No personal MIDI data or music analysis files are included in this repository. `Tests/TimingTests.cpp` covers equal-cycle 3:2, different-length cycles, starting mid-song, off-grid seeking, loop wrapping, and a tempo change. `Tests/StateTests.cpp` checks a parameter save and restore roundtrip. Both tests passed on Windows x64 with Visual Studio 2026. Mixcraft 10.6 Pro Studio has been reported to load and play the plugin. Other DAWs have not been tested yet; feel free to try it in a Windows DAW that supports x64 VST3 instruments and [report what works or fails](https://github.com/demoproud/OrbitClick/issues). Detailed seek, loop, tempo-change, and session-restore behavior also remains open for host testing.

## Privacy and security

Orbit Click's own code has no account, telemetry, network calls, or runtime file access. It synthesizes audio from settings and timing supplied by the host. The build downloads JUCE from its official repository at the pinned commit above. The Windows package is unsigned; review the source and verify the ZIP checksum before installing a downloaded copy.

## Licence and source

Orbit Click is free to use and open source under the [GNU Affero General Public License, version 3](LICENSE). JUCE 8 is dual licensed under AGPLv3 and a commercial licence; this build uses the AGPLv3 option. The source for the Windows package is at [github.com/demoproud/OrbitClick](https://github.com/demoproud/OrbitClick), and JUCE is fetched at the pinned commit above. See [third-party notices](THIRD_PARTY_NOTICES.md) for bundled dependencies and licence texts. If you modify and distribute the plugin, the AGPLv3 requires you to provide corresponding source under its terms.
