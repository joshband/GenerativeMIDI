/*
  ==============================================================================
    PluginProcessor.cpp

    Main plugin processor implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    presetManager(parameters)
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

    // Stochastic/Chaos parameters
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        PARAM_STOCHASTIC_TYPE, "Stochastic Type",
        juce::StringArray{"Brownian", "Perlin", "Drunk Walk", "Lorenz"},
        0));

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

    // Initialize polyrhythm engine with a default pattern
    if (polyrhythmEngine.getNumLayers() == 0)
    {
        int layerIdx = polyrhythmEngine.addLayer();
        auto* layer = polyrhythmEngine.getLayer(layerIdx);
        if (layer)
        {
            // Create a simple pattern
            for (int i = 0; i < layer->length; ++i)
            {
                layer->pattern[i] = (i % 4 == 0); // Every 4 steps
                layer->velocities[i] = 0.8f;
                layer->pitches[i] = 60 + (i % 12); // C major scale-ish
            }
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
    euclideanEngine.setRotation(rotation);

    auto tempo = parameters.getRawParameterValue(PARAM_TEMPO)->load();
    clockManager.setTempo(tempo);

    // Process MIDI clock messages for external sync
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        clockManager.processExternalMidiClock(message);
    }

    // Advance clock
    clockManager.advance(buffer.getNumSamples());

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

    // Generate events based on selected generator type
    auto generatorType = parameters.getRawParameterValue(PARAM_GENERATOR_TYPE)->load();

    // Get velocity and pitch range parameters
    auto velocityMin = parameters.getRawParameterValue(PARAM_VELOCITY_MIN)->load();
    auto velocityMax = parameters.getRawParameterValue(PARAM_VELOCITY_MAX)->load();
    auto pitchMin = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MIN)->load());
    auto pitchMax = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MAX)->load());

    // Get MIDI channel parameter
    auto midiChannel = static_cast<int>(parameters.getRawParameterValue(PARAM_MIDI_CHANNEL)->load());

    // Get probability/density parameter - applies to ALL generators
    auto density = parameters.getRawParameterValue(PARAM_NOTE_DENSITY)->load();

    // Get MIDI expression parameters
    auto aftertouchEnable = parameters.getRawParameterValue(PARAM_AFTERTOUCH_ENABLE)->load() > 0.5f;
    auto aftertouchAmount = parameters.getRawParameterValue(PARAM_AFTERTOUCH_AMOUNT)->load();
    auto pitchbendEnable = parameters.getRawParameterValue(PARAM_PITCHBEND_ENABLE)->load() > 0.5f;
    auto pitchbendRange = parameters.getRawParameterValue(PARAM_PITCHBEND_RANGE)->load();
    auto ccEnable = parameters.getRawParameterValue(PARAM_CC_ENABLE)->load() > 0.5f;
    auto ccNumber = static_cast<int>(parameters.getRawParameterValue(PARAM_CC_NUMBER)->load());
    auto ccAmount = parameters.getRawParameterValue(PARAM_CC_AMOUNT)->load();

    switch (static_cast<int>(generatorType))
    {
        case 0: // Euclidean
        {
            int step = lastSubdivisionStep % euclideanEngine.getSteps();
            if (euclideanEngine.getStep(step))
            {
                // Apply probability check to Euclidean rhythm
                if (juce::Random::getSystemRandom().nextFloat() < density)
                {
                    // Get raw velocity from engine and map to user-defined range
                    float rawVelocity = euclideanEngine.getVelocity(step);
                    float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));

                    // Apply velocity humanization
                    velocity = swingEngine.humanizeVelocity(velocity);

                    // Map step to pitch range instead of just adding offset
                    int pitchRange = pitchMax - pitchMin;
                    int rawPitch = pitchMin + (step % (pitchRange + 1));

                    // Apply scale quantization
                    int pitch = scaleQuantizer.quantize(rawPitch);

                    // Calculate swing and humanization timing offset
                    int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));
                    int timingOffset = swingEngine.calculateTotalTimingOffset(
                        step, samplesPerStep, getSampleRate());

                    // Check if ratcheting should be applied
                    bool useRatcheting = ratchetEngine.shouldRatchet();
                    auto ratchetOffsets = useRatcheting ?
                        ratchetEngine.calculateRatchetOffsets(samplesPerStep) :
                        std::vector<int>{0};

                    // Schedule note(s) with optional ratcheting
                    for (size_t ratchetIdx = 0; ratchetIdx < ratchetOffsets.size(); ++ratchetIdx)
                    {
                        // Calculate velocity with ratchet decay
                        float ratchetVelocity = ratchetEngine.calculateRatchetVelocity(
                            velocity, static_cast<int>(ratchetIdx));

                        int ratchetTimingOffset = timingOffset + ratchetOffsets[ratchetIdx];

                        // Schedule note on
                        eventScheduler.scheduleNoteOn(pitch, ratchetVelocity, midiChannel,
                            currentSamplePosition + ratchetTimingOffset);

                        // Schedule note off using gate length controller
                        int noteDuration = gateLengthController.calculateGateLengthSamples(samplesPerStep);
                        eventScheduler.scheduleNoteOff(pitch, midiChannel,
                            currentSamplePosition + ratchetTimingOffset + noteDuration);
                    }
                }
                // else: probability miss - note scheduled but not played
            }
            lastSubdivisionStep++;
            break;
        }

        case 1: // Polyrhythm
        {
            for (int i = 0; i < polyrhythmEngine.getNumLayers(); ++i)
            {
                auto* layer = polyrhythmEngine.getLayer(i);
                if (!layer || !layer->enabled)
                    continue;

                // Check if this step should trigger
                if (layer->pattern[layer->currentStep])
                {
                    // Apply probability check to polyrhythm
                    if (juce::Random::getSystemRandom().nextFloat() < density)
                    {
                        // Constrain pitch to user-defined range then quantize to scale
                        int rawPitch = juce::jlimit(pitchMin, pitchMax, layer->pitches[layer->currentStep]);
                        int pitch = scaleQuantizer.quantize(rawPitch);

                        // Map velocity to user-defined range
                        float rawVelocity = layer->velocities[layer->currentStep];
                        float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));

                        // Apply velocity humanization
                        velocity = swingEngine.humanizeVelocity(velocity);

                        // Calculate swing and timing offset
                        int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));
                        int timingOffset = swingEngine.calculateTotalTimingOffset(
                            layer->currentStep, samplesPerStep, getSampleRate());

                        // Check if ratcheting should be applied
                        bool useRatcheting = ratchetEngine.shouldRatchet();
                        auto ratchetOffsets = useRatcheting ?
                            ratchetEngine.calculateRatchetOffsets(samplesPerStep) :
                            std::vector<int>{0};

                        // Schedule note(s) with optional ratcheting
                        for (size_t ratchetIdx = 0; ratchetIdx < ratchetOffsets.size(); ++ratchetIdx)
                        {
                            // Calculate velocity with ratchet decay
                            float ratchetVelocity = ratchetEngine.calculateRatchetVelocity(
                                velocity, static_cast<int>(ratchetIdx));

                            int ratchetTimingOffset = timingOffset + ratchetOffsets[ratchetIdx];

                            // Schedule note on
                            eventScheduler.scheduleNoteOn(pitch, ratchetVelocity, 1,
                                currentSamplePosition + ratchetTimingOffset);

                            // Schedule note off using gate length controller
                            int noteDuration = gateLengthController.calculateGateLengthSamples(samplesPerStep);
                            eventScheduler.scheduleNoteOff(pitch, 1,
                                currentSamplePosition + ratchetTimingOffset + noteDuration);
                        }
                    }
                    // else: probability miss - note scheduled but not played
                }

                // Advance to next step
                layer->currentStep = (layer->currentStep + 1) % layer->length;
            }
            break;
        }

        default: // Algorithmic generators
        {
            // Apply probability check to algorithmic generators
            if (juce::Random::getSystemRandom().nextFloat() < density)
            {
                // Update algorithmic engine with current parameter ranges
                algorithmicEngine.setPitchRange(pitchMin, pitchMax);
                algorithmicEngine.setVelocityRange(velocityMin, velocityMax);

                auto notes = algorithmicEngine.generateNoteSequence(1);
                if (!notes.empty() && notes[0] >= 0)
                {
                    // Constrain generated note to user-defined pitch range then quantize to scale
                    int rawPitch = juce::jlimit(pitchMin, pitchMax, notes[0]);
                    int pitch = scaleQuantizer.quantize(rawPitch);

                    auto velocities = algorithmicEngine.generateVelocitySequence(1);
                    float rawVelocity = velocities.empty() ? 0.7f : velocities[0];

                    // Map velocity to user-defined range
                    float velocity = velocityMin + (rawVelocity * (velocityMax - velocityMin));

                    // Apply velocity humanization
                    velocity = swingEngine.humanizeVelocity(velocity);

                    // Calculate swing and timing offset
                    int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));
                    int timingOffset = swingEngine.calculateTotalTimingOffset(
                        lastSubdivisionStep, samplesPerStep, getSampleRate());

                    // Check if ratcheting should be applied
                    bool useRatcheting = ratchetEngine.shouldRatchet();
                    auto ratchetOffsets = useRatcheting ?
                        ratchetEngine.calculateRatchetOffsets(samplesPerStep) :
                        std::vector<int>{0};

                    // Schedule note(s) with optional ratcheting
                    for (size_t ratchetIdx = 0; ratchetIdx < ratchetOffsets.size(); ++ratchetIdx)
                    {
                        // Calculate velocity with ratchet decay
                        float ratchetVelocity = ratchetEngine.calculateRatchetVelocity(
                            velocity, static_cast<int>(ratchetIdx));

                        int ratchetTimingOffset = timingOffset + ratchetOffsets[ratchetIdx];

                        // Schedule note on
                        eventScheduler.scheduleNoteOn(pitch, ratchetVelocity, midiChannel,
                            currentSamplePosition + ratchetTimingOffset);

                        // Schedule note off using gate length controller
                        int noteDuration = gateLengthController.calculateGateLengthSamples(samplesPerStep);
                        eventScheduler.scheduleNoteOff(pitch, midiChannel,
                            currentSamplePosition + ratchetTimingOffset + noteDuration);
                    }
                }
            }
            // else: probability miss - note scheduled but not played
            break;
        }

        case 6: // Brownian Motion
        case 7: // Perlin Noise
        case 8: // Drunk Walk
        case 9: // Lorenz Attractor
        {
            // Update stochastic engine from parameters
            auto stochasticType = static_cast<int>(parameters.getRawParameterValue(PARAM_STOCHASTIC_TYPE)->load());
            auto stepSize = parameters.getRawParameterValue(PARAM_STEP_SIZE)->load();
            auto momentum = parameters.getRawParameterValue(PARAM_MOMENTUM)->load();
            auto timeScale = parameters.getRawParameterValue(PARAM_TIME_SCALE)->load();

            // Map generator type to stochastic type
            StochasticEngine::GeneratorType type;
            int genTypeInt = static_cast<int>(generatorType);
            switch (genTypeInt)
            {
                case 6: type = StochasticEngine::GeneratorType::BrownianMotion; break;
                case 7: type = StochasticEngine::GeneratorType::PerlinNoise; break;
                case 8: type = StochasticEngine::GeneratorType::DrunkWalk; break;
                case 9: type = StochasticEngine::GeneratorType::LorenzAttractor; break;
                default: type = StochasticEngine::GeneratorType::BrownianMotion; break;
            }

            stochasticEngine.setGeneratorType(type);
            stochasticEngine.setDensity(density);
            stochasticEngine.setStepSize(stepSize);
            stochasticEngine.setMomentum(momentum);
            stochasticEngine.setTimeScale(timeScale);

            // Advance the stochastic system
            double secondsPerSubdivision = clockManager.getSamplesPerSubdivision(16) / getSampleRate();
            stochasticEngine.advance(static_cast<float>(secondsPerSubdivision));

            // Check if note should trigger based on density
            if (stochasticEngine.shouldTriggerNote())
            {
                // Get pitch from stochastic engine
                int pitch = stochasticEngine.getCurrentPitch(pitchMin, pitchMax);
                pitch = scaleQuantizer.quantize(pitch);

                // Get velocity from stochastic engine
                float velocity = stochasticEngine.getCurrentVelocity(velocityMin, velocityMax);
                velocity = swingEngine.humanizeVelocity(velocity);

                // Calculate timing offset
                int samplesPerStep = static_cast<int>(clockManager.getSamplesPerSubdivision(16));
                int timingOffset = swingEngine.calculateTotalTimingOffset(
                    lastSubdivisionStep, samplesPerStep, getSampleRate());

                // Check if ratcheting should be applied
                bool useRatcheting = ratchetEngine.shouldRatchet();
                auto ratchetOffsets = useRatcheting ?
                    ratchetEngine.calculateRatchetOffsets(samplesPerStep) :
                    std::vector<int>{0};

                // Schedule note(s) with optional ratcheting
                for (size_t ratchetIdx = 0; ratchetIdx < ratchetOffsets.size(); ++ratchetIdx)
                {
                    // Calculate velocity with ratchet decay
                    float ratchetVelocity = ratchetEngine.calculateRatchetVelocity(
                        velocity, static_cast<int>(ratchetIdx));

                    int ratchetTimingOffset = timingOffset + ratchetOffsets[ratchetIdx];

                    // Schedule note on
                    eventScheduler.scheduleNoteOn(pitch, ratchetVelocity, 1,
                        currentSamplePosition + ratchetTimingOffset);

                    // Schedule note off using gate length controller
                    int noteDuration = gateLengthController.calculateGateLengthSamples(samplesPerStep);
                    eventScheduler.scheduleNoteOff(pitch, 1,
                        currentSamplePosition + ratchetTimingOffset + noteDuration);
                }
            }
            break;
        }
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
void GenerativeMIDIProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GenerativeMIDIProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GenerativeMIDIProcessor();
}
