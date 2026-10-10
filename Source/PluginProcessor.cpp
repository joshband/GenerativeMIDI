/*
  ==============================================================================
    PluginProcessor.cpp

    Main plugin processor implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Core/GeneratorTypeMapping.h"
#include "DSP/NoteSchedulerHelpers.h"

namespace
{
int quantizeCc(float value)
{
    return static_cast<int>(juce::jlimit(0.0f, 1.0f, value) * 127.0f);
}

int quantizePitchWheel(float bendMinus1To1)
{
    // 0 maps to 8192 (wheel centre); must match EventScheduler::schedulePitchBend.
    const int bendValue = juce::roundToInt((juce::jlimit(-1.0f, 1.0f, bendMinus1To1) + 1.0f) * 8192.0f);
    return juce::jlimit(0, 16383, bendValue);
}

// The pitch bend control is in semitones; the wheel's full +/-1 span is taken as 24 semitones
// (the control's maximum). Result is the signed -1..1 value schedulePitchBend expects.
constexpr float kBendFullScaleSemitones = 24.0f;

float bendFromSemitones(float semitones)
{
    return juce::jlimit(-1.0f, 1.0f, semitones / kBendFullScaleSemitones);
}
}

//==============================================================================
GenerativeMIDIProcessor::GenerativeMIDIProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    :
#endif
    parameters(*this, nullptr, juce::Identifier("GenerativeMIDI"), createParameterLayout()),
    presetManager(parameters, polyrhythmEngine)
{
    if (wrapperType == wrapperType_Standalone)
        addBus(false);

    // Cache the audio-thread parameters used every block (no string lookups in processBlock).
    tempoParam = parameters.getRawParameterValue(PARAM_TEMPO);
    syncToHostParam = parameters.getRawParameterValue(PARAM_SYNC_TO_HOST);
    const auto tempoRange = parameters.getParameterRange(PARAM_TEMPO);
    tempoMin = tempoRange.start;
    tempoMax = tempoRange.end;

    // Setup clock manager callback
    clockManager.onSubdivisionHit = [this](int subdivision) {
        onSubdivisionHit(subdivision, 0);
    };
    clockManager.onSubdivisionHitAt = [this](int subdivision, int sampleOffset) {
        onSubdivisionHit(subdivision, sampleOffset);
    };
}

GenerativeMIDIProcessor::~GenerativeMIDIProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GenerativeMIDIProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_TEMPO, "Tempo", 20.0f, 400.0f, 120.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_TIME_SIG_NUM, "Time Signature Numerator", 1, 16, 4));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_TIME_SIG_DENOM, "Time Signature Denominator", 1, 16, 4));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_EUCLIDEAN_STEPS, "Euclidean Steps", 1, 64, 16));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_EUCLIDEAN_PULSES, "Euclidean Pulses", 0, 64, 4));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_EUCLIDEAN_ROTATION, "Euclidean Rotation", 0, 64, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_GENERATOR_TYPE, "Generator Type",
        juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic",
                         "Brownian", "Perlin Noise", "Drunk Walk", "Lorenz"},
        0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_NOTE_DENSITY, "Note Density", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_VELOCITY_MIN, "Velocity Min", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_VELOCITY_MAX, "Velocity Max", 0.0f, 1.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_PITCH_MIN, "Pitch Min", 0, 127, 48));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_PITCH_MAX, "Pitch Max", 0, 127, 84));

    // Scale quantization parameters
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_SCALE_ROOT, "Scale Root", 0, 11, 0)); // 0=C, 1=C#, etc.

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_SCALE_TYPE, "Scale Type",
        juce::StringArray{"Chromatic", "Major", "Minor", "Harmonic Minor", "Melodic Minor",
                          "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian",
                          "Major Pentatonic", "Minor Pentatonic", "Blues", "Whole Tone",
                          "Diminished", "Harmonic Major"},
        0));

    // Swing and humanization parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_SWING_AMOUNT, "Swing Amount", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_TIMING_HUMANIZE, "Timing Humanize", 0.0f, 50.0f, 0.0f)); // milliseconds

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_VELOCITY_HUMANIZE, "Velocity Humanize", 0.0f, 1.0f, 0.0f));

    // Gate length parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_GATE_LENGTH, "Gate Length", 0.01f, 2.0f, 0.8f)); // 1-200%, default 80%

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_LEGATO_MODE, "Legato Mode", false));

    // Ratcheting parameters
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_RATCHET_COUNT, "Ratchet Count", 1, 16, 1)); // 1-16 repeats, default 1 (off)

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_RATCHET_PROBABILITY, "Ratchet Probability", 0.0f, 1.0f, 0.0f)); // 0-100%, default 0%

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_RATCHET_DECAY, "Ratchet Decay", 0.0f, 1.0f, 0.5f)); // 0-100%, default 50%

    // Legacy stochastic subtype: kept in the layout so older sessions that stored
    // stochasticType still load. PARAM_GENERATOR_TYPE is the DSP source of truth;
    // this param is non-automatable and ignored in processBlock.
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_STOCHASTIC_TYPE, "Stochastic Type (legacy)",
        juce::StringArray{"Brownian", "Perlin", "Drunk Walk", "Lorenz"},
        0,
        juce::AudioParameterChoiceAttributes().withAutomatable(false)));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_STEP_SIZE, "Step Size", 0.01f, 1.0f, 0.1f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOMENTUM, "Momentum", 0.0f, 1.0f, 0.9f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_TIME_SCALE, "Time Scale", 0.01f, 10.0f, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_MARKOV_ORDER, "Markov Order", 1, 4, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_MARKOV_STEP, "Markov Step", 1, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MARKOV_SURPRISE, "Markov Surprise", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_LSYSTEM_GRAMMAR, "L-System Grammar", 0, LSystemCatalog::kCount - 1, 0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_LSYSTEM_GENERATION, "L-System Generation", 0, 6, 4));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_LSYSTEM_INTERVAL, "L-System Interval", 1, 12, 2));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_CELLULAR_RULE, "Cellular Rule", 0, 255, 30));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_CELLULAR_SEED, "Cellular Seed", 0, 31, 16));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_CELLULAR_LISTEN, "Cellular Listen", 0, 31, 16));

    // MIDI Routing
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_MIDI_CHANNEL, "MIDI Channel", 1, 16, 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_VOICE_MODE, "Voice",
        juce::StringArray { "Poly", "Mono" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_PART_COUNT, "Parts",
        juce::StringArray { "1", "2", "3", "4" }, 0));

    // MIDI Expression
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_AFTERTOUCH_ENABLE, "Aftertouch Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_AFTERTOUCH_AMOUNT, "Aftertouch Amount", 0.0f, 1.0f, 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_PITCHBEND_ENABLE, "Pitch Bend Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_PITCHBEND_RANGE, "Pitch Bend Range", 1.0f, 24.0f, 2.0f)); // semitones

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_CC_ENABLE, "CC Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_CC_NUMBER, "CC Number", 1, 127, 1)); // CC1 = Modulation Wheel

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_CC_AMOUNT, "CC Amount", 0.0f, 1.0f, 0.5f));

    // Modulation v2: LFO depths plus two free routes (see docs/developer/MODULATION_V2.md)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_MOD_LFO_ENABLE, "Mod LFO Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_RATE, "Mod LFO Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.4f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_DEPTH, "Mod LFO Depth", 0.0f, 1.0f, 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_DENSITY_DEPTH, "Mod LFO Density Depth", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_SH_RATE, "Mod S&H Rate",
        juce::NormalisableRange<float>(0.05f, 20.0f, 0.01f, 0.4f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_MOD_ROUTE3_SOURCE, "Mod Route 3 Source",
        juce::StringArray { "LFO", "S&H" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_MOD_ROUTE3_DEST, "Mod Route 3 Destination",
        juce::StringArray { "Off", "Gate", "Pitch", "CC", "Bend" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_ROUTE3_AMOUNT, "Mod Route 3 Amount", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_MOD_ROUTE4_SOURCE, "Mod Route 4 Source",
        juce::StringArray { "LFO", "S&H" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_MOD_ROUTE4_DEST, "Mod Route 4 Destination",
        juce::StringArray { "Off", "Gate", "Pitch", "CC", "Bend" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_ROUTE4_AMOUNT, "Mod Route 4 Amount", 0.0f, 1.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_PIANO_ENABLE, "Piano", false));

    // Appended last so sessions saved before this parameter existed load with the default (on).
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_SYNC_TO_HOST, "Sync to Host", true));

    return {params.begin(), params.end()};
}

//==============================================================================
const juce::String GenerativeMIDIProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GenerativeMIDIProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool GenerativeMIDIProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool GenerativeMIDIProcessor::isMidiEffect() const
{
    if (wrapperType == wrapperType_Standalone)
        return false;

#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double GenerativeMIDIProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GenerativeMIDIProcessor::getNumPrograms()
{
    return 1;
}

int GenerativeMIDIProcessor::getCurrentProgram()
{
    return 0;
}

void GenerativeMIDIProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String GenerativeMIDIProcessor::getProgramName(int /*index*/)
{
    return {};
}

void GenerativeMIDIProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void GenerativeMIDIProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    clockManager.setSampleRate(sampleRate);
    modLfo.reset();
    modSampleHold.reset();
    pianoSynth.reset();
    for (auto& voice : melodyVoices)
        voice = {};
    currentTriad = {};
    harmonyFromMelody = false;
    expressionHoldUntil = 0;
    lastHeldCcNumber = -1;
    lastHeldCcValue = -1;
    lastHeldPitchBend = -1;
    wasAdvancing = false;
    haveLastHostPpq = false;
    lastGeneratorType = -1;
    lastPitchbendEnabled = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;

    // Update parameters from value tree
    auto tempo = parameters.getRawParameterValue(PARAM_TEMPO)->load();
    clockManager.setTempo(tempo);

    auto timeSigNum = parameters.getRawParameterValue(PARAM_TIME_SIG_NUM)->load();
    auto timeSigDenom = parameters.getRawParameterValue(PARAM_TIME_SIG_DENOM)->load();
    clockManager.setTimeSignature(static_cast<int>(timeSigNum), static_cast<int>(timeSigDenom));

    // Initialize engines
    auto steps = parameters.getRawParameterValue(PARAM_EUCLIDEAN_STEPS)->load();
    auto pulses = parameters.getRawParameterValue(PARAM_EUCLIDEAN_PULSES)->load();
    euclideanEngine.setSteps(static_cast<int>(steps));
    euclideanEngine.setPulses(static_cast<int>(pulses));

    // Initialize algorithmic engine with parameter ranges
    auto pitchMin = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MIN)->load());
    auto pitchMax = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MAX)->load());
    auto velocityMin = parameters.getRawParameterValue(PARAM_VELOCITY_MIN)->load();
    auto velocityMax = parameters.getRawParameterValue(PARAM_VELOCITY_MAX)->load();
    algorithmicEngine.setPitchRange(pitchMin, pitchMax);
    algorithmicEngine.setVelocityRange(velocityMin, velocityMax);

    // Reserve event queue so schedule/process stay within capacity on the audio thread
    const int queueCap = juce::jmax(256, samplesPerBlock * 8);
    eventScheduler.prepare(queueCap);

    // Ensure polyrhythm engine follows clock tempo / meter
    polyrhythmEngine.setTempo(tempo);
    polyrhythmEngine.setTimeSignature(static_cast<int>(timeSigNum), static_cast<int>(timeSigDenom));

    // Ensure polyrhythm layer 0 has an audible default pattern (constructor starts empty).
    polyrhythmEngine.ensureLayerAudible(0);

    clockManager.start();
}

