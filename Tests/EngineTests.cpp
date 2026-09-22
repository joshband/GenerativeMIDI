/*
  ==============================================================================
    Unit tests (Catch2) for generative engines and helpers.
  ==============================================================================
*/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "Core/EuclideanEngine.h"
#include "Core/PolyrhythmEngine.h"
#include "Core/ScaleQuantizer.h"
#include "Core/GeneratorTypeMapping.h"
#include "Core/StochasticEngine.h"
#include "DSP/ClockManager.h"
#include "Modulation/ModLfo.h"
#include "Modulation/ModulationDestination.h"

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

TEST_CASE("GeneratorTypeMapping indices cover ten UI generators", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::kCount == 10);
    REQUIRE(GeneratorTypeMapping::isPolyrhythm(1));
    REQUIRE_FALSE(GeneratorTypeMapping::isPolyrhythm(0));
    REQUIRE(GeneratorTypeMapping::isAlgorithmic(2));
    REQUIRE(GeneratorTypeMapping::isAlgorithmic(5));
    REQUIRE_FALSE(GeneratorTypeMapping::isAlgorithmic(0));
    REQUIRE_FALSE(GeneratorTypeMapping::isAlgorithmic(1));
    REQUIRE(GeneratorTypeMapping::isStochastic(6));
    REQUIRE(GeneratorTypeMapping::isStochastic(9));
    REQUIRE_FALSE(GeneratorTypeMapping::isStochastic(5));
}

TEST_CASE("GeneratorTypeMapping algorithmic enums", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(2) == AlgorithmicEngine::Markov);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(3) == AlgorithmicEngine::LSystem);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(4) == AlgorithmicEngine::CellularAutomatonType);
    REQUIRE(GeneratorTypeMapping::toAlgorithmic(5) == AlgorithmicEngine::Probabilistic);
}

TEST_CASE("GeneratorTypeMapping stochastic enums", "[mapping]")
{
    REQUIRE(GeneratorTypeMapping::toStochastic(6) == StochasticEngine::GeneratorType::BrownianMotion);
    REQUIRE(GeneratorTypeMapping::toStochastic(7) == StochasticEngine::GeneratorType::PerlinNoise);
    REQUIRE(GeneratorTypeMapping::toStochastic(8) == StochasticEngine::GeneratorType::DrunkWalk);
    REQUIRE(GeneratorTypeMapping::toStochastic(9) == StochasticEngine::GeneratorType::LorenzAttractor);
}

TEST_CASE("MarkovChain trained generateOrDefault uses learned transitions", "[markov]")
{
    MarkovChain chain(1);
    chain.learn(std::vector<int>{60, 62, 64, 62, 60, 62});

    REQUIRE(chain.hasTransitions());

    const int state[] = {60};
    const int note = chain.generateOrDefault(state, 1, 48);
    // Trained from 60→62 only in this sequence fragment of order-1 starts.
    REQUIRE(note == 62);

    // Repeated lookups from a deterministic trained state stay on the learned next.
    for (int i = 0; i < 32; ++i)
        REQUIRE(chain.generateOrDefault(state, 1, 48) == 62);
}

TEST_CASE("PolyrhythmEngine seeds audible default layer", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    REQUIRE(engine.getNumLayers() == 1);

    auto* layer = engine.getLayer(0);
    REQUIRE(layer != nullptr);
    REQUIRE(layer->enabled);
    REQUIRE(layer->length == 16);
    REQUIRE(static_cast<int>(layer->pattern.size()) == 16);

    int activeSteps = 0;
    for (bool step : layer->pattern)
        if (step)
            ++activeSteps;

    REQUIRE(activeSteps > 0);
}

TEST_CASE("PolyrhythmEngine addLayer seeds distinct pattern", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    const int second = engine.addLayer();
    REQUIRE(second == 1);
    REQUIRE(engine.getNumLayers() == 2);

    auto* layer = engine.getLayer(1);
    REQUIRE(layer != nullptr);

    int activeSteps = 0;
    for (bool step : layer->pattern)
        if (step)
            ++activeSteps;
    REQUIRE(activeSteps > 0);
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
                juce::StringArray{"Euclidean", "Polyrhythm", "Markov", "L-System", "Cellular", "Probabilistic",
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
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);

    int factoryCount = 0;
    for (int i = 0; i < manager.getNumPresets(); ++i)
    {
        if (manager.getPreset(i).isFactory)
            ++factoryCount;
    }

    REQUIRE(factoryCount == 11);
    REQUIRE(manager.getNumPresets() >= 11);

    // Load Euclidean Basic and confirm generatorType lands on index 0
    REQUIRE(manager.loadPresetByName("Euclidean Basic"));
    auto* gen = processor.apvts.getRawParameterValue("generatorType");
    REQUIRE(gen != nullptr);
    REQUIRE(static_cast<int>(gen->load()) == 0);

    // Load Brownian Drift and confirm generatorType index 6
    REQUIRE(manager.loadPresetByName("Brownian Drift"));
    REQUIRE(static_cast<int>(gen->load()) == 6);

    REQUIRE(manager.loadPresetByName("Polyrhythm Layers"));
    REQUIRE(static_cast<int>(gen->load()) == 1);

    // Factory Polyrhythm Layers embeds a PolyrhythmLayers snapshot
    const PresetManager::Preset* polyPreset = nullptr;
    for (int i = 0; i < manager.getNumPresets(); ++i)
        if (manager.getPreset(i).name == "Polyrhythm Layers")
            polyPreset = &manager.getPreset(i);
    REQUIRE(polyPreset != nullptr);
    REQUIRE(polyPreset->state.getChildWithName(PolyrhythmEngine::kStateTreeType).isValid());

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
            == Catch::Approx(6.0f));
}

