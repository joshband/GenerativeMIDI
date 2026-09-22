/*
  ==============================================================================
    Host smoke tests — headless GenerativeMIDIProcessor + fake AudioPlayHead.

    Contrast with Standalone free-run: when getPlayHead() is null, processBlock
    always advances the clock. These tests attach a host-style playhead and
    assert transport gating (playing → note-ons; stopped → no new note-ons).
  ==============================================================================
*/

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "Core/GeneratorTypeMapping.h"
#include "Core/PolyrhythmEngine.h"

namespace
{
/** Minimal host playhead: only isPlaying (and bpm) matter for the gate under test. */
class FakePlayHead final : public juce::AudioPlayHead
{
public:
    bool playing = false;
    double bpm = 120.0;

    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setIsPlaying(playing);
        info.setBpm(bpm);
        return info;
    }
};

int countNoteOns(const juce::MidiBuffer& midi)
{
    int count = 0;
    for (const auto metadata : midi)
        if (metadata.getMessage().isNoteOn())
            ++count;
    return count;
}

void setFloatParam(GenerativeMIDIProcessor& proc, const juce::String& id, float value)
{
    if (auto* p = proc.getValueTreeState().getParameter(id))
        p->setValueNotifyingHost(p->convertTo0to1(value));
}

void setChoiceParam(GenerativeMIDIProcessor& proc, const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(proc.getValueTreeState().getParameter(id)))
        p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(index)));
}

void setIntParam(GenerativeMIDIProcessor& proc, const juce::String& id, int value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterInt*>(proc.getValueTreeState().getParameter(id)))
        p->setValueNotifyingHost(p->convertTo0to1(static_cast<float>(value)));
}

/** Dense Euclidean so note-ons appear quickly under a playing host playhead. */
void configureDenseEuclidean(GenerativeMIDIProcessor& proc)
{
    setChoiceParam(proc, "generatorType", 0); // Euclidean
    setIntParam(proc, "euclideanSteps", 16);
    setIntParam(proc, "euclideanPulses", 16);
    setIntParam(proc, "euclideanRotation", 0);
    setFloatParam(proc, "noteDensity", 1.0f);
    setFloatParam(proc, "tempo", 240.0f); // faster subdivisions for short block runs
}

int processBlocksCollectingNoteOns(GenerativeMIDIProcessor& proc,
                                   int numBlocks,
                                   int blockSize)
{
    juce::AudioBuffer<float> buffer(0, blockSize); // MIDI effect: no audio buses
    int totalNoteOns = 0;

    for (int i = 0; i < numBlocks; ++i)
    {
        juce::MidiBuffer midi;
        proc.processBlock(buffer, midi);
        totalNoteOns += countNoteOns(midi);
    }

    return totalNoteOns;
}
} // namespace

TEST_CASE("Host playhead playing: processBlock emits note-ons", "[host][smoke]")
{
    // Standalone free-run is separate (null playhead → always advance).
    // This case simulates a DAW host with transport playing.
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;
    processor.prepareToPlay(sampleRate, blockSize);
    configureDenseEuclidean(processor);

    // ~2s of audio at 512/48k ≈ enough 16ths at 240 BPM for many Euclidean hits
    constexpr int numBlocks = 200;
    const int noteOns = processBlocksCollectingNoteOns(processor, numBlocks, blockSize);

    REQUIRE(noteOns > 0);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Host playhead stopped: processBlock emits no new note-ons", "[host][smoke]")
{
    // After a playing phase schedules notes, stop transport and assert the gate:
    // clock no longer advances → no new note-ons (note-offs from prior gates OK).
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;
    processor.prepareToPlay(sampleRate, blockSize);
    configureDenseEuclidean(processor);

    const int playingNoteOns = processBlocksCollectingNoteOns(processor, 100, blockSize);
    REQUIRE(playingNoteOns > 0);

    playHead.playing = false;

    const int stoppedNoteOns = processBlocksCollectingNoteOns(processor, 100, blockSize);
    REQUIRE(stoppedNoteOns == 0);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Polyrhythm layers round-trip through get/setStateInformation", "[host][persist]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor source;
    auto& engine = source.getPolyrhythmEngine();
    engine.clearLayer(0);
    engine.setLayerDivision(0, 6);
    engine.setLayerLength(0, 12);
    engine.setLayerPhase(0, 0.5f);
    engine.setLayerPitchOffset(0, 3);
    engine.setLayerVelocityMultiplier(0, 0.75f);
    engine.setStep(0, 0, true, 0.85f, 64);
    engine.setStep(0, 4, true, 0.6f, 71);
    engine.setStep(0, 11, true, 1.0f, 55);

    const int layer1 = engine.addLayer();
    engine.setLayerDivision(layer1, 9);
    engine.setLayerEnabled(layer1, false);
    engine.setLayerPitchOffset(layer1, -5);
    engine.clearLayer(layer1);
    engine.setStep(layer1, 2, true, 0.4f, 36);

    juce::MemoryBlock stateBlob;
    source.getStateInformation(stateBlob);
    REQUIRE(stateBlob.getSize() > 0);

    // New processor starts with default layers; loading must replace them.
    GenerativeMIDIProcessor dest;
    dest.setStateInformation(stateBlob.getData(), static_cast<int>(stateBlob.getSize()));

    auto& loaded = dest.getPolyrhythmEngine();
    REQUIRE(loaded.getNumLayers() == 2);

    auto* a = loaded.getLayer(0);
    auto* b = loaded.getLayer(1);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);

    REQUIRE(a->division == 6);
    REQUIRE(a->length == 12);
    REQUIRE(a->phase == Catch::Approx(0.5f));
    REQUIRE(a->pitchOffset == 3);
    REQUIRE(a->velocityMultiplier == Catch::Approx(0.75f));
    REQUIRE(a->pattern[0]);
    REQUIRE(a->pattern[4]);
    REQUIRE(a->pattern[11]);
    REQUIRE(a->velocities[0] == Catch::Approx(0.85f));
    REQUIRE(a->pitches[4] == 71);

    REQUIRE(b->division == 9);
    REQUIRE_FALSE(b->enabled);
    REQUIRE(b->pitchOffset == -5);
    REQUIRE(b->pattern[2]);
    REQUIRE(b->pitches[2] == 36);

    REQUIRE(juce::String(GeneratorTypeMapping::kPresetSchemaVersion) == "1.2");
}
