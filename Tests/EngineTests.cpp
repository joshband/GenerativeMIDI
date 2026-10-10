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
#include "Core/HarmonyParts.h"
#include "Core/GeneratorTypeMapping.h"
#include "Core/StochasticEngine.h"
#include "DSP/ClockManager.h"
#include "DSP/PianoSynth.h"
#include "Modulation/ModLfo.h"
#include "Modulation/ModulationDestination.h"
#include "Modulation/ModulationRouter.h"

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
