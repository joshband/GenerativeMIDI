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

#include <atomic>
#include <set>
#include <thread>

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

void setBoolParam(GenerativeMIDIProcessor& proc, const juce::String& id, bool enabled)
{
    if (auto* p = proc.getValueTreeState().getParameter(id))
        p->setValueNotifyingHost(enabled ? 1.0f : 0.0f);
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

namespace
{
struct ExpressionStats
{
    int noteOns = 0;
    int ccCount = 0;
    int pitchBendCount = 0;
    std::set<int> ccValues;
    std::set<int> bendValues;
};

void collectExpression(const juce::MidiBuffer& midi, ExpressionStats& stats)
{
    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
            ++stats.noteOns;
        else if (message.isController())
        {
            ++stats.ccCount;
            stats.ccValues.insert(message.getControllerValue());
        }
        else if (message.isPitchWheel())
        {
            ++stats.pitchBendCount;
            stats.bendValues.insert(message.getPitchWheelValue());
        }
    }
}

ExpressionStats processBlocksCollectingExpression(GenerativeMIDIProcessor& proc,
                                                  int numBlocks,
                                                  int blockSize)
{
    juce::AudioBuffer<float> buffer(0, blockSize);
    ExpressionStats stats;

    for (int i = 0; i < numBlocks; ++i)
    {
        juce::MidiBuffer midi;
        proc.processBlock(buffer, midi);
        collectExpression(midi, stats);
    }

    return stats;
}

void enableStaticExpression(GenerativeMIDIProcessor& proc)
{
    setBoolParam(proc, "ccEnable", true);
    setIntParam(proc, "ccNumber", 1);
    setFloatParam(proc, "ccAmount", 0.5f);
    setBoolParam(proc, "pitchbendEnable", true);
    setFloatParam(proc, "pitchbendRange", 12.0f);
    setFloatParam(proc, "gateLength", 1.0f);
    setBoolParam(proc, "modLfoEnable", false);
    setFloatParam(proc, "modLfoDepth", 0.0f);
}
} // namespace

TEST_CASE("Held notes keep a static CC and pitch bend when the LFO is off", "[host][expression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 120.0;
    processor.setPlayHead(&playHead);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "tempo", 120.0f);
    enableStaticExpression(processor);

    const auto stats = processBlocksCollectingExpression(processor, 400, blockSize);

    REQUIRE(stats.noteOns > 0);
    REQUIRE(stats.ccCount == stats.noteOns);
    REQUIRE(stats.ccValues.size() == 1);
    REQUIRE(stats.pitchBendCount == stats.noteOns);
    REQUIRE(stats.bendValues.size() == 1);
    REQUIRE(juce::String(GeneratorTypeMapping::kPresetSchemaVersion) == "1.2");

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Held notes continuously modulate CC and pitch bend from the LFO", "[host][expression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 120.0;
    processor.setPlayHead(&playHead);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "tempo", 120.0f);
    enableStaticExpression(processor);
    setBoolParam(processor, "modLfoEnable", true);
    setFloatParam(processor, "modLfoRate", 6.0f);
    setFloatParam(processor, "modLfoDepth", 1.0f);

    REQUIRE(processor.getValueTreeState().getRawParameterValue("modLfoEnable")->load() > 0.5f);
    REQUIRE(processor.getValueTreeState().getRawParameterValue("modLfoDepth")->load() == Catch::Approx(1.0f));
    REQUIRE(processor.getValueTreeState().getRawParameterValue("ccEnable")->load() > 0.5f);

    const auto moving = processBlocksCollectingExpression(processor, 400, blockSize);

    REQUIRE(moving.noteOns > 0);
    REQUIRE(moving.ccCount > moving.noteOns);
    REQUIRE(moving.ccValues.size() >= 2);
    REQUIRE(moving.pitchBendCount > moving.noteOns);
    REQUIRE(moving.bendValues.size() >= 2);

    setFloatParam(processor, "modLfoDepth", 0.0f);
    const auto identity = processBlocksCollectingExpression(processor, 200, blockSize);
    REQUIRE(identity.noteOns > 0);
    REQUIRE(identity.ccCount == identity.noteOns);
    REQUIRE(identity.ccValues.size() == 1);
    REQUIRE(identity.pitchBendCount == identity.noteOns);
    REQUIRE(identity.bendValues.size() == 1);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Standalone piano plays generated notes and stays quiet when disabled", "[piano][host]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
    GenerativeMIDIProcessor processor;
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);

    REQUIRE(processor.wrapperType == juce::AudioProcessor::wrapperType_Standalone);
    REQUIRE(processor.getTotalNumOutputChannels() == 2);

    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 256;
    processor.prepareToPlay(sampleRate, blockSize);
    configureDenseEuclidean(processor);
    setBoolParam(processor, "pianoEnable", true);

    juce::AudioBuffer<float> buffer(2, blockSize);
    float peak = 0.0f;
    int noteOns = 0;
    for (int i = 0; i < 80; ++i)
    {
        buffer.clear();
        juce::MidiBuffer midi;
        processor.processBlock(buffer, midi);
        noteOns += countNoteOns(midi);
        peak = juce::jmax(peak, buffer.getMagnitude(0, 0, blockSize));
    }

    REQUIRE(noteOns > 0);
    REQUIRE(peak > 0.01f);

    setBoolParam(processor, "pianoEnable", false);
    buffer.clear();
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);
    REQUIRE(buffer.getMagnitude(0, 0, blockSize) == Catch::Approx(0.0f));

    processor.releaseResources();
}

