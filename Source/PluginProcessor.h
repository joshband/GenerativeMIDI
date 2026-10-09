/*
  ==============================================================================
    PluginProcessor.h

    Main plugin processor integrating all generative engines

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "Core/EuclideanEngine.h"
#include "Core/PolyrhythmEngine.h"
#include "Core/AlgorithmicEngine.h"
#include "Core/StochasticEngine.h"
#include "Core/MIDIGenerator.h"
#include "Core/ScaleQuantizer.h"
#include "Core/HarmonyParts.h"
#include "Core/SwingEngine.h"
#include "Core/GateLengthController.h"
#include "Core/RatchetEngine.h"
#include "Core/PresetManager.h"
#include "DSP/ClockManager.h"
#include "DSP/EventScheduler.h"
#include "DSP/MidiActivityLog.h"
#include "DSP/PianoSynth.h"
#include "Modulation/ModLfo.h"
#include "Modulation/ModulationDestination.h"
#include "Modulation/ModulationRouter.h"

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
    bool canAddBus(bool isInput) const override;
    bool canApplyBusCountChange(bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties) override;

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
    ModLfo& getModLfo() { return modLfo; }
    const ModLfo& getModLfo() const { return modLfo; }

    // Parameter tree
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

    // Playback state
    int getCurrentStep() const { return lastSubdivisionStep; }
    uint32_t getNoteActivityCount() const { return noteActivityCounter.load(std::memory_order_relaxed); }
    bool isClockAdvancing() const { return clockAdvancing.load(std::memory_order_relaxed); }
    MidiActivityLog& getMidiActivityLog() { return midiActivityLog; }

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
    // Legacy: retained for session compatibility; unused by DSP (see createParameterLayout).
    static constexpr const char* PARAM_STOCHASTIC_TYPE = "stochasticType";
    static constexpr const char* PARAM_STEP_SIZE = "stepSize";
    static constexpr const char* PARAM_MOMENTUM = "momentum";
    static constexpr const char* PARAM_TIME_SCALE = "timeScale";

    static constexpr const char* PARAM_MARKOV_ORDER = "markovOrder";
    static constexpr const char* PARAM_MARKOV_STEP = "markovStep";
    static constexpr const char* PARAM_MARKOV_SURPRISE = "markovSurprise";
    static constexpr const char* PARAM_LSYSTEM_GRAMMAR = "lsystemGrammar";
    static constexpr const char* PARAM_LSYSTEM_GENERATION = "lsystemGeneration";
    static constexpr const char* PARAM_LSYSTEM_INTERVAL = "lsystemInterval";
    static constexpr const char* PARAM_CELLULAR_RULE = "cellularRule";
    static constexpr const char* PARAM_CELLULAR_SEED = "cellularSeed";
    static constexpr const char* PARAM_CELLULAR_LISTEN = "cellularListen";

    // MIDI routing parameters
    static constexpr const char* PARAM_MIDI_CHANNEL = "midiChannel";
    static constexpr const char* PARAM_VOICE_MODE = "voiceMode";
    static constexpr const char* PARAM_PART_COUNT = "partCount";

    // MIDI expression parameters
    static constexpr const char* PARAM_AFTERTOUCH_ENABLE = "aftertouchEnable";
    static constexpr const char* PARAM_AFTERTOUCH_AMOUNT = "aftertouchAmount";
    static constexpr const char* PARAM_PITCHBEND_ENABLE = "pitchbendEnable";
    static constexpr const char* PARAM_PITCHBEND_RANGE = "pitchbendRange";
    static constexpr const char* PARAM_CC_ENABLE = "ccEnable";
    static constexpr const char* PARAM_CC_NUMBER = "ccNumber";
    static constexpr const char* PARAM_CC_AMOUNT = "ccAmount";

    // Modulation v2 MVP (LFO → velocity); see docs/developer/MODULATION_V2.md
    static constexpr const char* PARAM_MOD_LFO_ENABLE = "modLfoEnable";
    static constexpr const char* PARAM_MOD_LFO_RATE = "modLfoRate";
    static constexpr const char* PARAM_MOD_LFO_DEPTH = "modLfoDepth";
    static constexpr const char* PARAM_MOD_LFO_DENSITY_DEPTH = "modLfoDensityDepth";
    static constexpr const char* PARAM_MOD_SH_RATE = "modShRate";
    static constexpr const char* PARAM_MOD_ROUTE3_SOURCE = "modRoute3Source";
    static constexpr const char* PARAM_MOD_ROUTE3_DEST = "modRoute3Dest";
    static constexpr const char* PARAM_MOD_ROUTE3_AMOUNT = "modRoute3Amount";
    static constexpr const char* PARAM_MOD_ROUTE4_SOURCE = "modRoute4Source";
    static constexpr const char* PARAM_MOD_ROUTE4_DEST = "modRoute4Dest";
    static constexpr const char* PARAM_MOD_ROUTE4_AMOUNT = "modRoute4Amount";
    static constexpr const char* PARAM_PIANO_ENABLE = "pianoEnable";

    // Processing state
    int64_t currentSamplePosition = 0;
    int lastSubdivisionStep = 0;
    std::atomic<uint32_t> noteActivityCounter { 0 };
    std::atomic<bool> clockAdvancing { false };
    juce::Random rtRandom;
    ModLfo modLfo;
    ModSampleHold modSampleHold;
    PianoSynth pianoSynth;
    MidiActivityLog midiActivityLog;

    // Helper methods
    void processGenerativeOutput(juce::MidiBuffer& midiMessages, int numSamples);
    void onSubdivisionHit(int subdivision);
    bool continuousExpressionActive() const;
    void emitContinuousExpression(int64_t sampleTime);
    void scheduleExpressionSweep(int64_t noteOn, int64_t noteOff);
    void scheduleRoleParts(int step, int melodyChannel, int samplesPerStep,
                           float velocityMin, float velocityMax);
    ModulationRouter::Frame readModFrame(float lfo, float sampleHold) const;

    // Held-note CC / pitch bend. Fixed members only — no heap on the audio thread.
    int64_t expressionHoldUntil = 0;
    int currentBlockSamples = 0;
    int lastHeldCcNumber = -1;
    int lastHeldCcValue = -1;
    int lastHeldPitchBend = -1;

    // One held melody note per MIDI channel. Mono steals on the melody channel only.
    struct MelodyVoice
    {
        int pitch = -1;
        int64_t offSample = 0;
        bool active = false;
    };
    MelodyVoice melodyVoices[16] {};
    HarmonyParts::Triad currentTriad {};
    bool harmonyFromMelody = false;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenerativeMIDIProcessor)
};
