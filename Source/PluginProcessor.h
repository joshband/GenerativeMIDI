/*
  ==============================================================================
    PluginProcessor.h

    Main plugin processor integrating all generative engines

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/EuclideanEngine.h"
#include "Core/PolyrhythmEngine.h"
#include "Core/AlgorithmicEngine.h"
#include "Core/MIDIGenerator.h"
#include "DSP/ClockManager.h"
#include "DSP/EventScheduler.h"

class GenerativeMIDIProcessor : public juce::AudioProcessor
{
public:
    GenerativeMIDIProcessor();
    ~GenerativeMIDIProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Engine access
    EuclideanEngine& getEuclideanEngine() { return euclideanEngine; }
    PolyrhythmEngine& getPolyrhythmEngine() { return polyrhythmEngine; }
    AlgorithmicEngine& getAlgorithmicEngine() { return algorithmicEngine; }
    MIDIGenerator& getMIDIGenerator() { return midiGenerator; }
    ClockManager& getClockManager() { return clockManager; }
    EventScheduler& getEventScheduler() { return eventScheduler; }

    // Parameter tree
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    // Core engines
    EuclideanEngine euclideanEngine;
    PolyrhythmEngine polyrhythmEngine;
    AlgorithmicEngine algorithmicEngine;
    MIDIGenerator midiGenerator;
    ClockManager clockManager;
    EventScheduler eventScheduler;

    // Parameters
    juce::AudioProcessorValueTreeState parameters;

    // Parameter IDs
    static constexpr const char* PARAM_TEMPO = "tempo";
    static constexpr const char* PARAM_TIME_SIG_NUM = "timeSigNum";
    static constexpr const char* PARAM_TIME_SIG_DENOM = "timeSigDenom";
    static constexpr const char* PARAM_EUCLIDEAN_STEPS = "euclideanSteps";
    static constexpr const char* PARAM_EUCLIDEAN_PULSES = "euclideanPulses";
    static constexpr const char* PARAM_EUCLIDEAN_ROTATION = "euclideanRotation";
    static constexpr const char* PARAM_GENERATOR_TYPE = "generatorType";
    static constexpr const char* PARAM_NOTE_DENSITY = "noteDensity";
    static constexpr const char* PARAM_VELOCITY_MIN = "velocityMin";
    static constexpr const char* PARAM_VELOCITY_MAX = "velocityMax";
    static constexpr const char* PARAM_PITCH_MIN = "pitchMin";
    static constexpr const char* PARAM_PITCH_MAX = "pitchMax";

    // Processing state
    int64_t currentSamplePosition = 0;
    int lastSubdivisionStep = 0;

    // Helper methods
    void processGenerativeOutput(juce::MidiBuffer& midiMessages, int numSamples);
    void onSubdivisionHit(int subdivision);

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIProcessor)
};