namespace
{
    float rawParam(juce::AudioProcessorValueTreeState& apvts, const char* id)
    {
        auto* p = apvts.getRawParameterValue(id);
        REQUIRE(p != nullptr);
        return p->load();
    }

    juce::File writeTempPresetFile(const juce::String& fileName, const juce::String& contents)
    {
        auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("GenerativeMIDIPresetTests");
        dir.createDirectory();
        auto file = dir.getChildFile(fileName);
        file.replaceWithText(contents);
        return file;
    }

    juce::String minimalValidPresetXml(const juce::String& name)
    {
        // APVTS state type for MinimalPresetTestProcessor is "GenerativeMIDI"
        return juce::String()
            + "<GenerativeMIDIPreset name=\"" + name + "\" author=\"Test\" "
            + "category=\"Test\" description=\"round-trip\" version=\"1.0\">"
            + "<GenerativeMIDI>"
            + "<PARAM id=\"generatorType\" value=\"0.0\"/>"
            + "<PARAM id=\"tempo\" value=\"120.0\"/>"
            + "</GenerativeMIDI>"
            + "</GenerativeMIDIPreset>";
    }
}

TEST_CASE("PresetManager factory loadPreset round-trips key params", "[preset]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    MinimalPresetTestProcessor processor;
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);

    struct FactoryExpectation
    {
        const char* name;
        int generatorType;
    };

    // Order matches initializeFactoryPresets() — indices 0..10 are always factories.
    const FactoryExpectation expected[] = {
        { "Euclidean Basic", 0 },
        { "Euclidean Complex", 0 },
        { "Polyrhythm Layers", 1 },
        { "Brownian Drift", 6 },
        { "Markov Melody", 2 },
        { "L-System Fractal", 3 },
        { "Cellular Automata", 4 },
        { "Probabilistic Sparse", 5 },
        { "Ratchet Groove", 0 },
        { "Ambient Drift", 5 },
        { "Percussive Hits", 0 },
    };

    REQUIRE(manager.getNumPresets() >= 11);

    for (int i = 0; i < 11; ++i)
    {
        const auto& preset = manager.getPreset(i);
        REQUIRE(preset.isFactory);
        REQUIRE(preset.name == expected[i].name);
        REQUIRE(manager.loadPreset(i));
        REQUIRE(static_cast<int>(rawParam(processor.apvts, "generatorType")) == expected[i].generatorType);
    }

    // Euclidean Basic: steps / pulses / rotation
    REQUIRE(manager.loadPreset(0));
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanSteps")) == 16);
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanPulses")) == 4);
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanRotation")) == 0);

    // Euclidean Complex: rotated uneven pattern
    REQUIRE(manager.loadPreset(1));
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanSteps")) == 23);
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanPulses")) == 7);
    REQUIRE(static_cast<int>(rawParam(processor.apvts, "euclideanRotation")) == 3);

    // Brownian Drift: stochastic fields
    REQUIRE(manager.loadPresetByName("Brownian Drift"));
    REQUIRE(rawParam(processor.apvts, "noteDensity") == Catch::Approx(0.55f));
    REQUIRE(rawParam(processor.apvts, "stepSize") == Catch::Approx(0.15f));
    REQUIRE(rawParam(processor.apvts, "momentum") == Catch::Approx(0.85f));
}

TEST_CASE("PresetManager Polyrhythm Layers factory restores layer snapshot", "[preset][persist]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    MinimalPresetTestProcessor processor;
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);

    const int factoryLayers = polyEngine.getNumLayers();
    REQUIRE(factoryLayers >= 1);
    const int factoryDivision = polyEngine.getLayer(0)->division;

    // Mutate away from the factory snapshot captured at PresetManager construction.
    polyEngine.addLayer();
    polyEngine.setLayerDivision(0, 11);
    polyEngine.clearLayer(0);
    REQUIRE(polyEngine.getNumLayers() == factoryLayers + 1);
    REQUIRE(polyEngine.getLayer(0)->division == 11);

    REQUIRE(manager.loadPresetByName("Polyrhythm Layers"));
    REQUIRE(polyEngine.getNumLayers() == factoryLayers);
    REQUIRE(polyEngine.getLayer(0)->division == factoryDivision);
}