void GenerativeMIDIProcessor::releaseResources()
{
    clockManager.stop();
    // There is no MIDI buffer here, so the all-notes-off goes out with the next block.
    eventScheduler.clearAll();
    flushRequested.store(true, std::memory_order_relaxed);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GenerativeMIDIProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

bool GenerativeMIDIProcessor::canAddBus(bool isInput) const
{
    return !isInput
        && wrapperType == wrapperType_Standalone
        && getBusCount(false) == 0;
}

bool GenerativeMIDIProcessor::canApplyBusCountChange(bool isInput, bool isAddingBuses, BusProperties& outNewBusProperties)
{
    if (isInput || !isAddingBuses || wrapperType != wrapperType_Standalone)
        return false;

    outNewBusProperties.busName = "Output";
    outNewBusProperties.defaultLayout = juce::AudioChannelSet::stereo();
    outNewBusProperties.isActivatedByDefault = true;
    return true;
}

void GenerativeMIDIProcessor::releaseAllVoices(juce::MidiBuffer& midiMessages)
{
    eventScheduler.allNotesOff(midiMessages, 0, &midiActivityLog);
    for (auto& voice : melodyVoices)
        voice = {};
    expressionHoldUntil = 0;
    lastHeldCcNumber = -1;
    lastHeldCcValue = -1;
    lastHeldPitchBend = -1;
}

void GenerativeMIDIProcessor::realignToHost(double startSixteenths)
{
    clockManager.restart(startSixteenths);
    polyrhythmEngine.requestRestart();
    lastSubdivisionStep = static_cast<int>(std::ceil(startSixteenths - 1.0e-6));
}

void GenerativeMIDIProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Read the host position once per block. Standalone ignores the host entirely.
    juce::Optional<juce::AudioPlayHead::PositionInfo> hostPosition;
    if (wrapperType != wrapperType_Standalone)
        if (auto* hostPlayHead = getPlayHead())
            hostPosition = hostPlayHead->getPosition();

    const int bufferChannels = buffer.getNumChannels();
    const int totalNumInputChannels = juce::jmin(getTotalNumInputChannels(), bufferChannels);
    const int totalNumOutputChannels = juce::jmin(getTotalNumOutputChannels(), bufferChannels);
    currentBlockSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update engines from parameters (live parameter updates)
    auto steps = static_cast<int>(parameters.getRawParameterValue(PARAM_EUCLIDEAN_STEPS)->load());
    auto pulses = static_cast<int>(parameters.getRawParameterValue(PARAM_EUCLIDEAN_PULSES)->load());
    auto rotation = static_cast<int>(parameters.getRawParameterValue(PARAM_EUCLIDEAN_ROTATION)->load());

    // Ensure pulses never exceeds steps
    pulses = juce::jmin(pulses, steps);

    if (euclideanEngine.getSteps() != steps)
        euclideanEngine.setSteps(steps);
    if (euclideanEngine.getPulses() != pulses)
        euclideanEngine.setPulses(pulses);
    if (euclideanEngine.getRotation() != rotation)
        euclideanEngine.setRotation(rotation);

    // Tempo: the Tempo parameter, or the host's BPM (clamped to the parameter's range) when
    // Sync to Host is on and the host reports one. Set before any restart so the grid uses it.
    double effectiveTempo = static_cast<double>(tempoParam->load());
    juce::Optional<double> hostBpm;
    if (hostPosition.hasValue())
        hostBpm = hostPosition->getBpm();
    if (hostBpm.hasValue() && *hostBpm > 0.0 && syncToHostParam->load() > 0.5f)
        effectiveTempo = juce::jlimit(static_cast<double>(tempoMin), static_cast<double>(tempoMax), *hostBpm);
    clockManager.setTempo(effectiveTempo);

    // Process MIDI clock messages for external sync
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        clockManager.processExternalMidiClock(message);
    }

    // Advance when host is playing. Standalone free-runs (ignore transport gate);
    // host smoke tests attach a FakePlayHead on a non-Standalone processor.
    bool shouldAdvance = true;
    if (hostPosition.hasValue())
        shouldAdvance = hostPosition->getIsPlaying();
    clockAdvancing.store(shouldAdvance, std::memory_order_relaxed);

    // Panic when the host stops, the generator changes, or the session was torn down:
    // release every sounding note and re-centre the pitch wheel.
    const int generatorNow = static_cast<int>(parameters.getRawParameterValue(PARAM_GENERATOR_TYPE)->load());
    const bool bendEnabledNow = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;
    const bool panic = flushRequested.exchange(false, std::memory_order_relaxed)
                       || (wasAdvancing && !shouldAdvance)
                       || (lastGeneratorType >= 0 && generatorNow != lastGeneratorType);
    if (panic)
    {
        releaseAllVoices(midiMessages);
    }
    else if (lastPitchbendEnabled && !bendEnabledNow)
    {
        eventScheduler.centrePitchBend(midiMessages, 0, &midiActivityLog);
    }
    if (shouldAdvance && !wasAdvancing)
    {
        // Play edge: restart the grid and the step counter so bar-aligned parts line up with
        // the host bar. With a host ppq the grid starts at that song position; without one
        // (standalone free-run) it starts at the top.
        double startSixteenths = 0.0;
        if (hostPosition.hasValue())
            if (auto ppq = hostPosition->getPpqPosition())
                startSixteenths = juce::jmax(0.0, *ppq * 4.0);

        realignToHost(startSixteenths);
    }

    // Loop / jump detection. While the transport keeps playing, the host ppq should advance by the
    // previous block's duration at the host's tempo; anything further than a quarter of a 16th off
    // means the host looped or relocated. A tempo change alone is not a jump (the prediction uses
    // the previous block's tempo). Blocks that were already a play edge or a panic are not re-handled.
    juce::Optional<double> hostPpq;
    if (shouldAdvance && hostPosition.hasValue())
        hostPpq = hostPosition->getPpqPosition();

    const double jumpSampleRate = clockManager.getSampleRate();
    if (shouldAdvance && wasAdvancing && !panic && hostPpq.hasValue() && haveLastHostPpq && jumpSampleRate > 0.0)
    {
        const double predicted = lastHostPpq
                                 + (static_cast<double>(lastBlockSamples) / jumpSampleRate) * (lastBlockTempo / 60.0);
        if (std::abs(*hostPpq - predicted) > 1.0 / 16.0)
        {
            releaseAllVoices(midiMessages);
            realignToHost(juce::jmax(0.0, *hostPpq * 4.0));
        }
    }

    if (hostPpq.hasValue())
    {
        lastHostPpq = *hostPpq;
        // The ppq timeline runs at the host's tempo, even when Sync to Host is off.
        lastBlockTempo = hostBpm.hasValue() && *hostBpm > 0.0 ? *hostBpm : effectiveTempo;
        lastBlockSamples = buffer.getNumSamples();
        haveLastHostPpq = true;
    }
    else
    {
        haveLastHostPpq = false;
    }
    wasAdvancing = shouldAdvance;
    lastGeneratorType = generatorNow;
    lastPitchbendEnabled = bendEnabledNow;

    if (shouldAdvance)
    {
        clockManager.advance(buffer.getNumSamples());

        // Modulation v2: advance LFO in wall-clock time (RT-safe)
        const double sr = clockManager.getSampleRate();
        if (sr > 0.0)
        {
            const float rate = parameters.getRawParameterValue(PARAM_MOD_LFO_RATE)->load();
            modLfo.setRateHz(rate);
            modLfo.advance(static_cast<double>(buffer.getNumSamples()) / sr);
            modSampleHold.setRateHz(parameters.getRawParameterValue(PARAM_MOD_SH_RATE)->load());
            modSampleHold.advance(static_cast<double>(buffer.getNumSamples()) / sr);
        }

        // Notes that started on an earlier block are still held: one CC / pitch-bend
        // update at this block, using the LFO position at the block start.
        if (expressionHoldUntil > currentSamplePosition)
            emitContinuousExpression(currentSamplePosition);
    }

    // Generate MIDI events
    processGenerativeOutput(midiMessages, buffer.getNumSamples());

    // Process scheduled events (feeds MIDI activity log for the editor pane)
    eventScheduler.processEvents(currentSamplePosition, midiMessages, buffer.getNumSamples(),
                                 &midiActivityLog);

    if (bufferChannels > 0 && parameters.getRawParameterValue(PARAM_PIANO_ENABLE)->load() > 0.5f)
        pianoSynth.render(buffer, midiMessages, clockManager.getSampleRate());
    else
        pianoSynth.reset();

    currentSamplePosition += buffer.getNumSamples();
}

