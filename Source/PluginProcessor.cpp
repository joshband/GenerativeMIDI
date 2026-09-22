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
    // Setup clock manager callback
    clockManager.onSubdivisionHit = [this](int subdivision) {
        onSubdivisionHit(subdivision);
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

    // MIDI Routing
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        PARAM_MIDI_CHANNEL, "MIDI Channel", 1, 16, 1));

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

    // Modulation v2 MVP: one LFO → velocity (see docs/developer/MODULATION_V2.md)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        PARAM_MOD_LFO_ENABLE, "Mod LFO Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_RATE, "Mod LFO Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.4f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_DEPTH, "Mod LFO Depth", 0.0f, 1.0f, 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        PARAM_MOD_LFO_DENSITY_DEPTH, "Mod LFO Density Depth", 0.0f, 1.0f, 0.0f));

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

void GenerativeMIDIProcessor::setCurrentProgram(int index)
{
}

const juce::String GenerativeMIDIProcessor::getProgramName(int index)
{
    return {};
}

void GenerativeMIDIProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void GenerativeMIDIProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    clockManager.setSampleRate(sampleRate);
    modLfo.reset();

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
    if (auto* layer = polyrhythmEngine.getLayer(0))
    {
        bool anyActive = false;
        for (bool step : layer->pattern)
        {
            if (step)
            {
                anyActive = true;
                break;
            }
        }

        if (!anyActive)
        {
            for (int i = 0; i < layer->length; ++i)
            {
                layer->pattern[static_cast<size_t>(i)] = (i % 4 == 0);
                layer->velocities[static_cast<size_t>(i)] = 0.8f;
                layer->pitches[static_cast<size_t>(i)] = 60 + (i % 12);
            }
            layer->enabled = true;
        }
    }

    clockManager.start();
}

void GenerativeMIDIProcessor::releaseResources()
{
    clockManager.stop();
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

void GenerativeMIDIProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
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

    auto tempo = parameters.getRawParameterValue(PARAM_TEMPO)->load();
    clockManager.setTempo(tempo);

    // Process MIDI clock messages for external sync
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        clockManager.processExternalMidiClock(message);
    }

    // Advance clock when host is playing, or always when no playhead (Standalone)
    bool shouldAdvance = true;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
            shouldAdvance = position->getIsPlaying();
    }

    if (shouldAdvance)
    {
        clockManager.advance(buffer.getNumSamples());

        // Modulation v2: advance LFO in wall-clock time (RT-safe)
        const double sr = getSampleRate();
        if (sr > 0.0)
        {
            const float rate = parameters.getRawParameterValue(PARAM_MOD_LFO_RATE)->load();
            modLfo.setRateHz(rate);
            modLfo.advance(static_cast<double>(buffer.getNumSamples()) / sr);
        }
    }

    // Generate MIDI events
    processGenerativeOutput(midiMessages, buffer.getNumSamples());

    // Process scheduled events
    eventScheduler.processEvents(currentSamplePosition, midiMessages, buffer.getNumSamples());

    currentSamplePosition += buffer.getNumSamples();
}

void GenerativeMIDIProcessor::processGenerativeOutput(juce::MidiBuffer& midiMessages, int numSamples)
{
    // This method is called to generate MIDI events based on current settings
    // Events are generated in onSubdivisionHit callback
}

