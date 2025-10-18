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
#include "Core/StochasticEngine.h"
#include "Core/MIDIGenerator.h"
#include "Core/ScaleQuantizer.h"
#include "Core/SwingEngine.h"
#include "Core/GateLengthController.h"
#include "Core/RatchetEngine.h"
#include "Core/PresetManager.h"
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
    StochasticEngine& getStochasticEngine() { return stochasticEngine; }
    MIDIGenerator& getMIDIGenerator() { return midiGenerator; }
    ClockManager& getClockManager() { return clockManager; }
    EventScheduler& getEventScheduler() { return eventScheduler; }
    ScaleQuantizer& getScaleQuantizer() { return scaleQuantizer; }
    SwingEngine& getSwingEngine() { return swingEngine; }
    GateLengthController& getGateLengthController() { return gateLengthController; }
    RatchetEngine& getRatchetEngine() { return ratchetEngine; }
    PresetManager& getPresetManager() { return presetManager; }

    // Parameter tree
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    // Core engines
    EuclideanEngine euclideanEngine;
    PolyrhythmEngine polyrhythmEngine;
    AlgorithmicEngine algorithmicEngine;
    StochasticEngine stochasticEngine;
    MIDIGenerator midiGenerator;
    ClockManager clockManager;
    EventScheduler eventScheduler;
    ScaleQuantizer scaleQuantizer;
    SwingEngine swingEngine;
    GateLengthController gateLengthController;
    RatchetEngine ratchetEngine;

    // Parameters
    juce::AudioProcessorValueTreeState parameters;

    // Preset management (must be initialized after parameters)
    PresetManager presetManager;

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

    // Scale and humanization parameters
    static constexpr const char* PARAM_SCALE_ROOT = "scaleRoot";
    static constexpr const char* PARAM_SCALE_TYPE = "scaleType";
    static constexpr const char* PARAM_SWING_AMOUNT = "swingAmount";
    static constexpr const char* PARAM_TIMING_HUMANIZE = "timingHumanize";
    static constexpr const char* PARAM_VELOCITY_HUMANIZE = "velocityHumanize";
    static constexpr const char* PARAM_GATE_LENGTH = "gateLength";
    static constexpr const char* PARAM_LEGATO_MODE = "legatoMode";
    static constexpr const char* PARAM_RATCHET_COUNT = "ratchetCount";
    static constexpr const char* PARAM_RATCHET_PROBABILITY = "ratchetProbability";
    static constexpr const char* PARAM_RATCHET_DECAY = "ratchetDecay";

    // Stochastic/Chaos parameters
    static constexpr const char* PARAM_STOCHASTIC_TYPE = "stochasticType";
    static constexpr const char* PARAM_STEP_SIZE = "stepSize";
    static constexpr const char* PARAM_MOMENTUM = "momentum";
    static constexpr const char* PARAM_TIME_SCALE = "timeScale";

    // Processing state
    int64_t currentSamplePosition = 0;
    int lastSubdivisionStep = 0;

    // Helper methods
    void processGenerativeOutput(juce::MidiBuffer& midiMessages, int numSamples);
    void onSubdivisionHit(int subdivision);

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIProcessor)
};
