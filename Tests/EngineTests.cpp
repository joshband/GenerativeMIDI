/*
  ==============================================================================
    Unit tests (Catch2) for generative engines and helpers.
  ==============================================================================
*/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "Core/AlgorithmicEngine.h"
#include "Core/EuclideanEngine.h"
#include "Core/PolyrhythmEngine.h"
#include "Core/ScaleQuantizer.h"
#include "Core/HarmonyParts.h"
#include "Core/GeneratorTypeMapping.h"
#include "Core/StochasticEngine.h"
#include "Core/TimeSignature.h"
#include "DSP/ClockManager.h"
#include "DSP/PianoSynth.h"
#include "Modulation/ModLfo.h"
#include "Modulation/ModulationDestination.h"
#include "Modulation/ModulationRouter.h"

#include <atomic>
#include <cmath>
#include <limits>
#include <thread>
#include <vector>

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

TEST_CASE("Untrained Markov walk follows order and step", "[markov]")
{
    AlgorithmicEngine engine;
    engine.setGeneratorType(AlgorithmicEngine::Markov);
    engine.setPitchRange(48, 72);
    engine.setMarkovControls(1, 4, 0.0f);

    REQUIRE(engine.generateNextNote() == 64);
    REQUIRE(engine.generateNextNote() == 60);
    REQUIRE(engine.generateNextNote() == 64);

    engine.setMarkovControls(3, 2, 0.0f);
    REQUIRE(engine.generateNextNote() == 66);
    REQUIRE(engine.generateNextNote() == 68);
    REQUIRE(engine.generateNextNote() == 66);
}

TEST_CASE("Markov history serial keeps advancing after the ring fills", "[markov]")
{
    AlgorithmicEngine engine;
    engine.setGeneratorType(AlgorithmicEngine::Markov);
    engine.setPitchRange(48, 72);
    engine.setMarkovControls(1, 2, 0.0f);

    int distinct = 0;
    int previous = -1;
    for (int i = 0; i < 200; ++i)
    {
        const int note = engine.generateNextNote();
        if (note != previous)
            ++distinct;
        previous = note;
    }

    REQUIRE(engine.getHistorySerial() == 200u);
    REQUIRE(engine.getHistoryCount() < 200);
    REQUIRE(engine.getHistoryNoteFromNewest(0) == previous);
    REQUIRE(distinct > 2);
}

TEST_CASE("L-System grammar and interval change the tape", "[lsystem]")
{
    char symbols[LSystemCatalog::kMaxSymbols];
    int length = 0;
    LSystemCatalog::expand(0, 4, symbols, LSystemCatalog::kMaxSymbols, length);
    REQUIRE(juce::String(symbols) == "ABAABABA");

    AlgorithmicEngine engine;
    engine.setGeneratorType(AlgorithmicEngine::LSystem);
    engine.setPitchRange(48, 84);
    engine.setLSystemControls(0, 1, 5);

    // Generation 1 is "AB": center, then center plus the interval.
    REQUIRE(engine.generateNextNote() == 66);
    REQUIRE(engine.generateNextNote() == 71);
    REQUIRE(engine.generateNextNote() == 66);

    engine.setLSystemControls(3, 1, 2);
    // Koch generation 1 is "A+B": A at center, + raises an octave, B adds the interval.
    REQUIRE(engine.generateNextNote() == 66);
    REQUIRE(engine.generateNextNote() == 80);
}

TEST_CASE("Cellular rule, seed, and listen choose the sounding cell", "[cellular]")
{
    AlgorithmicEngine engine;
    engine.setGeneratorType(AlgorithmicEngine::CellularAutomatonType);
    engine.setPitchRange(40, 71);

    engine.setCellularControls(204, 4, 4);
    const int voiced = engine.generateNextNote();
    // Identity rule holds one live cell, so the pitch stays put.
    REQUIRE(voiced >= 40);
    REQUIRE(voiced <= 71);
    REQUIRE(voiced != 60);
    REQUIRE(engine.generateNextNote() == voiced);

    engine.setCellularControls(204, 4, 0);
    REQUIRE(engine.generateNextNote() == -1);

    engine.setCellularControls(0, 4, 4);
    REQUIRE(engine.generateNextNote() == -1);
}

