/*
  ==============================================================================
    Unit tests (Catch2) for generative engines and helpers.
  ==============================================================================
*/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "Core/EuclideanEngine.h"
#include "Core/ScaleQuantizer.h"
#include "Core/GeneratorTypeMapping.h"
#include "Core/StochasticEngine.h"
#include "DSP/ClockManager.h"

TEST_CASE("EuclideanEngine pulse count matches requested pulses", "[euclidean]")
{
    EuclideanEngine engine;
    engine.setSteps(16);
    engine.setPulses(4);
    engine.setRotation(0);

    int pulseCount = 0;
    for (int i = 0; i < engine.getSteps(); ++i)
        if (engine.getStep(i))
            ++pulseCount;

    REQUIRE(pulseCount == 4);
}

TEST_CASE("EuclideanEngine setRotation is idempotent for same value", "[euclidean]")
{
    EuclideanEngine engine;
    engine.setSteps(8);
    engine.setPulses(3);
    engine.setRotation(2);
    const int first = engine.getRotation();
    engine.setRotation(2);
    REQUIRE(engine.getRotation() == first);
}

TEST_CASE("EuclideanEngine zero pulses yields empty pattern", "[euclidean]")
{
    EuclideanEngine engine;
    engine.setSteps(8);
    engine.setPulses(0);

    for (int i = 0; i < engine.getSteps(); ++i)
        REQUIRE_FALSE(engine.getStep(i));
}

TEST_CASE("ScaleQuantizer chromatic is identity", "[scale]")
{
    ScaleQuantizer quantizer;
    quantizer.setScale(ScaleQuantizer::Scale::Chromatic);
    quantizer.setRootNote(0);
    REQUIRE(quantizer.quantize(60) == 60);
    REQUIRE(quantizer.quantize(61) == 61);
}

TEST_CASE("ScaleQuantizer major keeps C in C major", "[scale]")
{
    ScaleQuantizer quantizer;
    quantizer.setScale(ScaleQuantizer::Scale::Major);
    quantizer.setRootNote(0); // C
    REQUIRE(quantizer.quantize(60) == 60); // C
}

TEST_CASE("GeneratorTypeMapping indices cover nine UI generators", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::kCount == 9);
    REQUIRE(GeneratorTypeMapping::isAlgorithmic(1));
    REQUIRE(GeneratorTypeMapping::isAlgorithmic(4));
    REQUIRE_FALSE(GeneratorTypeMapping::isAlgorithmic(0));
    REQUIRE(GeneratorTypeMapping::isStochastic(5));
    REQUIRE(GeneratorTypeMapping::isStochastic(8));
    REQUIRE_FALSE(GeneratorTypeMapping::isStochastic(4));
}

TEST_CASE("GeneratorTypeMapping algorithmic enums", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(1) == AlgorithmicEngine::Markov);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(2) == AlgorithmicEngine::LSystem);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(3) == AlgorithmicEngine::CellularAutomatonType);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(4) == AlgorithmicEngine::Probabilistic);
}

TEST_CASE("GeneratorTypeMapping stochastic enums", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::toStochastic(5) == StochasticEngine::GeneratorType::BrownianMotion);
    REQUIRE(GeneratorTypeMapping::toStochastic(6) == StochasticEngine::GeneratorType::PerlinNoise);
    REQUIRE(GeneratorTypeMapping::toStochastic(7) == StochasticEngine::GeneratorType::DrunkWalk);
    REQUIRE(GeneratorTypeMapping::toStochastic(8) == StochasticEngine::GeneratorType::LorenzAttractor);
}

TEST_CASE("ClockManager samples per beat at 120 BPM 48kHz", "[clock]")
{
    ClockManager clock;
    clock.setSampleRate(48000.0);
    clock.setTempo(120.0);
    // 120 BPM => 0.5s per beat => 24000 samples/beat
    REQUIRE(clock.getSamplesPerBeat() == Catch::Approx(24000.0));
}

TEST_CASE("ClockManager subdivision advances with callback", "[clock]")
{
    ClockManager clock;
    clock.setSampleRate(48000.0);
    clock.setTempo(120.0);
    clock.start();

    int hits = 0;
    clock.onSubdivisionHit = [&hits](int) { ++hits; };

    // One 16th at 120BPM 48kHz = 6000 samples
    const int samplesPerSixteenth = static_cast<int>(clock.getSamplesPerSubdivision(16));
    clock.advance(samplesPerSixteenth + 1);
    REQUIRE(hits >= 1);
}

TEST_CASE("StochasticEngine respects density extremes", "[stochastic]")
{
    StochasticEngine engine;
    engine.setGeneratorType(StochasticEngine::GeneratorType::BrownianMotion);
    engine.setDensity(0.0f);
    engine.advance(0.01f);
    // With density 0, should rarely/never trigger — allow zero triggers over a few steps
    int triggers = 0;
    for (int i = 0; i < 32; ++i)
    {
        engine.advance(0.01f);
        if (engine.shouldTriggerNote())
            ++triggers;
    }
    REQUIRE(triggers == 0);
}