void GenerativeMIDIProcessor::processGenerativeOutput(juce::MidiBuffer& /*midiMessages*/, int /*numSamples*/)
{
    // This method is called to generate MIDI events based on current settings
    // Events are generated in onSubdivisionHit callback
}

ModulationRouter::Frame GenerativeMIDIProcessor::readModFrame(float lfo, float sampleHold) const
{
    const auto slot = [this](const char* sourceId, const char* destId, const char* amountId)
    {
        ModulationRouter::Slot route;
        route.source = juce::roundToInt(parameters.getRawParameterValue(sourceId)->load());
        route.dest = juce::roundToInt(parameters.getRawParameterValue(destId)->load());
        route.amount = parameters.getRawParameterValue(amountId)->load();
        return route;
    };

    ModulationRouter::Frame frame;
    frame.lfoEnabled = parameters.getRawParameterValue(PARAM_MOD_LFO_ENABLE)->load() > 0.5f;
    frame.lfo = lfo;
    frame.sampleHold = sampleHold;
    frame.velocityAmount = parameters.getRawParameterValue(PARAM_MOD_LFO_DEPTH)->load();
    frame.densityAmount = parameters.getRawParameterValue(PARAM_MOD_LFO_DENSITY_DEPTH)->load();
    frame.extras[0] = slot(PARAM_MOD_ROUTE3_SOURCE, PARAM_MOD_ROUTE3_DEST, PARAM_MOD_ROUTE3_AMOUNT);
    frame.extras[1] = slot(PARAM_MOD_ROUTE4_SOURCE, PARAM_MOD_ROUTE4_DEST, PARAM_MOD_ROUTE4_AMOUNT);
    return frame;
}

bool GenerativeMIDIProcessor::continuousExpressionActive() const
{
    const auto mix = ModulationRouter::evaluate(readModFrame(modLfo.getBipolar(), modSampleHold.getBipolar()));
    const bool lfoEnabled = parameters.getRawParameterValue(PARAM_MOD_LFO_ENABLE)->load() > 0.5f;
    const float depth = parameters.getRawParameterValue(PARAM_MOD_LFO_DEPTH)->load();
    const bool legacy = lfoEnabled && depth > 0.0f;
    if (!legacy && !mix.ccRouted && !mix.bendRouted)
        return false;

    const bool ccEnabled = parameters.getRawParameterValue(PARAM_CC_ENABLE)->load() > 0.5f;
    const bool pitchbendEnabled = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;
    return ccEnabled || pitchbendEnabled;
}