TEST_CASE("PresetManager importPreset rejects unsafe or invalid files", "[preset][import]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    MinimalPresetTestProcessor processor;
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);
    const int baselineCount = manager.getNumPresets();

    SECTION("oversized file")
    {
        auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
                       .getChildFile("GenerativeMIDIPresetTests");
        dir.createDirectory();
        auto huge = dir.getChildFile("oversized.gmpreset");
        // Just over the 1 MiB import hard limit
        juce::MemoryBlock blob(1024 * 1024 + 8, true);
        REQUIRE(huge.replaceWithData(blob.getData(), blob.getSize()));
        REQUIRE_FALSE(manager.importPreset(huge));
        REQUIRE(manager.getNumPresets() == baselineCount);
        huge.deleteFile();
    }

    SECTION("path traversal name")
    {
        auto file = writeTempPresetFile("traversal.gmpreset", minimalValidPresetXml("../evil"));
        REQUIRE_FALSE(manager.importPreset(file));
        REQUIRE(manager.getNumPresets() == baselineCount);
        file.deleteFile();
    }

    SECTION("malformed XML")
    {
        auto file = writeTempPresetFile("malformed.gmpreset", "not xml at all {{{");
        REQUIRE_FALSE(manager.importPreset(file));
        REQUIRE(manager.getNumPresets() == baselineCount);
        file.deleteFile();
    }

    SECTION("wrong root tag")
    {
        const juce::String xml =
            "<WrongRoot name=\"Sneaky\" author=\"Test\" category=\"Test\" description=\"x\">"
            "<GenerativeMIDI>"
            "<PARAM id=\"generatorType\" value=\"0.0\"/>"
            "</GenerativeMIDI>"
            "</WrongRoot>";
        auto file = writeTempPresetFile("wrongroot.gmpreset", xml);
        REQUIRE_FALSE(manager.importPreset(file));
        REQUIRE(manager.getNumPresets() == baselineCount);
        file.deleteFile();
    }

    SECTION("missing state child fails closed")
    {
        const juce::String xml =
            "<GenerativeMIDIPreset name=\"NoState\" author=\"Test\" "
            "category=\"Test\" description=\"x\" version=\"1.0\"/>";
        auto file = writeTempPresetFile("nostate.gmpreset", xml);
        REQUIRE_FALSE(manager.importPreset(file));
        REQUIRE(manager.getNumPresets() == baselineCount);
        file.deleteFile();
    }
}

TEST_CASE("ModLfo bipolar sine stays in range and advances", "[modulation]")
{
    ModLfo lfo;
    lfo.setRateHz(1.0f);
    lfo.reset();

    REQUIRE(lfo.getBipolar() == Catch::Approx(0.0f).margin(1.0e-5f));

    // Advance a quarter period at 1 Hz → ~+1
    lfo.advance(0.25);
    REQUIRE(lfo.getBipolar() == Catch::Approx(1.0f).margin(0.02f));

    // Half period from start → ~0 (after another quarter)
    lfo.advance(0.25);
    REQUIRE(lfo.getBipolar() == Catch::Approx(0.0f).margin(0.02f));

    REQUIRE(ModLfo::applyToUnipolar(0.5f, 1.0f, 0.25f) == Catch::Approx(0.75f));
    REQUIRE(ModLfo::applyToUnipolar(0.1f, -1.0f, 0.5f) == Catch::Approx(0.0f));
    REQUIRE(kModulationDestinationCount >= 2);
    REQUIRE(static_cast<int>(ModulationDestination::Velocity) == 0);
}