struct TimedNote
{
    int64_t sample = 0;
    int channel = 0;
    int note = 0;
    bool on = false;
};

static void collectNotes(GenerativeMIDIProcessor& proc, int numBlocks, int blockSize, std::vector<TimedNote>& out)
{
    juce::AudioBuffer<float> buffer(0, blockSize);
    for (int block = 0; block < numBlocks; ++block)
    {
        juce::MidiBuffer midi;
        proc.processBlock(buffer, midi);
        for (const auto metadata : midi)
        {
            const auto message = metadata.getMessage();
            if (!message.isNoteOn() && !message.isNoteOff())
                continue;
            TimedNote event;
            event.sample = static_cast<int64_t>(block) * blockSize + metadata.samplePosition;
            event.channel = message.getChannel();
            event.note = message.getNoteNumber();
            event.on = message.isNoteOn();
            out.push_back(event);
        }
    }
}

static bool noteOffBeforeNextOn(const std::vector<TimedNote>& notes, int channel)
{
    int previous = -1;
    int64_t previousOn = -1;
    for (const auto& event : notes)
    {
        if (event.channel != channel || !event.on)
            continue;
        if (previous >= 0 && event.note != previous)
        {
            for (const auto& other : notes)
            {
                if (!other.on && other.channel == channel && other.note == previous
                    && other.sample > previousOn && other.sample < event.sample)
                    return true;
            }
            return false;
        }
        previous = event.note;
        previousOn = event.sample;
    }
    return false;
}

TEST_CASE("Mono cuts the previous melody note and Poly lets it ring", "[voice][host]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;

    auto run = [&](int voiceIndex)
    {
        GenerativeMIDIProcessor processor;
        FakePlayHead playHead;
        playHead.playing = true;
        playHead.bpm = 240.0;
        processor.setPlayHead(&playHead);
        processor.prepareToPlay(sampleRate, blockSize);
        configureDenseEuclidean(processor);
        setFloatParam(processor, "gateLength", 2.0f);
        setFloatParam(processor, "swingAmount", 0.0f);
        setFloatParam(processor, "timingHumanize", 0.0f);
        setIntParam(processor, "pitchMin", 48);
        setIntParam(processor, "pitchMax", 72);
        setChoiceParam(processor, "scaleType", 0);
        setChoiceParam(processor, "voiceMode", voiceIndex);
        setChoiceParam(processor, "partCount", 0);
        setIntParam(processor, "midiChannel", 1);

        std::vector<TimedNote> notes;
        collectNotes(processor, 24, blockSize, notes);
        processor.releaseResources();
        processor.setPlayHead(nullptr);
        return notes;
    };

    const auto mono = run(1);
    const auto poly = run(0);
    REQUIRE(noteOffBeforeNextOn(mono, 1));
    REQUIRE_FALSE(noteOffBeforeNextOn(poly, 1));
}