void GenerativeMIDIProcessor::emitContinuousExpression(int64_t sampleTime)
{
    if (!continuousExpressionActive())
    {
        lastHeldCcNumber = -1;
        lastHeldCcValue = -1;
        lastHeldPitchBend = -1;
        return;
    }

    const double sampleRate = clockManager.getSampleRate();
    if (sampleRate <= 0.0 || currentBlockSamples <= 0)
        return;

    const int64_t phaseSample = currentSamplePosition + static_cast<int64_t>(currentBlockSamples);
    const double deltaSeconds = static_cast<double>(sampleTime - phaseSample) / sampleRate;
    const float lfo = modLfo.peekBipolar(deltaSeconds);
    const auto mix = ModulationRouter::evaluate(readModFrame(lfo, modSampleHold.peekBipolar(deltaSeconds)));
    const bool lfoEnabled = parameters.getRawParameterValue(PARAM_MOD_LFO_ENABLE)->load() > 0.5f;
    const float depth = parameters.getRawParameterValue(PARAM_MOD_LFO_DEPTH)->load();
    const int channel = static_cast<int>(parameters.getRawParameterValue(PARAM_MIDI_CHANNEL)->load());

    if (parameters.getRawParameterValue(PARAM_CC_ENABLE)->load() > 0.5f)
    {
        const int ccNumber = static_cast<int>(parameters.getRawParameterValue(PARAM_CC_NUMBER)->load());
        const float ccAmount = parameters.getRawParameterValue(PARAM_CC_AMOUNT)->load();
        const float delta = mix.ccRouted ? mix.ccDelta : (lfoEnabled ? lfo * depth : 0.0f);
        const float value = ModulationRouter::applyAdditive(ccAmount, delta, 0.0f, 1.0f);
        const int midiValue = quantizeCc(value);
        if (midiValue != lastHeldCcValue || ccNumber != lastHeldCcNumber)
        {
            eventScheduler.scheduleCC(ccNumber, value, channel, sampleTime);
            lastHeldCcValue = midiValue;
            lastHeldCcNumber = ccNumber;
        }
    }

    if (parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f)
    {
        const float range = parameters.getRawParameterValue(PARAM_PITCHBEND_RANGE)->load();
        float bendNorm = bendFromSemitones(range);
        const float delta = mix.bendRouted ? mix.bendDelta : (lfoEnabled ? lfo * depth : 0.0f);
        bendNorm = ModulationRouter::applyAdditive(bendNorm, delta, -1.0f, 1.0f); // both directions
        const int midiValue = quantizePitchWheel(bendNorm);
        if (midiValue != lastHeldPitchBend)
        {
            eventScheduler.schedulePitchBend(bendNorm, channel, sampleTime);
            lastHeldPitchBend = midiValue;
        }
    }
}

void GenerativeMIDIProcessor::scheduleExpressionSweep(int64_t noteOn, int64_t noteOff)
{
    if (noteOff <= noteOn || currentBlockSamples <= 0 || !continuousExpressionActive())
        return;

    const int64_t blockEnd = currentSamplePosition + static_cast<int64_t>(currentBlockSamples);
    int64_t sample = noteOn + 1;
    if (sample < currentSamplePosition)
        sample = currentSamplePosition;

    const int64_t end = noteOff < blockEnd ? noteOff : blockEnd;
    int emitted = 0;
    while (sample < end && emitted < 8)
    {
        emitContinuousExpression(sample);
        sample += 128;
        ++emitted;
    }
}

