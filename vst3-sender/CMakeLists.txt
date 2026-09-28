cmake_minimum_required(VERSION 3.22)
project(VSTream VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.10
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VSTream
    COMPANY_NAME "VSTream"
    BUNDLE_ID "com.vstream.sender"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE VStr
    PLUGIN_CODE VStr
    FORMATS VST3
    PRODUCT_NAME "VSTream"
)

target_sources(VSTream PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/Session.cpp
    src/AudioRingBuffer.cpp
)

target_include_directories(VSTream PRIVATE include)

target_link_libraries(VSTream PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

target_compile_definitions(VSTream PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
