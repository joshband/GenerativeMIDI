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