TEST_CASE("Part count routes root chord and arp onto the next channels", "[parts][host]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 512;

    auto run = [&](int partIndex, int channel)
    {
        GenerativeMIDIProcessor processor;
        FakePlayHead playHead;
        playHead.playing = true;
        playHead.bpm = 240.0;
        processor.setPlayHead(&playHead);
        processor.prepareToPlay(sampleRate, blockSize);
        configureDenseEuclidean(processor);
        setFloatParam(processor, "gateLength", 0.5f);
        setIntParam(processor, "pitchMin", 60);
        setIntParam(processor, "pitchMax", 60);
        setIntParam(processor, "scaleRoot", 0);
        setChoiceParam(processor, "scaleType", 1);
        setChoiceParam(processor, "voiceMode", 1);
        setChoiceParam(processor, "partCount", partIndex);
        setIntParam(processor, "midiChannel", channel);

        std::vector<TimedNote> notes;
        collectNotes(processor, 16, blockSize, notes);
        processor.releaseResources();
        processor.setPlayHead(nullptr);
        return notes;
    };

    const auto melodyOnly = run(0, 4);
    REQUIRE_FALSE(melodyOnly.empty());
    for (const auto& event : melodyOnly)
        if (event.on)
            REQUIRE(event.channel == 4);

    const auto parts = run(3, 1);
    std::set<int> chordNotes;
    int64_t chordSample = -1;
    bool root = false;
    bool arp = false;
    for (const auto& event : parts)
    {
        if (!event.on)
            continue;
        if (event.channel == 2)
            root = event.note == 48;
        if (event.channel == 3)
        {
            if (chordSample < 0)
                chordSample = event.sample;
            if (event.sample == chordSample)
                chordNotes.insert(event.note);
        }
        if (event.channel == 4)
            arp = true;
    }
    REQUIRE(root);
    REQUIRE(arp);
    REQUIRE(chordNotes.count(48) == 1);
    REQUIRE(chordNotes.count(52) == 1);
    REQUIRE(chordNotes.count(55) == 1);

    int offsAtChord = 0;
    for (const auto& event : parts)
        if (!event.on && event.channel == 3 && event.sample <= chordSample)
            ++offsAtChord;
    REQUIRE(offsAtChord == 0);

    const auto wrapped = run(3, 15);
    bool ch15 = false;
    bool ch16 = false;
    bool ch1 = false;
    bool ch2 = false;
    for (const auto& event : wrapped)
    {
        if (!event.on)
            continue;
        ch15 = ch15 || event.channel == 15;
        ch16 = ch16 || event.channel == 16;
        ch1 = ch1 || event.channel == 1;
        ch2 = ch2 || event.channel == 2;
    }
    REQUIRE(ch15);
    REQUIRE(ch16);
    REQUIRE(ch1);
    REQUIRE(ch2);
}


namespace
{
/** Raw note-on status bytes seen in a buffer, including velocity-0 ones that JUCE reports as note-offs. */
void collectRawNoteOns(GenerativeMIDIProcessor& proc, int numBlocks, int blockSize,
                       std::vector<std::pair<int, int>>& noteVelocity)
{
    juce::AudioBuffer<float> buffer(0, blockSize);
    for (int i = 0; i < numBlocks; ++i)
    {
        juce::MidiBuffer midi;
        proc.processBlock(buffer, midi);
        for (const auto metadata : midi)
        {
            const auto message = metadata.getMessage();
            if (message.getRawDataSize() == 3 && (message.getRawData()[0] & 0xF0) == 0x90)
                noteVelocity.emplace_back(message.getRawData()[1], message.getRawData()[2]);
        }
    }
}
} // namespace

