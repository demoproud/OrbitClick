cmake_minimum_required(VERSION 3.22)
project(OrbitClick VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point JUCE_DIR at a JUCE checkout, or fetch the verified 8.0.17 security release.
if (DEFINED JUCE_DIR)
    add_subdirectory("${JUCE_DIR}" JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG dabd5ad13cd869cba41046947fffc8e5ce5cd662)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(OrbitClick
    COMPANY_NAME "Orbit Click"
    BUNDLE_ID com.orbitclick.instrument
    PLUGIN_MANUFACTURER_CODE Orbt
    PLUGIN_CODE OrCk
    FORMATS VST3
    PRODUCT_NAME "Orbit Click"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(OrbitClick)

target_sources(OrbitClick PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(OrbitClick PRIVATE Source)
target_compile_definitions(OrbitClick PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(OrbitClick PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

enable_testing()
add_executable(orbit_timing_tests Tests/TimingTests.cpp)
target_include_directories(orbit_timing_tests PRIVATE Source)
add_test(NAME orbit_timing_tests COMMAND orbit_timing_tests)

juce_add_console_app(orbit_state_tests PRODUCT_NAME "Orbit Click State Tests")
juce_generate_juce_header(orbit_state_tests)
target_sources(orbit_state_tests PRIVATE
    Tests/StateTests.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(orbit_state_tests PRIVATE Source)
target_compile_definitions(orbit_state_tests PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(orbit_state_tests PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    juce::juce_recommended_config_flags)
add_test(NAME orbit_state_tests COMMAND orbit_state_tests)