void GenerativeMIDIProcessor::onSubdivisionHit(int subdivision, int sampleOffset)
{
    juce::ignoreUnused(subdivision);

    // Where this step falls on the timeline: block start plus its offset inside the block.
    const int64_t stepSample = currentSamplePosition + static_cast<int64_t>(sampleOffset);

    // Update scale quantizer from parameters
    auto scaleRoot = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_ROOT)->load());
    auto scaleType = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_TYPE)->load());
    scaleQuantizer.setRootNote(scaleRoot);
    scaleQuantizer.setScale(static_cast<ScaleQuantizer::Scale>(scaleType));

    const bool chromaticScale = scaleType == static_cast<int>(ScaleQuantizer::Scale::Chromatic);
    int scaleIntervals[12];
    int scaleIntervalCount = 0;
    if (!chromaticScale)
    {
        const auto& intervals = scaleQuantizer.getScaleIntervals();
        scaleIntervalCount = juce::jmin(12, static_cast<int>(intervals.size()));
        for (int i = 0; i < scaleIntervalCount; ++i)
            scaleIntervals[i] = intervals[static_cast<size_t>(i)];
    }

    // Update swing engine from parameters
    auto swingAmount = parameters.getRawParameterValue(PARAM_SWING_AMOUNT)->load();
    auto timingHumanize = parameters.getRawParameterValue(PARAM_TIMING_HUMANIZE)->load();
    auto velocityHumanize = parameters.getRawParameterValue(PARAM_VELOCITY_HUMANIZE)->load();
    swingEngine.setSwingAmount(swingAmount);
    swingEngine.setTimingRandomness(timingHumanize);
    swingEngine.setVelocityRandomness(velocityHumanize);

    const auto modMix = ModulationRouter::evaluate(
        readModFrame(modLfo.getBipolar(), modSampleHold.getBipolar()));

    // Update gate length controller from parameters
    auto gateLength = parameters.getRawParameterValue(PARAM_GATE_LENGTH)->load();
    gateLength = ModulationRouter::applyAdditive(gateLength, modMix.gateDelta, 0.01f, 2.0f);
    auto legatoMode = parameters.getRawParameterValue(PARAM_LEGATO_MODE)->load() > 0.5f;
    gateLengthController.setGateLength(gateLength);
    gateLengthController.setLegatoMode(legatoMode);

    // Update ratchet engine from parameters
    auto ratchetCount = static_cast<int>(parameters.getRawParameterValue(PARAM_RATCHET_COUNT)->load());
    auto ratchetProbability = parameters.getRawParameterValue(PARAM_RATCHET_PROBABILITY)->load();
    auto ratchetDecay = parameters.getRawParameterValue(PARAM_RATCHET_DECAY)->load();
    ratchetEngine.setRatchetCount(ratchetCount);
    ratchetEngine.setRatchetProbability(ratchetProbability);
    ratchetEngine.setVelocityDecay(ratchetDecay);

    // Indices: GeneratorTypeMapping (0 Euclidean, 1 Polyrhythm, 2–5 algo, 6–9 stochastic)
    const int generatorType = static_cast<int>(parameters.getRawParameterValue(PARAM_GENERATOR_TYPE)->load());

    const float velocityMin = parameters.getRawParameterValue(PARAM_VELOCITY_MIN)->load();
    const float velocityMax = parameters.getRawParameterValue(PARAM_VELOCITY_MAX)->load();
    // The two range parameters are independent, so either one can be the larger.
    // Order them once here: later code takes jlimit(pitchMin, pitchMax, ...) and
    // `step % (pitchMax - pitchMin + 1)`, which need min <= max.
    const int pitchParamA = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MIN)->load());
    const int pitchParamB = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MAX)->load());
    const int pitchMin = juce::jmin(pitchParamA, pitchParamB);
    const int pitchMax = juce::jmax(pitchParamA, pitchParamB);
    const int midiChannel = static_cast<int>(parameters.getRawParameterValue(PARAM_MIDI_CHANNEL)->load());
    const bool monoVoice = juce::roundToInt(parameters.getRawParameterValue(PARAM_VOICE_MODE)->load()) == 1;
    const int harmonyStep = lastSubdivisionStep;
    const float density = parameters.getRawParameterValue(PARAM_NOTE_DENSITY)->load();
    const int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));

    const bool aftertouchEnable = parameters.getRawParameterValue(PARAM_AFTERTOUCH_ENABLE)->load() > 0.5f;
    const float aftertouchAmount = parameters.getRawParameterValue(PARAM_AFTERTOUCH_AMOUNT)->load();
    const bool pitchbendEnable = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;
    const float pitchbendRange = parameters.getRawParameterValue(PARAM_PITCHBEND_RANGE)->load();
    const bool ccEnable = parameters.getRawParameterValue(PARAM_CC_ENABLE)->load() > 0.5f;
    const int ccNumber = static_cast<int>(parameters.getRawParameterValue(PARAM_CC_NUMBER)->load());
    const float ccAmount = parameters.getRawParameterValue(PARAM_CC_AMOUNT)->load();

    const float effectiveDensity = ModulationRouter::applyAdditive(density, modMix.densityDelta, 0.0f, 1.0f);

    auto modulatePitch = [&](int rawPitch)
    {
        const int shifted = juce::jlimit(0, 127, rawPitch + juce::roundToInt(modMix.pitchSemitones));
        return juce::jlimit(0, 127, scaleQuantizer.quantize(shifted));
    };

    auto scheduleNote = [&](int pitch, float velocity, int stepForSwing)
    {
        velocity = swingEngine.humanizeVelocity(velocity);
        velocity = ModulationRouter::applyAdditive(velocity, modMix.velocityDelta, 0.0f, 1.0f);

        int timingOffset = swingEngine.calculateTotalTimingOffset(
            stepForSwing, samplesPerStep, clockManager.getSampleRate());
        const bool useRatcheting = ratchetEngine.shouldRatchet();

        if (monoVoice)
        {
            const int slotIndex = juce::jlimit(0, 15, midiChannel - 1);
            auto& slot = melodyVoices[slotIndex];
            int64_t start = stepSample + static_cast<int64_t>(timingOffset);
            if (slot.active && slot.offSample > start)
            {
                int64_t offAt = start - 1;
                if (offAt < currentSamplePosition)
                {
                    // The previous sample already played. Park the note-off on the
                    // first sample of this block and let the new note follow it.
                    offAt = currentSamplePosition;
                    ++timingOffset;
                }
                // Drop the stolen note's own later events so they cannot cut a retriggered pitch.
                eventScheduler.cancelNoteEventsAfter(slot.pitch, midiChannel, offAt);
                eventScheduler.scheduleNoteOff(slot.pitch, midiChannel, offAt);
            }
        }

        const int64_t noteOnSample = stepSample + static_cast<int64_t>(timingOffset);

        const int64_t noteOffSample = NoteSchedulerHelpers::scheduleGeneratedNote(
            eventScheduler, ratchetEngine, gateLengthController,
            pitch, velocity, midiChannel, stepSample,
            timingOffset, samplesPerStep, useRatcheting);

        if (monoVoice)
        {
            const int slotIndex = juce::jlimit(0, 15, midiChannel - 1);
            auto& slot = melodyVoices[slotIndex];
            slot.pitch = pitch;
            slot.active = true;
            slot.offSample = noteOffSample;
        }

        currentTriad = HarmonyParts::triadForMelody(
            pitch, scaleRoot, scaleIntervals, scaleIntervalCount, chromaticScale);
        harmonyFromMelody = true;

        if (noteOffSample > expressionHoldUntil)
            expressionHoldUntil = noteOffSample;

        // Note-on expression stays a single static emit. Held-note updates below
        // move CC amount and pitch-bend range with the velocity LFO.
        if (aftertouchEnable)
            eventScheduler.scheduleAftertouch(pitch, aftertouchAmount, midiChannel, noteOnSample);

        if (ccEnable)
        {
            eventScheduler.scheduleCC(ccNumber, ccAmount, midiChannel, noteOnSample);
            lastHeldCcNumber = ccNumber;
            lastHeldCcValue = quantizeCc(ccAmount);
        }

        if (pitchbendEnable)
        {
            // Map PB semitones (1–24) onto the signed -1..1 wheel span
            const float bendNorm = bendFromSemitones(pitchbendRange);
            eventScheduler.schedulePitchBend(bendNorm, midiChannel, noteOnSample);
            lastHeldPitchBend = quantizePitchWheel(bendNorm);
        }

        scheduleExpressionSweep(noteOnSample, noteOffSample);

        noteActivityCounter.fetch_add(1, std::memory_order_relaxed);
    };

    switch (generatorType)
    {
        case GeneratorTypeMapping::kEuclidean:
        {
            const int step = lastSubdivisionStep % euclideanEngine.getSteps();
            if (euclideanEngine.getStep(step)
                && rtRandom.nextFloat() < effectiveDensity)
            {
                const float rawVelocity = euclideanEngine.getVelocity(step);
                const float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));
                const int pitchRange = pitchMax - pitchMin;
                const int rawPitch = pitchMin + (step % (pitchRange + 1));
                const int pitch = modulatePitch(rawPitch);
                scheduleNote(pitch, velocity, step);
            }
            lastSubdivisionStep++;
            break;
        }

        case GeneratorTypeMapping::kPolyrhythm:
        {
            // Sixteenth-note clock grid; layer.division scales step rate (4 = quarters).
            constexpr int kClockGrid = 16;
            polyrhythmEngine.processTick(kClockGrid, [&](const PolyrhythmLayer& layer, int step)
            {
                if (step >= 0 && step < static_cast<int>(layer.pattern.size())
                    && step < static_cast<int>(layer.pitches.size())
                    && step < static_cast<int>(layer.velocities.size())
                    && layer.pattern[static_cast<size_t>(step)]
                    && rtRandom.nextFloat() < effectiveDensity)
                {
                    const int rawPitch = juce::jlimit(
                        pitchMin, pitchMax,
                        layer.pitches[static_cast<size_t>(step)] + layer.pitchOffset);
                    const int pitch = modulatePitch(rawPitch);
                    const float rawVelocity = juce::jlimit(
                        0.0f, 1.0f,
                        layer.velocities[static_cast<size_t>(step)] * layer.velocityMultiplier);
                    const float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));
                    scheduleNote(pitch, velocity, step);
                }
            });
            lastSubdivisionStep++;
            break;
        }

        case GeneratorTypeMapping::kMarkov:
        case GeneratorTypeMapping::kLSystem:
        case GeneratorTypeMapping::kCellular:
        case GeneratorTypeMapping::kProbabilistic:
        {
            const auto algoType = GeneratorTypeMapping::toAlgorithmic(generatorType);
            algorithmicEngine.setGeneratorType(algoType);
            algorithmicEngine.setPitchRange(pitchMin, pitchMax);
            algorithmicEngine.setVelocityRange(velocityMin, velocityMax);

            if (generatorType == GeneratorTypeMapping::kMarkov)
            {
                const int order = static_cast<int>(parameters.getRawParameterValue(PARAM_MARKOV_ORDER)->load());
                const int step = static_cast<int>(parameters.getRawParameterValue(PARAM_MARKOV_STEP)->load());
                const float surprise = parameters.getRawParameterValue(PARAM_MARKOV_SURPRISE)->load();
                algorithmicEngine.setMarkovControls(order, step, surprise);
            }
            else if (generatorType == GeneratorTypeMapping::kLSystem)
            {
                const int grammar = static_cast<int>(parameters.getRawParameterValue(PARAM_LSYSTEM_GRAMMAR)->load());
                const int generation = static_cast<int>(parameters.getRawParameterValue(PARAM_LSYSTEM_GENERATION)->load());
                const int interval = static_cast<int>(parameters.getRawParameterValue(PARAM_LSYSTEM_INTERVAL)->load());
                algorithmicEngine.setLSystemControls(grammar, generation, interval);
            }
            else if (generatorType == GeneratorTypeMapping::kCellular)
            {
                const int rule = static_cast<int>(parameters.getRawParameterValue(PARAM_CELLULAR_RULE)->load());
                const int seed = static_cast<int>(parameters.getRawParameterValue(PARAM_CELLULAR_SEED)->load());
                const int listen = static_cast<int>(parameters.getRawParameterValue(PARAM_CELLULAR_LISTEN)->load());
                algorithmicEngine.setCellularControls(rule, seed, listen);
            }

            // Markov, L-System, and Cellular advance every tick. Density only gates the note.
            const bool gated = generatorType == GeneratorTypeMapping::kProbabilistic
                                   ? rtRandom.nextFloat() < effectiveDensity
                                   : true;
            if (gated)
            {
                const int rawNote = algorithmicEngine.generateNextNote();
                const bool emit = generatorType == GeneratorTypeMapping::kProbabilistic
                                      || rtRandom.nextFloat() < effectiveDensity;
                if (emit && rawNote >= 0)
                {
                    const int rawPitch = juce::jlimit(pitchMin, pitchMax, rawNote);
                    const int pitch = modulatePitch(rawPitch);
                    const float rawVelocity = algorithmicEngine.generateNextVelocity();
                    const float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));
                    scheduleNote(pitch, velocity, lastSubdivisionStep);
                }
            }
            lastSubdivisionStep++;
            break;
        }

        case GeneratorTypeMapping::kBrownian:
        case GeneratorTypeMapping::kPerlin:
        case GeneratorTypeMapping::kDrunkWalk:
        case GeneratorTypeMapping::kLorenz:
        {
            // Ignore PARAM_STOCHASTIC_TYPE: legacy APVTS slot for session load only.
            // DSP subtype comes solely from PARAM_GENERATOR_TYPE (indices 6–9).
            const auto type = GeneratorTypeMapping::toStochastic(generatorType);

            const float stepSize = parameters.getRawParameterValue(PARAM_STEP_SIZE)->load();
            const float momentum = parameters.getRawParameterValue(PARAM_MOMENTUM)->load();
            const float timeScale = parameters.getRawParameterValue(PARAM_TIME_SCALE)->load();

            stochasticEngine.setGeneratorType(type);
            stochasticEngine.setDensity(effectiveDensity);
            stochasticEngine.setStepSize(stepSize);
            stochasticEngine.setMomentum(momentum);
            stochasticEngine.setTimeScale(timeScale);

            const double secondsPerSubdivision = clockManager.getSamplesPerSubdivision(16) / getSampleRate();
            stochasticEngine.advance(static_cast<float>(secondsPerSubdivision));

            if (stochasticEngine.shouldTriggerNote())
            {
                const int pitch = modulatePitch(stochasticEngine.getCurrentPitch(pitchMin, pitchMax));
                float velocity = stochasticEngine.getCurrentVelocity(velocityMin, velocityMax);
                scheduleNote(pitch, velocity, lastSubdivisionStep);
            }
            lastSubdivisionStep++;
            break;
        }

        default:
            lastSubdivisionStep++;
            break;
    }

    scheduleRoleParts(stepSample, harmonyStep, midiChannel, samplesPerStep, velocityMin, velocityMax);
}