TEST_CASE("Zero velocity range still sends audible note-ons", "[host][regression]")
{
    // Regression: velocity truncated to 0, which receivers read as a note-off.
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, 512);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "velocityMin", 0.0f);
    setFloatParam(processor, "velocityMax", 0.0f);

    std::vector<std::pair<int, int>> noteOns;
    collectRawNoteOns(processor, 200, 512, noteOns);

    REQUIRE_FALSE(noteOns.empty());
    for (const auto& [note, velocity] : noteOns)
    {
        INFO("note " << note);
        REQUIRE(velocity >= 1);
    }
}

TEST_CASE("Inverted pitch range is ordered instead of dividing by zero", "[host][regression]")
{
    // Regression: pitchMax == pitchMin - 1 made `step % (range + 1)` a modulo by zero,
    // and pitchMax < pitchMin asserted inside jlimit.
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, 512);
    configureDenseEuclidean(processor);
    setIntParam(processor, "pitchMin", 61);
    setIntParam(processor, "pitchMax", 60);

    std::vector<std::pair<int, int>> noteOns;
    collectRawNoteOns(processor, 200, 512, noteOns);

    REQUIRE_FALSE(noteOns.empty());
    for (const auto& [note, velocity] : noteOns)
    {
        juce::ignoreUnused(velocity);
        REQUIRE(note >= 60);
        REQUIRE(note <= 61);
    }
}

TEST_CASE("Polyrhythm layers can be edited while processBlock runs", "[host][polyrhythm][threads]")
{
    // Regression for a data race: the audio thread read the layer vectors while the
    // message thread resized / pushed / erased / replaced them. Run under
    // -fsanitize=thread (or address) to catch it; without a sanitizer it still has to
    // survive and keep producing sane MIDI.
    juce::ScopedJuceInitialiser_GUI juceInit;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);

    constexpr int blockSize = 128;
    processor.prepareToPlay(48000.0, blockSize);
    setChoiceParam(processor, "generatorType", 1); // Polyrhythm
    setFloatParam(processor, "noteDensity", 1.0f);
    setFloatParam(processor, "tempo", 240.0f);

    auto& engine = processor.getPolyrhythmEngine();
    engine.addLayer();
    engine.addLayer();

    std::atomic<bool> audioDone { false };
    std::atomic<int> noteOns { 0 };
    std::atomic<int> badMessages { 0 };

    std::thread audio([&]
    {
        juce::AudioBuffer<float> buffer(0, blockSize);
        for (int block = 0; block < 20000; ++block)
        {
            juce::MidiBuffer midi;
            processor.processBlock(buffer, midi);
            for (const auto metadata : midi)
            {
                const auto m = metadata.getMessage();
                if (m.getRawDataSize() == 3 && (m.getRawData()[0] & 0xF0) == 0x90)
                {
                    ++noteOns;
                    if (m.getRawData()[1] > 127 || m.getRawData()[2] < 1 || m.getRawData()[2] > 127)
                        ++badMessages;
                }
            }
        }
        audioDone = true;
    });

    juce::Random rng(1234);
    int iteration = 0;
    while (!audioDone.load())
    {
        const int layer = rng.nextInt(juce::jmax(1, engine.getNumLayers()));
        switch (iteration++ % 8)
        {
            case 0: engine.addLayer(); break;
            case 1: if (engine.getNumLayers() > 1) engine.removeLayer(engine.getNumLayers() - 1); break;
            case 2: engine.setLayerLength(layer, 1 + rng.nextInt(128)); break;
            case 3: engine.setStep(layer, rng.nextInt(16), rng.nextBool(), rng.nextFloat(), rng.nextInt(128)); break;
            case 4: engine.setLayerDivision(layer, 1 + rng.nextInt(16)); break;
            case 5: engine.setLayerEnabled(layer, true); break;
            case 6: engine.loadFromValueTree(engine.toValueTree()); break;
            case 7: engine.randomizeLayer(layer, 0.7f); break;
        }
    }

    audio.join();

    REQUIRE(badMessages.load() == 0);
    REQUIRE(noteOns.load() > 0);
    REQUIRE(engine.getNumLayers() >= 1);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}