TEST_CASE("Cellular rule 30 pitch changes as the row evolves", "[cellular]")
{
    AlgorithmicEngine engine;
    engine.setGeneratorType(AlgorithmicEngine::CellularAutomatonType);
    engine.setPitchRange(48, 84);
    engine.setCellularControls(30, 16, 16);

    bool seen[128] {};
    int distinct = 0;
    int sounded = 0;
    for (int i = 0; i < 40; ++i)
    {
        const int note = engine.generateNextNote();
        if (note < 0)
            continue;
        REQUIRE(note <= 127);
        ++sounded;
        if (!seen[note])
        {
            seen[note] = true;
            ++distinct;
        }
    }

    REQUIRE(sounded > 1);
    REQUIRE(distinct > 1);
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
    REQUIRE(lfo.peekBipolar(0.0) == Catch::Approx(lfo.getBipolar()).margin(1.0e-5f));
    REQUIRE(lfo.peekBipolar(-0.25) == Catch::Approx(0.0f).margin(0.02f));
    REQUIRE(lfo.getBipolar() == Catch::Approx(1.0f).margin(0.02f));

    // Half period from start → ~0 (after another quarter)
    lfo.advance(0.25);
    REQUIRE(lfo.getBipolar() == Catch::Approx(0.0f).margin(0.02f));

    REQUIRE(ModLfo::applyToUnipolar(0.5f, 1.0f, 0.25f) == Catch::Approx(0.75f));
    REQUIRE(ModLfo::applyToUnipolar(0.1f, -1.0f, 0.5f) == Catch::Approx(0.0f));
    REQUIRE(kModulationDestinationCount >= 2);
    REQUIRE(static_cast<int>(ModulationDestination::Velocity) == 0);
}

TEST_CASE("Piano synth sounds a note and falls silent after release", "[piano]")
{
    PianoSynth piano;
    juce::AudioBuffer<float> buffer(2, 256);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, 0.9f), 0);

    buffer.clear();
    piano.render(buffer, midi, 48000.0);
    REQUIRE(buffer.getMagnitude(0, 0, 256) > 0.01f);
    REQUIRE(buffer.getMagnitude(1, 0, 256) > 0.01f);

    midi.clear();
    midi.addEvent(juce::MidiMessage::noteOff(1, 60), 0);
    float tail = 1.0f;
    for (int i = 0; i < 200 && tail >= 0.01f; ++i)
    {
        buffer.clear();
        piano.render(buffer, midi, 48000.0);
        midi.clear();
        tail = buffer.getMagnitude(0, 0, 256);
    }
    REQUIRE(tail < 0.01f);
}

TEST_CASE("Modulation router sums slots and keeps a zero amount as identity", "[modulation]")
{
    ModSampleHold hold;
    hold.reset();
    hold.setRateHz(10.0f);
    REQUIRE(hold.getBipolar() == Catch::Approx(0.0f));
    for (int i = 0; i < 40; ++i)
    {
        hold.advance(0.2);
        REQUIRE(hold.getBipolar() >= -1.0f);
        REQUIRE(hold.getBipolar() <= 1.0f);
    }

    ModulationRouter::Frame frame;
    frame.lfoEnabled = true;
    frame.lfo = 1.0f;
    frame.sampleHold = 1.0f;
    frame.velocityAmount = 0.0f;
    auto mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.velocityDelta == Catch::Approx(0.0f));
    REQUIRE(ModulationRouter::applyAdditive(0.5f, mix.velocityDelta, 0.0f, 1.0f) == Catch::Approx(0.5f));
    REQUIRE_FALSE(mix.ccRouted);

    frame.velocityAmount = 0.25f;
    frame.lfo = -1.0f;
    mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.velocityDelta == Catch::Approx(-0.25f));
    REQUIRE(ModulationRouter::applyAdditive(0.1f, mix.velocityDelta, 0.0f, 1.0f) == Catch::Approx(0.0f));

    frame.lfoEnabled = false;
    frame.velocityAmount = 1.0f;
    mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.velocityDelta == Catch::Approx(0.0f));

    frame.lfoEnabled = true;
    frame.lfo = 1.0f;
    frame.sampleHold = 1.0f;
    frame.extras[0] = { static_cast<int>(ModulationRouter::Source::Lfo),
                        static_cast<int>(ModulationRouter::SlotDest::Gate), 0.4f };
    frame.extras[1] = { static_cast<int>(ModulationRouter::Source::SampleHold),
                        static_cast<int>(ModulationRouter::SlotDest::Gate), 0.4f };
    mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.gateDelta == Catch::Approx(0.8f));
    REQUIRE(ModulationRouter::applyAdditive(0.5f, mix.gateDelta, 0.0f, 1.0f) == Catch::Approx(1.0f));

    frame.extras[0].dest = static_cast<int>(ModulationRouter::SlotDest::Pitch);
    frame.extras[0].amount = 1.0f;
    frame.extras[1].dest = static_cast<int>(ModulationRouter::SlotDest::Off);
    mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.pitchSemitones == Catch::Approx(12.0f));

    frame.extras[0] = { static_cast<int>(ModulationRouter::Source::SampleHold),
                        static_cast<int>(ModulationRouter::SlotDest::Cc), 0.5f };
    frame.extras[1] = { static_cast<int>(ModulationRouter::Source::Lfo),
                        static_cast<int>(ModulationRouter::SlotDest::Cc), 0.5f };
    frame.sampleHold = -1.0f;
    frame.lfo = 1.0f;
    mix = ModulationRouter::evaluate(frame);
    REQUIRE(mix.ccRouted);
    REQUIRE(mix.ccDelta == Catch::Approx(0.0f));
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