TEST_CASE("PolyrhythmEngine ValueTree round-trips layer fields", "[polyrhythm][persist]")
{
    PolyrhythmEngine engine;
    engine.clearLayer(0);
    engine.setLayerDivision(0, 5);
    engine.setLayerLength(0, 8);
    engine.setLayerPhase(0, 0.25f);
    engine.setLayerPitchOffset(0, -7);
    engine.setLayerVelocityMultiplier(0, 1.5f);
    engine.setStep(0, 0, true, 0.9f, 62);
    engine.setStep(0, 3, true, 0.55f, 67);
    engine.setStep(0, 7, true, 0.7f, 69);

    const int layer1 = engine.addLayer();
    engine.setLayerDivision(layer1, 7);
    engine.setLayerEnabled(layer1, false);
    engine.setLayerPitchOffset(layer1, 12);
    engine.clearLayer(layer1);
    engine.setStep(layer1, 1, true, 1.0f, 48);

    const auto tree = engine.toValueTree();
    REQUIRE(tree.hasType(PolyrhythmEngine::kStateTreeType));
    REQUIRE(tree.getNumChildren() == 2);

    PolyrhythmEngine restored;
    restored.loadFromValueTree(tree);

    REQUIRE(restored.getNumLayers() == 2);

    auto* a = restored.getLayer(0);
    auto* b = restored.getLayer(1);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);

    REQUIRE(a->division == 5);
    REQUIRE(a->length == 8);
    REQUIRE(a->phase == Catch::Approx(0.25f));
    REQUIRE(a->pitchOffset == -7);
    REQUIRE(a->velocityMultiplier == Catch::Approx(1.5f));
    REQUIRE(a->pattern[0]);
    REQUIRE_FALSE(a->pattern[1]);
    REQUIRE(a->pattern[3]);
    REQUIRE(a->pattern[7]);
    REQUIRE(a->velocities[0] == Catch::Approx(0.9f));
    REQUIRE(a->pitches[3] == 67);

    REQUIRE(b->division == 7);
    REQUIRE_FALSE(b->enabled);
    REQUIRE(b->pitchOffset == 12);
    REQUIRE(b->pattern[1]);
    REQUIRE(b->pitches[1] == 48);
}

TEST_CASE("migrateGeneratorTypeIndex shifts 9-gen layout", "[mapping][migration]")
{
    REQUIRE(GeneratorTypeMapping::migrateGeneratorTypeIndex(0) == 0);
    REQUIRE(GeneratorTypeMapping::migrateGeneratorTypeIndex(1) == 2); // was Markov
    REQUIRE(GeneratorTypeMapping::migrateGeneratorTypeIndex(4) == 5);
    REQUIRE(GeneratorTypeMapping::migrateGeneratorTypeIndex(8) == 9);
    REQUIRE(GeneratorTypeMapping::migrateGeneratorTypeIndex(9) == 9);
}

TEST_CASE("migrateApvtsStateIfNeeded rewrites generatorType PARAM", "[mapping][migration]")
{
    juce::ValueTree state("GenerativeMIDI");
    juce::ValueTree param("PARAM");
    param.setProperty("id", "generatorType", nullptr);
    param.setProperty("value", 1.0f, nullptr); // old Markov
    state.appendChild(param, nullptr);

    GeneratorTypeMapping::migrateApvtsStateIfNeeded(state, "1.0");
    REQUIRE((float) state.getChildWithProperty("id", "generatorType").getProperty("value")
            == Catch::Approx(2.0f));

    // Already 1.1 — no shift (Polyrhythm stays 1)
    juce::ValueTree state2("GenerativeMIDI");
    juce::ValueTree param2("PARAM");
    param2.setProperty("id", "generatorType", nullptr);
    param2.setProperty("value", 1.0f, nullptr);
    state2.appendChild(param2, nullptr);
    GeneratorTypeMapping::migrateApvtsStateIfNeeded(state2, "1.1");
    REQUIRE((float) state2.getChildWithProperty("id", "generatorType").getProperty("value")
            == Catch::Approx(1.0f));

    // 1.2 (layer persistence) must not re-shift generatorType
    juce::ValueTree state3("GenerativeMIDI");
    juce::ValueTree param3("PARAM");
    param3.setProperty("id", "generatorType", nullptr);
    param3.setProperty("value", 1.0f, nullptr);
    state3.appendChild(param3, nullptr);
    GeneratorTypeMapping::migrateApvtsStateIfNeeded(state3, "1.2");
    REQUIRE((float) state3.getChildWithProperty("id", "generatorType").getProperty("value")
            == Catch::Approx(1.0f));
}

TEST_CASE("PolyrhythmEngine division rate diverges over ticks", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    REQUIRE(engine.getNumLayers() >= 1);

    const int a = 0;
    const int b = engine.addLayer();
    engine.setLayerDivision(a, 4);  // advance every 4 sixteenth ticks
    engine.setLayerDivision(b, 16); // advance every tick
    engine.setLayerLength(a, 16);
    engine.setLayerLength(b, 16);
    engine.resetLayer(a);
    engine.resetLayer(b);

    int advancesA = 0;
    int advancesB = 0;
    for (int tick = 0; tick < 16; ++tick)
    {
        if (engine.shouldEmitOnThisTick(a, 16))
        {
            ++advancesA;
            engine.advanceStep(a);
        }
        if (engine.shouldEmitOnThisTick(b, 16))
        {
            ++advancesB;
            engine.advanceStep(b);
        }
    }

    REQUIRE(advancesA == 4);
    REQUIRE(advancesB == 16);
}