void GenerativeMIDIProcessor::scheduleRoleParts(int64_t stepSample, int step, int melodyChannel, int samplesPerStep,
                                                float velocityMin, float velocityMax)
{
    const int partCount = juce::jlimit(
        1, 4, juce::roundToInt(parameters.getRawParameterValue(PARAM_PART_COUNT)->load()) + 1);
    if (partCount < 2 || samplesPerStep <= 0)
        return;

    const int scaleRoot = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_ROOT)->load());
    const int scaleType = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_TYPE)->load());
    const bool chromatic = scaleType == static_cast<int>(ScaleQuantizer::Scale::Chromatic);

    int intervals[12];
    int intervalCount = 0;
    if (!chromatic)
    {
        const auto& src = scaleQuantizer.getScaleIntervals();
        intervalCount = juce::jmin(12, static_cast<int>(src.size()));
        for (int i = 0; i < intervalCount; ++i)
            intervals[i] = src[static_cast<size_t>(i)];
    }

    if (!harmonyFromMelody)
    {
        currentTriad = HarmonyParts::triadForMelody(
            60 + scaleRoot, scaleRoot, intervals, intervalCount, chromatic);
    }

    const int timeNum = juce::jmax(1, static_cast<int>(parameters.getRawParameterValue(PARAM_TIME_SIG_NUM)->load()));
    const int timeDen = juce::jmax(1, static_cast<int>(parameters.getRawParameterValue(PARAM_TIME_SIG_DENOM)->load()));
    const int sixteenthsPerBar = juce::jmax(1, timeNum * 16 / timeDen);

    const float span = velocityMax - velocityMin;
    const int timingOffset = swingEngine.calculateTotalTimingOffset(
        step, samplesPerStep, clockManager.getSampleRate());
    const int64_t noteOnSample = stepSample + static_cast<int64_t>(timingOffset);
    const int duration = juce::jmax(1, gateLengthController.calculateGateLengthSamples(samplesPerStep));

    auto play = [&](int pitch, float velocity, int channel)
    {
        eventScheduler.scheduleNote(pitch, velocity, channel, noteOnSample, duration);
        noteActivityCounter.fetch_add(1, std::memory_order_relaxed);
    };

    if (partCount >= 2 && (step % 4) == 0)
        play(currentTriad.root, velocityMin + span * 0.70f, HarmonyParts::roleChannel(melodyChannel, 1));

    if (partCount >= 3 && (step % sixteenthsPerBar) == 0)
    {
        const int chordChannel = HarmonyParts::roleChannel(melodyChannel, 2);
        const int tones[3] = { currentTriad.root, currentTriad.third, currentTriad.fifth };
        int played[3] = { -1, -1, -1 };
        int playedCount = 0;
        for (int tone : tones)
        {
            bool duplicate = false;
            for (int i = 0; i < playedCount; ++i)
                if (played[i] == tone)
                    duplicate = true;
            if (duplicate)
                continue;
            played[playedCount++] = tone;
            play(tone, velocityMin + span * 0.55f, chordChannel);
        }
    }

    if (partCount >= 4 && (step % 2) == 0)
        play(currentTriad.tone((step / 2) % 3), velocityMin + span * 0.45f,
             HarmonyParts::roleChannel(melodyChannel, 3));
}