// ---------------------------------------------------------------------------
// PresetManager factory smoke (minimal APVTS + stub processor)
// ---------------------------------------------------------------------------

#include "Core/PresetManager.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace
{
    class MinimalPresetTestProcessor : public juce::AudioProcessor
    {
    public:
        MinimalPresetTestProcessor()
            : juce::AudioProcessor(BusesProperties()),
              apvts(*this, nullptr, juce::Identifier("GenerativeMIDI"), createLayout())
        {
        }

        static juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
        {
            std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
            params.push_back(std::make_unique<juce::AudioParameterFloat>("tempo", "Tempo", 20.0f, 400.0f, 120.0f));
            params.push_back(std::make_unique<juce::AudioParameterInt>("euclideanSteps", "Euclidean Steps", 1, 64, 16));
            params.push_back(std::make_unique<juce::AudioParameterInt>("euclideanPulses", "Euclidean Pulses", 0, 64, 4));
            params.push_back(std::make_unique<juce::AudioParameterInt>("euclideanRotation", "Euclidean Rotation", 0, 64, 0));
            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                "generatorType", "Generator Type",
                juce::StringArray{"Euclidean", "Markov", "L-System", "Cellular", "Probabilistic",
                                  "Brownian", "Perlin Noise", "Drunk Walk", "Lorenz"},
                0));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("noteDensity", "Note Density", 0.0f, 1.0f, 0.5f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("velocityMin", "Velocity Min", 0.0f, 1.0f, 0.5f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("velocityMax", "Velocity Max", 0.0f, 1.0f, 1.0f));
            params.push_back(std::make_unique<juce::AudioParameterInt>("pitchMin", "Pitch Min", 0, 127, 48));
            params.push_back(std::make_unique<juce::AudioParameterInt>("pitchMax", "Pitch Max", 0, 127, 84));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("gateLength", "Gate Length", 0.01f, 2.0f, 0.8f));
            params.push_back(std::make_unique<juce::AudioParameterBool>("legatoMode", "Legato Mode", false));
            params.push_back(std::make_unique<juce::AudioParameterInt>("ratchetCount", "Ratchet Count", 1, 16, 1));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("ratchetProbability", "Ratchet Probability", 0.0f, 1.0f, 0.0f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("ratchetDecay", "Ratchet Decay", 0.0f, 1.0f, 0.5f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("stepSize", "Step Size", 0.01f, 1.0f, 0.1f));
            params.push_back(std::make_unique<juce::AudioParameterFloat>("momentum", "Momentum", 0.0f, 1.0f, 0.9f));
            return { params.begin(), params.end() };
        }

        const juce::String getName() const override { return "MinimalPresetTest"; }
        void prepareToPlay(double, int) override {}
        void releaseResources() override {}
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
        double getTailLengthSeconds() const override { return 0.0; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return true; }
        bool isMidiEffect() const override { return true; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}
        void getStateInformation(juce::MemoryBlock&) override {}
        void setStateInformation(const void*, int) override {}
        bool hasEditor() const override { return false; }
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }

        juce::AudioProcessorValueTreeState apvts;
    };
}

TEST_CASE("PresetManager initializes non-zero factory presets", "[preset]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    MinimalPresetTestProcessor processor;
    PresetManager manager(processor.apvts);

    int factoryCount = 0;
    for (int i = 0; i < manager.getNumPresets(); ++i)
    {
        if (manager.getPreset(i).isFactory)
            ++factoryCount;
    }

    REQUIRE(factoryCount == 10);
    REQUIRE(manager.getNumPresets() >= 10);

    // Load Euclidean Basic and confirm generatorType lands on index 0
    REQUIRE(manager.loadPresetByName("Euclidean Basic"));
    auto* gen = processor.apvts.getRawParameterValue("generatorType");
    REQUIRE(gen != nullptr);
    REQUIRE(static_cast<int>(gen->load()) == 0);

    // Load Brownian Drift and confirm generatorType index 5
    REQUIRE(manager.loadPresetByName("Brownian Drift"));
    REQUIRE(static_cast<int>(gen->load()) == 5);

    // Factory ValueTrees must carry PARAM children with remapped IDs
    const PresetManager::Preset* brownianPtr = nullptr;
    for (int i = 0; i < manager.getNumPresets(); ++i)
        if (manager.getPreset(i).name == "Brownian Drift")
            brownianPtr = &manager.getPreset(i);
    REQUIRE(brownianPtr != nullptr);
    REQUIRE(brownianPtr->isFactory);
    REQUIRE(brownianPtr->state.isValid());
    REQUIRE(brownianPtr->state.getNumChildren() > 0);
    REQUIRE(brownianPtr->state.getChildWithProperty("id", "generatorType").isValid());
    REQUIRE(brownianPtr->state.getChildWithProperty("id", "stepSize").isValid());
    REQUIRE((float) brownianPtr->state.getChildWithProperty("id", "generatorType").getProperty("value")
            == Catch::Approx(5.0f));
}