TEST_CASE("PolyrhythmEngine step toggles persist at each layer length", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    engine.setLayerLength(0, 5);
    engine.clearLayer(0);
    engine.setStep(0, 2, true, 0.4f, 70);
    engine.setStep(0, 2, false, 0.4f, 70);

    auto* layer = engine.getLayer(0);
    REQUIRE(layer != nullptr);
    REQUIRE_FALSE(layer->pattern[2]);
    REQUIRE(layer->velocities[2] == Catch::Approx(0.4f));
    REQUIRE(layer->pitches[2] == 70);

    const int second = engine.addLayer();
    engine.setLayerLength(second, 9);
    engine.clearLayer(second);
    engine.setStep(second, 8, true, 0.25f, 40);

    REQUIRE(engine.getLayer(0)->length == 5);
    REQUIRE(engine.getLayer(second)->length == 9);
    REQUIRE(engine.getLayer(0)->pattern.size() != engine.getLayer(second)->pattern.size());

    engine.setLayerLength(0, 8);
    REQUIRE_FALSE(engine.getLayer(0)->pattern[2]);
    REQUIRE(engine.getLayer(0)->pitches[2] == 70);
    REQUIRE_FALSE(engine.getLayer(0)->pattern[7]);

    PolyrhythmEngine restored;
    restored.loadFromValueTree(engine.toValueTree());
    REQUIRE(restored.getNumLayers() == 2);
    REQUIRE(restored.getLayer(0)->length == 8);
    REQUIRE_FALSE(restored.getLayer(0)->pattern[2]);
    REQUIRE(restored.getLayer(0)->velocities[2] == Catch::Approx(0.4f));
    REQUIRE(restored.getLayer(0)->pitches[2] == 70);
    REQUIRE(restored.getLayer(second)->length == 9);
    REQUIRE(restored.getLayer(second)->pattern[8]);
    REQUIRE(restored.getLayer(second)->pitches[8] == 40);
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

TEST_CASE("PolyrhythmEngine processTick emits and advances like the per-layer API", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    engine.setLayerDivision(0, 16); // every tick
    engine.setLayerLength(0, 4);
    engine.resetLayer(0);

    std::vector<int> steps;
    for (int tick = 0; tick < 9; ++tick)
        engine.processTick(16, [&](const PolyrhythmLayer& layer, int step)
        {
            REQUIRE(layer.length == 4);
            steps.push_back(step);
        });

    REQUIRE(steps == std::vector<int> { 0, 1, 2, 3, 0, 1, 2, 3, 0 });
    REQUIRE(engine.getCurrentStep(0) == 1);

    engine.setLayerEnabled(0, false);
    int emitted = 0;
    engine.processTick(16, [&](const PolyrhythmLayer&, int) { ++emitted; });
    REQUIRE(emitted == 0);
}

TEST_CASE("PolyrhythmEngine caps layers and restarts playback after a load", "[polyrhythm]")
{
    PolyrhythmEngine engine;
    while (engine.getNumLayers() < PolyrhythmEngine::kMaxLayers)
    {
        // Evaluate addLayer() first: operand order inside == is unspecified in C++
        // (clang evaluated the left side first, GCC and MSVC the right side first).
        const int added = engine.addLayer();
        REQUIRE(added == engine.getNumLayers() - 1);
    }
    REQUIRE(engine.addLayer() == -1);
    REQUIRE(engine.getNumLayers() == PolyrhythmEngine::kMaxLayers);

    engine.setLayerDivision(0, 16);
    engine.setLayerLength(0, 8);
    engine.setLayerPhase(0, 0.5f);
    const auto tree = engine.toValueTree();
    engine.processTick(16, [](const PolyrhythmLayer&, int) {});
    engine.processTick(16, [](const PolyrhythmLayer&, int) {});

    engine.loadFromValueTree(tree);
    int firstStep = -1;
    engine.processTick(16, [&](const PolyrhythmLayer&, int step) { if (firstStep < 0) firstStep = step; });
    REQUIRE(firstStep == 4); // phase 0.5 of 8 steps
}

TEST_CASE("PolyrhythmEngine tolerates concurrent edits while ticking", "[polyrhythm][threads]")
{
    // Audio-style reader (processTick) against message-thread writers. Meaningful under
    // -fsanitize=thread/address; otherwise checks invariants only.
    PolyrhythmEngine engine;
    std::atomic<bool> done { false };
    std::atomic<int> violations { 0 };

    std::thread audio([&]
    {
        for (int i = 0; i < 200000; ++i)
            engine.processTick(16, [&](const PolyrhythmLayer& layer, int step)
            {
                if (step < 0 || step >= layer.length
                    || layer.pattern.size() != static_cast<size_t>(layer.length)
                    || layer.pitches.size() != static_cast<size_t>(layer.length))
                    ++violations;
            });
        done = true;
    });

    juce::Random rng(99);
    int it = 0;
    while (!done.load())
    {
        const int layer = rng.nextInt(juce::jmax(1, engine.getNumLayers()));
        switch (it++ % 6)
        {
            case 0: engine.addLayer(); break;
            case 1: if (engine.getNumLayers() > 1) engine.removeLayer(0); break;
            case 2: engine.setLayerLength(layer, 1 + rng.nextInt(128)); break;
            case 3: engine.setStep(layer, rng.nextInt(8), rng.nextBool(), 0.5f, 60); break;
            case 4: engine.loadFromValueTree(engine.toValueTree()); break;
            case 5: engine.setLayerDivision(layer, 1 + rng.nextInt(16)); break;
        }
    }
    audio.join();
    REQUIRE(violations.load() == 0);
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

#include "DSP/MidiActivityLog.h"

TEST_CASE("MidiActivityLog pushes and pops note events FIFO-order", "[midi-log]")
{
    MidiActivityLog log;

    REQUIRE(log.tryPushFromMessage(juce::MidiMessage::noteOn(1, 60, (juce::uint8) 100)));
    REQUIRE(log.tryPushFromMessage(juce::MidiMessage::noteOff(1, 60)));
    REQUIRE_FALSE(log.tryPushFromMessage(juce::MidiMessage::controllerEvent(1, 1, 64)));

    MidiActivityEvent out[8];
    const int n = log.pop(out, 8);
    REQUIRE(n == 2);
    REQUIRE(out[0].type == MidiActivityEvent::Type::NoteOn);
    REQUIRE(out[0].note == 60);
    REQUIRE(out[0].velocity == 100);
    REQUIRE(out[0].channel == 1);
    REQUIRE(out[1].type == MidiActivityEvent::Type::NoteOff);
    REQUIRE(out[1].note == 60);
    REQUIRE(out[1].channel == 1);
    REQUIRE(log.getNumReady() == 0);
}

TEST_CASE("MidiActivityLog drops newest when full", "[midi-log]")
{
    MidiActivityLog log;

    for (int i = 0; i < MidiActivityLog::kMaxEvents; ++i)
        REQUIRE(log.tryPushFromMessage(juce::MidiMessage::noteOn(2, 40 + (i % 20), (juce::uint8) 80)));

    REQUIRE_FALSE(log.tryPushFromMessage(juce::MidiMessage::noteOn(2, 72, (juce::uint8) 90)));
    REQUIRE(log.getNumReady() == MidiActivityLog::kMaxEvents);

    MidiActivityEvent out[MidiActivityLog::kMaxEvents];
    REQUIRE(log.pop(out, MidiActivityLog::kMaxEvents) == MidiActivityLog::kMaxEvents);
    REQUIRE(out[0].note == 40);
}

TEST_CASE("MidiActivityLog formatEvent includes type note vel channel", "[midi-log]")
{
    MidiActivityEvent e;
    e.type = MidiActivityEvent::Type::NoteOn;
    e.note = 60;
    e.velocity = 100;
    e.channel = 3;
    const auto s = MidiActivityLog::formatEvent(e);
    REQUIRE(s.contains("ON"));
    REQUIRE(s.contains("vel"));
    REQUIRE(s.contains("ch 3"));
}

TEST_CASE("Harmony parts build a scale triad and wrap channels", "[harmony]")
{
    const int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    const auto cMajor = HarmonyParts::triadForMelody(60, 0, major, 7, false);
    REQUIRE(cMajor.root == 48);
    REQUIRE(cMajor.third == 52);
    REQUIRE(cMajor.fifth == 55);

    const auto eMajorTriad = HarmonyParts::triadForMelody(64, 0, nullptr, 0, true);
    REQUIRE(eMajorTriad.root == 52);
    REQUIRE(eMajorTriad.third == 56);
    REQUIRE(eMajorTriad.fifth == 59);

    REQUIRE(HarmonyParts::roleChannel(1, 0) == 1);
    REQUIRE(HarmonyParts::roleChannel(1, 1) == 2);
    REQUIRE(HarmonyParts::roleChannel(1, 2) == 3);
    REQUIRE(HarmonyParts::roleChannel(1, 3) == 4);
    REQUIRE(HarmonyParts::roleChannel(15, 1) == 16);
    REQUIRE(HarmonyParts::roleChannel(15, 2) == 1);
    REQUIRE(HarmonyParts::roleChannel(15, 3) == 2);
}



TEST_CASE("ScaleQuantizer stays in scale and never jumps an octave for any root", "[scale][regression]")
{
    // Regression: notes just below the root used to snap an octave away for every root except C.
    for (int scaleIndex = 1; scaleIndex <= 15; ++scaleIndex) // Major .. HarmonicMajor
    {
        for (int root = 0; root < 12; ++root)
        {
            ScaleQuantizer quantizer;
            quantizer.setScale(static_cast<ScaleQuantizer::Scale>(scaleIndex));
            quantizer.setRootNote(root);
            const auto& intervals = quantizer.getScaleIntervals();

            auto inScale = [&](int note)
            {
                const int relative = ((note - root) % 12 + 12) % 12;
                return std::find(intervals.begin(), intervals.end(), relative) != intervals.end();
            };

            // Stay away from the 0 and 127 ends, where results are clamped.
            for (int note = 12; note <= 115; ++note)
            {
                INFO("scale " << scaleIndex << " root " << root << " note " << note);

                const int nearest = quantizer.quantize(note);
                REQUIRE(inScale(nearest));
                REQUIRE(std::abs(nearest - note) <= 2);

                const int up = quantizer.quantizeUp(note);
                REQUIRE(up > note);
                REQUIRE(up - note <= 3);
                REQUIRE(inScale(up));

                const int down = quantizer.quantizeDown(note);
                REQUIRE(down < note);
                REQUIRE(note - down <= 3);
                REQUIRE(inScale(down));
            }
        }
    }
}

TEST_CASE("ScaleQuantizer custom scale is normalised to sorted pitch classes", "[scale][regression]")
{
    ScaleQuantizer quantizer;
    quantizer.setRootNote(0);
    quantizer.setCustomScale({ 14, -1, 2, 2 }); // 14 -> 2, -1 -> 11, duplicate 2

    const auto& intervals = quantizer.getScaleIntervals();
    REQUIRE(intervals.size() == 2);
    REQUIRE(intervals[0] == 2);
    REQUIRE(intervals[1] == 11);

    REQUIRE(quantizer.quantize(60) == 59); // C snaps down to B, not up to D
    REQUIRE(quantizer.quantize(62) == 62);
}

TEST_CASE("StochasticEngine drunk-walk timing belongs to each instance", "[stochastic][regression]")
{
    // Regression: the step timer was a function-local static shared by every instance.
    auto configure = [](StochasticEngine& engine)
    {
        engine.setGeneratorType(StochasticEngine::GeneratorType::DrunkWalk);
        engine.setTimeScale(1.0f);   // one step every 0.1 s
        engine.setMomentum(0.5f);    // follow = 1: value lands on the walk position at once
        engine.setStepSize(1.0f);
        engine.reset();
    };

    StochasticEngine other;
    StochasticEngine subject;
    configure(other);
    configure(subject);

    // Both are 0.06 s into a 0.1 s interval. With one shared timer, `subject` would see
    // 0.06 + 0.06 s and step; with per-instance timers neither has stepped.
    other.advance(0.06f);
    subject.advance(0.06f);
    REQUIRE(subject.getCurrentValue() == Catch::Approx(0.5f));
}

TEST_CASE("PresetManager never lets a preset name escape the preset folder", "[preset][security][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
                    .getChildFile("GenerativeMIDIPresetSecurity")
                    .getNonexistentChildFile("run", "", false);
    auto presetDir = root.getChildFile("presets");
    REQUIRE(presetDir.createDirectory());

    // A file one level above the preset folder that a hostile preset name tries to reach.
    auto victim = root.getChildFile("victim.gmpreset");
    REQUIRE(victim.replaceWithText("keep me"));

    // A preset file whose *name attribute* is a traversal path.
    presetDir.getChildFile("evil.gmpreset").replaceWithText(minimalValidPresetXml("../victim"));

    MinimalPresetTestProcessor processor;
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);
    manager.setPresetDirectoryOverride(presetDir);
    manager.scanUserPresets();

    int evilIndex = -1;
    for (int i = 0; i < manager.getNumPresets(); ++i)
    {
        REQUIRE(manager.getPreset(i).name != "../victim");
        if (manager.getPreset(i).name == "evil")
            evilIndex = i;
    }
    REQUIRE(evilIndex >= 0); // falls back to the file name

    manager.deletePreset(evilIndex);
    REQUIRE(victim.existsAsFile());
    REQUIRE_FALSE(presetDir.getChildFile("evil.gmpreset").existsAsFile());

    // Saving under an unsafe name is refused outright.
    const int before = manager.getNumPresets();
    manager.savePreset("../escape", "Test", "Test", "traversal");
    REQUIRE(manager.getNumPresets() == before);
    REQUIRE_FALSE(root.getChildFile("escape.gmpreset").existsAsFile());

    root.deleteRecursively();
}

TEST_CASE("PresetManager scan skips oversized preset files", "[preset][security][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    auto root = juce::File::getSpecialLocation(juce::File::tempDirectory)
                    .getChildFile("GenerativeMIDIPresetSecurity")
                    .getNonexistentChildFile("big", "", false);
    REQUIRE(root.createDirectory());

    // Valid XML, but well over the 1 MiB cap.
    auto xml = minimalValidPresetXml("huge");
    xml = xml.replace("description=\"round-trip\"",
                      "description=\"" + juce::String::repeatedString("x", 1100 * 1024) + "\"");
    root.getChildFile("huge.gmpreset").replaceWithText(xml);
    root.getChildFile("small.gmpreset").replaceWithText(minimalValidPresetXml("small"));

    MinimalPresetTestProcessor processor;
    PolyrhythmEngine polyEngine;
    PresetManager manager(processor.apvts, polyEngine);
    manager.setPresetDirectoryOverride(root);
    manager.scanUserPresets();

    bool sawSmall = false;
    for (int i = 0; i < manager.getNumPresets(); ++i)
    {
        REQUIRE(manager.getPreset(i).name != "huge");
        sawSmall = sawSmall || manager.getPreset(i).name == "small";
    }
    REQUIRE(sawSmall);

    root.deleteRecursively();
}

// ============================================================================
// StochasticEngine: seeding, Lorenz stability, Perlin precision
// ============================================================================

#include <cmath>
#include <vector>
#include <cstdint>

namespace
{
    using GT = StochasticEngine::GeneratorType;

    std::vector<float> runEngine(GT type, std::uint32_t seed, int steps, float dtSec)
    {
        StochasticEngine e;
        e.setGeneratorType(type);
        e.setSeed(seed);
        std::vector<float> out;
        for (int i = 0; i < steps; ++i)
        {
            e.advance(dtSec);
            out.push_back(e.getCurrentValue());
            out.push_back(e.getSecondaryValue());
            out.push_back(e.getTertiaryValue());
            out.push_back(e.shouldTriggerNote() ? 1.0f : 0.0f);
        }
        return out;
    }

    struct Stats { double mean = 0, var = 0, railFraction = 0, crossings = 0; bool finiteAndBounded = true; };

    Stats lorenzStats(float stepSize, float timeScale, float dtSec, int steps, float momentum = 0.9f)
    {
        StochasticEngine e;
        e.setGeneratorType(GT::LorenzAttractor);
        e.setSeed(1);
        e.setStepSize(stepSize);
        e.setTimeScale(timeScale);
        e.setMomentum(momentum);
        std::vector<double> v;
        Stats s;
        for (int i = 0; i < steps; ++i)
        {
            e.advance(dtSec);
            for (float f : { e.getCurrentValue(), e.getSecondaryValue(), e.getTertiaryValue() })
                if (! std::isfinite(f) || f < 0.0f || f > 1.0f)
                    s.finiteAndBounded = false;
            v.push_back(e.getCurrentValue());
            if (e.getCurrentValue() <= 0.0f || e.getCurrentValue() >= 1.0f
                || e.getSecondaryValue() <= 0.0f || e.getSecondaryValue() >= 1.0f)
                s.railFraction += 1.0;
        }
        s.railFraction /= (double) steps;
        for (size_t i = 1; i < v.size(); ++i)
            if ((v[i - 1] - 0.5) * (v[i] - 0.5) < 0.0)
                s.crossings += 1.0; // lobe switches
        for (double d : v) s.mean += d;
        s.mean /= (double) v.size();
        for (double d : v) s.var += (d - s.mean) * (d - s.mean);
        s.var /= (double) v.size();
        return s;
    }
}

TEST_CASE("StochasticEngine is deterministic for a given seed", "[stochastic][seed]")
{
    for (auto type : { GT::BrownianMotion, GT::PerlinNoise, GT::DrunkWalk, GT::LorenzAttractor })
    {
        const auto a = runEngine(type, 1234u, 500, 0.02f);
        const auto b = runEngine(type, 1234u, 500, 0.02f);
        const auto c = runEngine(type, 4321u, 500, 0.02f);
        REQUIRE(a == b);
        REQUIRE(a != c); // note triggers at least differ between seeds
    }
}

TEST_CASE("StochasticEngine reset() restarts a seeded engine identically", "[stochastic][seed]")
{
    StochasticEngine e;
    e.setGeneratorType(GT::PerlinNoise);
    e.setSeed(7u);
    std::vector<float> first, second;
    for (int i = 0; i < 100; ++i) { e.advance(0.05f); first.push_back(e.getCurrentValue()); }
    e.reset();
    for (int i = 0; i < 100; ++i) { e.advance(0.05f); second.push_back(e.getCurrentValue()); }
    REQUIRE(first == second);
}

TEST_CASE("Lorenz stays finite, bounded and moving across the parameter grid", "[stochastic][lorenz][regression]")
{
    for (float step : { 0.01f, 0.1f, 0.5f, 1.0f })
        for (float scale : { 0.01f, 0.1f, 1.0f, 10.0f })
            for (float dtSec : { 0.001f, 0.01f, 0.1f })
            {
                INFO("stepSize=" << step << " timeScale=" << scale << " dt=" << dtSec);
                const auto s = lorenzStats(step, scale, dtSec, 2000);
                REQUIRE(s.finiteAndBounded);
                REQUIRE(s.var > 1.0e-5);
                REQUIRE(s.railFraction < 0.02);
            }
}

TEST_CASE("Lorenz does not stick to the rails at fast settings", "[stochastic][lorenz][regression]")
{
    // Fast settings used to blow up the Euler step and pin the output at 0 or 1.
    StochasticEngine e;
    e.setGeneratorType(GT::LorenzAttractor);
    e.setStepSize(1.0f);
    e.setTimeScale(10.0f);
    int atRail = 0;
    const int n = 2000;
    for (int i = 0; i < n; ++i)
    {
        e.advance(0.05f);
        const float v = e.getCurrentValue();
        if (v <= 0.0f || v >= 1.0f) ++atRail;
    }
    REQUIRE(atRail < n / 20);
}

TEST_CASE("Lorenz statistics do not depend on how time is chopped into calls", "[stochastic][lorenz][regression]")
{
    // Same 200 s delivered as 1 ms, 10 ms and 100 ms calls.
    const auto a = lorenzStats(0.1f, 1.0f, 0.001f, 200000);
    const auto b = lorenzStats(0.1f, 1.0f, 0.01f, 20000);
    const auto c = lorenzStats(0.1f, 1.0f, 0.1f, 2000);
    for (const auto* s : { &a, &b, &c })
        REQUIRE(s->finiteAndBounded);
    // Lobe-switch count tracks simulated time, so it exposes tempo-dependent speed.
    REQUIRE(a.crossings > 200.0);
    REQUIRE(std::abs(b.crossings - a.crossings) < 0.15 * a.crossings);
    REQUIRE(std::abs(c.crossings - a.crossings) < 0.15 * a.crossings);
    REQUIRE(std::abs(a.mean - b.mean) < 0.06);
    REQUIRE(std::abs(a.mean - c.mean) < 0.06);
    REQUIRE(std::abs(std::sqrt(a.var) - std::sqrt(b.var)) < 0.05);
    REQUIRE(std::abs(std::sqrt(a.var) - std::sqrt(c.var)) < 0.05);
}

TEST_CASE("Perlin noise stays smooth after very long runtimes", "[stochastic][perlin][regression]")
{
    StochasticEngine e;
    e.setGeneratorType(GT::PerlinNoise);
    e.setSeed(99u);
    e.setTimeScale(10.0f);
    // ~1e7 noise-time units: beyond float's integer precision.
    for (int i = 0; i < 1000; ++i) e.advance(1000.0f);
    e.setTimeScale(1.0f);

    float prev = e.getCurrentValue();
    float maxJump = 0.0f, minV = 1.0f, maxV = 0.0f;
    for (int i = 0; i < 2000; ++i)
    {
        e.advance(0.005f);
        const float v = e.getCurrentValue();
        maxJump = std::max(maxJump, std::abs(v - prev));
        minV = std::min(minV, v);
        maxV = std::max(maxV, v);
        prev = v;
    }
    REQUIRE(maxV - minV > 0.05f); // still evolving (float time froze at ~1e7)
    REQUIRE(maxJump < 0.05f);     // and still continuous
}

TEST_CASE("Perlin noise is continuous across the time wrap", "[stochastic][perlin]")
{
    StochasticEngine e;
    e.setGeneratorType(GT::PerlinNoise);
    e.setSeed(5u);
    e.setStepSize(0.1f);  // freq 1
    e.setTimeScale(1.0f);
    // noise time passes the 256 lattice period after 25600 steps of 10 ms.
    float prev = 0.0f, maxJump = 0.0f;
    for (int i = 0; i < 25800; ++i)
    {
        e.advance(0.01f);
        const float v = e.getCurrentValue();
        if (i > 25500) maxJump = std::max(maxJump, std::abs(v - prev));
        prev = v;
    }
    REQUIRE(maxJump < 0.03f);
}

TEST_CASE("LSystemEngine keeps non-ASCII symbols distinct from ASCII ones", "[algorithmic]")
{
    // U+0141 truncates to 'A' when squeezed into a char, so with a char-keyed rule map the two
    // symbols shared one rule list and U+0141 was rewritten by the 'A' rule (or vice versa).
    const juce::juce_wchar wideA = 0x0141;

    LSystemEngine engine;
    engine.clearRules();
    engine.setAxiom(juce::String::charToString(wideA) + "A");
    engine.addRule(wideA, "B", 1.0f);
    engine.addRule('A', "AA", 1.0f);

    REQUIRE(engine.iterate(1) == juce::String("BAA"));
}

TEST_CASE("TimeSignature denominators snap to a power of two", "[clock]")
{
    REQUIRE(TimeSignature::sanitizeDenominator(1) == 1);
    REQUIRE(TimeSignature::sanitizeDenominator(2) == 2);
    REQUIRE(TimeSignature::sanitizeDenominator(4) == 4);
    REQUIRE(TimeSignature::sanitizeDenominator(8) == 8);
    REQUIRE(TimeSignature::sanitizeDenominator(16) == 16);
    REQUIRE(TimeSignature::sanitizeDenominator(3) == 2);   // tie: smaller
    REQUIRE(TimeSignature::sanitizeDenominator(5) == 4);
    REQUIRE(TimeSignature::sanitizeDenominator(7) == 8);
    REQUIRE(TimeSignature::sanitizeDenominator(12) == 8);  // tie: smaller
    REQUIRE(TimeSignature::sanitizeDenominator(15) == 16);
    REQUIRE(TimeSignature::sanitizeDenominator(0) == 1);
    REQUIRE(TimeSignature::sanitizeDenominator(-5) == 1);
    REQUIRE(TimeSignature::sanitizeDenominator(100) == 32);
}

TEST_CASE("ClockManager applies a sane time signature and bar length", "[clock]")
{
    ClockManager clock;
    clock.setSampleRate(48000.0);
    clock.setTempo(120.0);

    clock.setTimeSignature(6, 6);   // 6 is not a note value: snaps to 4
    REQUIRE(clock.getTimeSignatureDenominator() == 4);
    REQUIRE(clock.getSamplesPerBar() == Catch::Approx(clock.getSamplesPerBeat() * 6.0));

    clock.setTimeSignature(6, 8);   // 6/8: six eighth notes = three beats
    REQUIRE(clock.getSamplesPerBar() == Catch::Approx(clock.getSamplesPerBeat() * 3.0));
}

TEST_CASE("ClockManager ignores an invalid sample rate instead of hanging", "[clock]")
{
    ClockManager clock;
    clock.setSampleRate(48000.0);
    clock.setTempo(120.0);

    clock.setSampleRate(0.0);
    clock.setSampleRate(-44100.0);
    clock.setSampleRate(std::numeric_limits<double>::quiet_NaN());
    REQUIRE(clock.getSampleRate() == Catch::Approx(48000.0));

    // The clock must still advance: with a zero sample rate every subdivision would be 0 samples
    // long and this loop could never terminate.
    int hits = 0;
    clock.onSubdivisionHit = [&](int) { ++hits; };
    clock.start();
    clock.advance(48000);                 // one second at 120 bpm = 8 sixteenths
    REQUIRE(hits == 8);

    REQUIRE(std::isfinite(clock.getSamplesPerSubdivision(0)));
}