void GenerativeMIDIProcessor::onSubdivisionHit(int subdivision)
{
    juce::ignoreUnused(subdivision);

    // Update scale quantizer from parameters
    auto scaleRoot = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_ROOT)->load());
    auto scaleType = static_cast<int>(parameters.getRawParameterValue(PARAM_SCALE_TYPE)->load());
    scaleQuantizer.setRootNote(scaleRoot);
    scaleQuantizer.setScale(static_cast<ScaleQuantizer::Scale>(scaleType));

    // Update swing engine from parameters
    auto swingAmount = parameters.getRawParameterValue(PARAM_SWING_AMOUNT)->load();
    auto timingHumanize = parameters.getRawParameterValue(PARAM_TIMING_HUMANIZE)->load();
    auto velocityHumanize = parameters.getRawParameterValue(PARAM_VELOCITY_HUMANIZE)->load();
    swingEngine.setSwingAmount(swingAmount);
    swingEngine.setTimingRandomness(timingHumanize);
    swingEngine.setVelocityRandomness(velocityHumanize);

    // Update gate length controller from parameters
    auto gateLength = parameters.getRawParameterValue(PARAM_GATE_LENGTH)->load();
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
    const int pitchMin = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MIN)->load());
    const int pitchMax = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MAX)->load());
    const int midiChannel = static_cast<int>(parameters.getRawParameterValue(PARAM_MIDI_CHANNEL)->load());
    const float density = parameters.getRawParameterValue(PARAM_NOTE_DENSITY)->load();
    const int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));

    const bool aftertouchEnable = parameters.getRawParameterValue(PARAM_AFTERTOUCH_ENABLE)->load() > 0.5f;
    const float aftertouchAmount = parameters.getRawParameterValue(PARAM_AFTERTOUCH_AMOUNT)->load();
    const bool pitchbendEnable = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;
    const float pitchbendRange = parameters.getRawParameterValue(PARAM_PITCHBEND_RANGE)->load();
    const bool ccEnable = parameters.getRawParameterValue(PARAM_CC_ENABLE)->load() > 0.5f;
    const int ccNumber = static_cast<int>(parameters.getRawParameterValue(PARAM_CC_NUMBER)->load());
    const float ccAmount = parameters.getRawParameterValue(PARAM_CC_AMOUNT)->load();

    const bool modLfoEnable = parameters.getRawParameterValue(PARAM_MOD_LFO_ENABLE)->load() > 0.5f;
    const float modLfoDepth = parameters.getRawParameterValue(PARAM_MOD_LFO_DEPTH)->load();
    const float modLfoDensityDepth = parameters.getRawParameterValue(PARAM_MOD_LFO_DENSITY_DEPTH)->load();
    const float modLfoValue = modLfo.getBipolar();

    float effectiveDensity = density;
    if (modLfoEnable && modLfoDensityDepth > 0.0f)
        effectiveDensity = ModLfo::applyToUnipolar(density, modLfoValue, modLfoDensityDepth);

    auto scheduleNote = [&](int pitch, float velocity, int stepForSwing)
    {
        velocity = swingEngine.humanizeVelocity(velocity);
        if (modLfoEnable && modLfoDepth > 0.0f)
            velocity = ModLfo::applyToUnipolar(velocity, modLfoValue, modLfoDepth);

        const int timingOffset = swingEngine.calculateTotalTimingOffset(
            stepForSwing, samplesPerStep, getSampleRate());
        const bool useRatcheting = ratchetEngine.shouldRatchet();
        const int64_t noteOnSample = currentSamplePosition + timingOffset;

        NoteSchedulerHelpers::scheduleGeneratedNote(
            eventScheduler, ratchetEngine, gateLengthController,
            pitch, velocity, midiChannel, currentSamplePosition,
            timingOffset, samplesPerStep, useRatcheting);

        // Minimal MIDI expression emit (RT-safe via existing EventScheduler path)
        if (aftertouchEnable)
            eventScheduler.scheduleAftertouch(pitch, aftertouchAmount, midiChannel, noteOnSample);

        if (ccEnable)
            eventScheduler.scheduleCC(ccNumber, ccAmount, midiChannel, noteOnSample);

        if (pitchbendEnable)
        {
            // Map PB range (1–24 semitones) to a fraction of full MIDI bend wheel
            const float bendNorm = juce::jlimit(0.0f, 1.0f, pitchbendRange / 24.0f);
            eventScheduler.schedulePitchBend(bendNorm, midiChannel, noteOnSample);
        }
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
                const int pitch = scaleQuantizer.quantize(rawPitch);
                scheduleNote(pitch, velocity, step);
            }
            lastSubdivisionStep++;
            break;
        }

        case GeneratorTypeMapping::kPolyrhythm:
        {
            // Sixteenth-note clock grid; layer.division scales step rate (4 = quarters).
            constexpr int kClockGrid = 16;
            for (int i = 0; i < polyrhythmEngine.getNumLayers(); ++i)
            {
                auto* layer = polyrhythmEngine.getLayer(i);
                if (!layer || !layer->enabled || layer->length <= 0)
                    continue;

                if (!polyrhythmEngine.shouldEmitOnThisTick(i, kClockGrid))
                    continue;

                const int step = layer->currentStep % layer->length;
                if (step >= 0 && step < static_cast<int>(layer->pattern.size())
                    && step < static_cast<int>(layer->pitches.size())
                    && step < static_cast<int>(layer->velocities.size())
                    && layer->pattern[static_cast<size_t>(step)]
                    && rtRandom.nextFloat() < effectiveDensity)
                {
                    const int rawPitch = juce::jlimit(
                        pitchMin, pitchMax,
                        layer->pitches[static_cast<size_t>(step)] + layer->pitchOffset);
                    const int pitch = scaleQuantizer.quantize(rawPitch);
                    const float rawVelocity = juce::jlimit(
                        0.0f, 1.0f,
                        layer->velocities[static_cast<size_t>(step)] * layer->velocityMultiplier);
                    const float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));
                    scheduleNote(pitch, velocity, step);
                }

                polyrhythmEngine.advanceStep(i);
            }
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

            if (rtRandom.nextFloat() < effectiveDensity)
            {
                algorithmicEngine.setPitchRange(pitchMin, pitchMax);
                algorithmicEngine.setVelocityRange(velocityMin, velocityMax);

                const int rawNote = algorithmicEngine.generateNextNote();
                if (rawNote >= 0)
                {
                    const int rawPitch = juce::jlimit(pitchMin, pitchMax, rawNote);
                    const int pitch = scaleQuantizer.quantize(rawPitch);
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
                int pitch = stochasticEngine.getCurrentPitch(pitchMin, pitchMax);
                pitch = scaleQuantizer.quantize(pitch);
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