//==============================================================================
bool GenerativeMIDIProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GenerativeMIDIProcessor::createEditor()
{
    return new GenerativeMIDIEditor(*this);
}

//==============================================================================
void GenerativeMIDIProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml(*xmlState);
        const auto schema = xmlState->getStringAttribute("generativeMidiSchema");
        GeneratorTypeMapping::migrateApvtsStateIfNeeded(tree, schema);

        // Strip non-PARAM child before APVTS replace; apply layers when present (schema 1.2+).
        auto layersNode = tree.getChildWithName(PolyrhythmEngine::kStateTreeType);
        juce::ValueTree layersCopy;
        if (layersNode.isValid())
        {
            layersCopy = layersNode.createCopy();
            tree.removeChild(layersNode, nullptr);
        }

        parameters.replaceState(tree);

        if (layersCopy.isValid())
            polyrhythmEngine.loadFromValueTree(layersCopy);
    }
}

void GenerativeMIDIProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();

    auto existing = state.getChildWithName(PolyrhythmEngine::kStateTreeType);
    if (existing.isValid())
        state.removeChild(existing, nullptr);
    state.appendChild(polyrhythmEngine.toValueTree(), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr)
        xml->setAttribute("generativeMidiSchema", GeneratorTypeMapping::kPresetSchemaVersion);
    copyXmlToBinary(*xml, destData);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GenerativeMIDIProcessor();
}
