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
    parameters(*this, nullptr, juce::Identifier("GenerativeMIDI"), createParameterLayout())
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
        juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic"},
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
    // Generate events based on selected generator type
    auto generatorType = parameters.getRawParameterValue(PARAM_GENERATOR_TYPE)->load();

    switch (static_cast<int>(generatorType))
    {
        case 0: // Euclidean
        {
            int step = lastSubdivisionStep % euclideanEngine.getSteps();
            if (euclideanEngine.getStep(step))
            {
                float velocity = euclideanEngine.getVelocity(step);
                int pitch = static_cast<int>(parameters.getRawParameterValue(PARAM_PITCH_MIN)->load()) + (step % 12);

                // Schedule note on immediately
                eventScheduler.scheduleNoteOn(pitch, velocity, 1, currentSamplePosition);

                // Schedule note off
                int noteDuration = static_cast<int>(clockManager.getSamplesPerSubdivision(16) * 0.5);
                eventScheduler.scheduleNoteOff(pitch, 1, currentSamplePosition + noteDuration);
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
                    int pitch = layer->pitches[layer->currentStep];
                    float velocity = layer->velocities[layer->currentStep];

                    // Schedule note on immediately
                    eventScheduler.scheduleNoteOn(pitch, velocity, 1, currentSamplePosition);

                    int noteDuration = static_cast<int>(clockManager.getSamplesPerSubdivision(16) * 0.5);
                    eventScheduler.scheduleNoteOff(pitch, 1, currentSamplePosition + noteDuration);
                }

                // Advance to next step
                layer->currentStep = (layer->currentStep + 1) % layer->length;
            }
            break;
        }

        default: // Algorithmic generators
        {
            auto density = parameters.getRawParameterValue(PARAM_NOTE_DENSITY)->load();
            if (juce::Random::getSystemRandom().nextFloat() < density)
            {
                auto notes = algorithmicEngine.generateNoteSequence(1);
                if (!notes.empty() && notes[0] >= 0)
                {
                    auto velocities = algorithmicEngine.generateVelocitySequence(1);
                    float velocity = velocities.empty() ? 0.7f : velocities[0];

                    // Schedule note on immediately
                    eventScheduler.scheduleNoteOn(notes[0], velocity, 1, currentSamplePosition);

                    int noteDuration = static_cast<int>(clockManager.getSamplesPerSubdivision(16) * 0.5);
                    eventScheduler.scheduleNoteOff(notes[0], 1, currentSamplePosition + noteDuration);
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
