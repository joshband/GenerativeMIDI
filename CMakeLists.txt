# CMakeLists.txt for GenerativeMIDI
# Modern CMake-based build system

cmake_minimum_required(VERSION 3.15)

project(GenerativeMIDI VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE - using symlink to ~/JUCE
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    message(FATAL_ERROR "JUCE not found. Please run: ln -s ~/JUCE JUCE")
endif()

# Create the plugin target
juce_add_plugin(GenerativeMIDI
    COMPANY_NAME "OpenSource"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Osrc
    PLUGIN_CODE Gmid
    FORMATS AU AUv3 VST3 Standalone
    PRODUCT_NAME "Generative MIDI"
    BUNDLE_ID "com.opensource.generativemidi"
    IPHONE_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight UIInterfaceOrientationPortrait
    IPAD_SCREEN_ORIENTATIONS UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown)

# Source files
set(SOURCE_FILES
    Source/Core/EuclideanEngine.cpp
    Source/Core/EuclideanEngine.h
    Source/Core/PolyrhythmEngine.cpp
    Source/Core/PolyrhythmEngine.h
    Source/Core/AlgorithmicEngine.cpp
    Source/Core/AlgorithmicEngine.h
    Source/Core/StochasticEngine.cpp
    Source/Core/StochasticEngine.h
    Source/Core/MIDIGenerator.cpp
    Source/Core/MIDIGenerator.h
    Source/DSP/ClockManager.cpp
    Source/DSP/ClockManager.h
    Source/DSP/EventScheduler.cpp
    Source/DSP/EventScheduler.h
    Source/Core/PresetManager.cpp
    Source/Core/PresetManager.h
    Source/UI/PresetBrowser.cpp
    Source/UI/PresetBrowser.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# Add source files to the plugin
target_sources(GenerativeMIDI PRIVATE ${SOURCE_FILES})

# Compiler definitions
target_compile_definitions(GenerativeMIDI
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link JUCE libraries
target_link_libraries(GenerativeMIDI
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
