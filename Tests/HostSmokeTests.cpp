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

#include <cmath>
#include <map>
#include <set>

namespace
{
/** Minimal host playhead: only isPlaying (and bpm) matter for the gate under test. */
class FakePlayHead final : public juce::AudioPlayHead
{
public:
    bool playing = false;
    double bpm = 120.0;
    juce::Optional<double> ppq; // host position in quarter notes, when the test sets one

    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo info;
        info.setIsPlaying(playing);
        info.setBpm(bpm);
        if (ppq.hasValue())
            info.setPpqPosition(*ppq);
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
    // 12 semitones of the 24-semitone full scale = half the wheel above centre (8192 + 4096).
    REQUIRE(*stats.bendValues.begin() == 12288);
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
    // The LFO swings the wheel on both sides of centre, over the full -1..1 span.
    REQUIRE(*moving.bendValues.begin() < 5000);
    REQUIRE(*moving.bendValues.rbegin() > 12288);

    setFloatParam(processor, "modLfoDepth", 0.0f);
    const auto identity = processBlocksCollectingExpression(processor, 200, blockSize);
    REQUIRE(identity.noteOns > 0);
    REQUIRE(identity.ccCount == identity.noteOns);
    REQUIRE(identity.ccValues.size() == 1);
    REQUIRE(identity.pitchBendCount == identity.noteOns);
    REQUIRE(identity.bendValues.size() == 1);
    REQUIRE(*identity.bendValues.begin() == 12288);

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

//==============================================================================
// Realtime MIDI correctness: note-off / note-on ordering

namespace
{
struct StreamReport
{
    int ons = 0;
    int offs = 0;
    int onWhileSounding = 0;   // a note-on with no preceding note-off for that pitch/channel
    int orphanOffs = 0;        // a note-off for a pitch/channel that was not sounding
    int zeroLength = 0;        // a note closed on the very sample it opened
    int stillSounding = 0;     // notes never closed by the end of the stream
    std::map<std::pair<int, int>, int64_t> lastLength; // (channel, note) -> length of the last closed note
};

/** Replays a note stream (in emitted order) and reports ordering / balance violations. */
StreamReport analyseStream(const std::vector<TimedNote>& stream)
{
    StreamReport report;
    std::map<std::pair<int, int>, int64_t> sounding; // (channel, note) -> note-on sample
    for (const auto& event : stream)
    {
        const auto key = std::make_pair(event.channel, event.note);
        if (event.on)
        {
            ++report.ons;
            if (sounding.count(key) != 0)
                ++report.onWhileSounding;
            sounding[key] = event.sample;
        }
        else
        {
            ++report.offs;
            const auto it = sounding.find(key);
            if (it == sounding.end())
            {
                ++report.orphanOffs;
                continue;
            }
            if (event.sample <= it->second)
                ++report.zeroLength;
            report.lastLength[key] = event.sample - it->second;
            sounding.erase(it);
        }
    }
    report.stillSounding = static_cast<int>(sounding.size());
    return report;
}

std::vector<TimedNote> drainScheduler(EventScheduler& scheduler, int64_t totalSamples, int blockSize)
{
    std::vector<TimedNote> out;
    for (int64_t start = 0; start < totalSamples; start += blockSize)
    {
        juce::MidiBuffer midi;
        scheduler.processEvents(start, midi, blockSize);
        for (const auto metadata : midi)
        {
            const auto message = metadata.getMessage();
            if (!message.isNoteOn() && !message.isNoteOff())
                continue;
            out.push_back({ start + metadata.samplePosition, message.getChannel(),
                            message.getNoteNumber(), message.isNoteOn() });
        }
    }
    return out;
}
} // namespace

TEST_CASE("Same-pitch notes keep a note-off before every retrigger at gate 1.0 and 2.0",
          "[scheduler][regression]")
{
    constexpr int step = 3000;
    for (const float gate : { 1.0f, 2.0f })
    {
        INFO("gate " << gate);
        EventScheduler scheduler;
        scheduler.prepare(256);
        const int duration = static_cast<int>(step * gate);
        for (int i = 0; i < 4; ++i)
            scheduler.scheduleNote(60, 0.8f, 1, static_cast<int64_t>(i) * step, duration);

        const auto stream = drainScheduler(scheduler, 12 * step, 512);
        const auto report = analyseStream(stream);

        REQUIRE(report.ons == 4);
        REQUIRE(report.onWhileSounding == 0);
        REQUIRE(report.orphanOffs == 0);
        REQUIRE(report.zeroLength == 0);
        REQUIRE(report.stillSounding == 0);
        // The newest note is never cut short by an older note's late note-off.
        REQUIRE(report.lastLength.at({ 1, 60 }) >= duration);
    }
}

TEST_CASE("Generated repeated pitches never overlap, vanish or leave a stuck note",
          "[host][scheduler][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 512;

    for (const int voiceIndex : { 0, 1 }) // Poly, Mono
    {
        for (const float gate : { 1.0f, 2.0f })
        {
            INFO("voice " << voiceIndex << " gate " << gate);
            GenerativeMIDIProcessor processor;
            FakePlayHead playHead;
            playHead.playing = true;
            playHead.bpm = 240.0;
            processor.setPlayHead(&playHead);
            processor.prepareToPlay(48000.0, blockSize);
            configureDenseEuclidean(processor);
            setFloatParam(processor, "gateLength", gate);
            setFloatParam(processor, "swingAmount", 0.0f);
            setFloatParam(processor, "timingHumanize", 0.0f);
            setIntParam(processor, "pitchMin", 60);
            setIntParam(processor, "pitchMax", 60);
            setChoiceParam(processor, "voiceMode", voiceIndex);
            setChoiceParam(processor, "partCount", 0);
            setIntParam(processor, "midiChannel", 1);

            std::vector<TimedNote> stream;
            collectNotes(processor, 40, blockSize, stream);
            setFloatParam(processor, "noteDensity", 0.0f); // let the pending note-offs drain
            std::vector<TimedNote> tail;
            collectNotes(processor, 40, blockSize, tail);
            for (auto& event : tail)
                event.sample += 40 * blockSize; // collectNotes timestamps restart at zero
            stream.insert(stream.end(), tail.begin(), tail.end());

            const auto report = analyseStream(stream);
            REQUIRE(report.ons > 4);
            REQUIRE(report.onWhileSounding == 0);
            REQUIRE(report.orphanOffs == 0);
            REQUIRE(report.zeroLength == 0);
            REQUIRE(report.stillSounding == 0);

            processor.releaseResources();
            processor.setPlayHead(nullptr);
        }
    }
}

//==============================================================================
// Realtime MIDI correctness: stuck notes

namespace
{
struct RawEvent
{
    int64_t sample = 0;
    juce::MidiMessage message;
};

std::vector<RawEvent> collectAll(GenerativeMIDIProcessor& proc, int numBlocks, int blockSize,
                                 int64_t firstSample = 0)
{
    std::vector<RawEvent> out;
    juce::AudioBuffer<float> buffer(0, blockSize);
    for (int block = 0; block < numBlocks; ++block)
    {
        juce::MidiBuffer midi;
        proc.processBlock(buffer, midi);
        for (const auto metadata : midi)
            out.push_back({ firstSample + static_cast<int64_t>(block) * blockSize + metadata.samplePosition,
                            metadata.getMessage() });
    }
    return out;
}

std::vector<TimedNote> notesOf(const std::vector<RawEvent>& events)
{
    std::vector<TimedNote> out;
    for (const auto& e : events)
        if (e.message.isNoteOn() || e.message.isNoteOff())
            out.push_back({ e.sample, e.message.getChannel(), e.message.getNoteNumber(), e.message.isNoteOn() });
    return out;
}

int countAllNotesOff(const std::vector<RawEvent>& events, int channel)
{
    int count = 0;
    for (const auto& e : events)
        if (e.message.isAllNotesOff() && e.message.getChannel() == channel)
            ++count;
    return count;
}

int lastPitchWheel(const std::vector<RawEvent>& events, int channel)
{
    int value = -1;
    for (const auto& e : events)
        if (e.message.isPitchWheel() && e.message.getChannel() == channel)
            value = e.message.getPitchWheelValue();
    return value;
}

/** Long, overlapping notes with a non-centre pitch bend so there is plenty to clean up. */
void configureSoundingNotes(GenerativeMIDIProcessor& processor)
{
    configureDenseEuclidean(processor);
    setFloatParam(processor, "gateLength", 2.0f);
    setFloatParam(processor, "swingAmount", 0.0f);
    setFloatParam(processor, "timingHumanize", 0.0f);
    setIntParam(processor, "pitchMin", 48);
    setIntParam(processor, "pitchMax", 72);
    setChoiceParam(processor, "scaleType", 0);
    setChoiceParam(processor, "voiceMode", 0);
    setChoiceParam(processor, "partCount", 0);
    setIntParam(processor, "midiChannel", 1);
    setBoolParam(processor, "pitchbendEnable", true);
    setFloatParam(processor, "pitchbendRange", 12.0f);
    setBoolParam(processor, "modLfoEnable", false);
    setFloatParam(processor, "modLfoDepth", 0.0f);
}
} // namespace

TEST_CASE("Note-offs survive a full event queue and drops are counted", "[scheduler][regression]")
{
    EventScheduler scheduler;
    scheduler.prepare(64);
    const int capacity = scheduler.getCapacity();
    REQUIRE(capacity >= 64);

    // One note sounds before the queue fills up.
    scheduler.scheduleNoteOn(60, 0.8f, 1, 0);
    {
        juce::MidiBuffer midi;
        scheduler.processEvents(0, midi, 512);
        REQUIRE(midi.getNumEvents() == 1);
    }

    // Flood the queue with future expression events.
    for (int i = 0; i < capacity + 50; ++i)
        scheduler.scheduleCC(1, 0.5f, 1, 100000 + i);

    REQUIRE(scheduler.getQueueSize() < capacity);          // room is kept back for note-offs
    REQUIRE(scheduler.getDroppedEventCount() >= 50u);
    REQUIRE(scheduler.getDroppedNoteOffCount() == 0u);

    scheduler.scheduleNoteOff(60, 1, 600);
    REQUIRE(scheduler.getDroppedNoteOffCount() == 0u);

    juce::MidiBuffer midi;
    scheduler.processEvents(512, midi, 512);
    bool sawOff = false;
    for (const auto metadata : midi)
        sawOff = sawOff || metadata.getMessage().isNoteOff();
    REQUIRE(sawOff);
}

TEST_CASE("A flood of notes keeps every accepted note balanced", "[scheduler][regression]")
{
    EventScheduler scheduler;
    scheduler.prepare(64);

    for (int i = 0; i < 2000; ++i)
        scheduler.scheduleNote(i % 128, 0.8f, 1 + (i / 128) % 16, 10, 1000);

    REQUIRE(scheduler.getDroppedEventCount() > 0u);
    REQUIRE(scheduler.getDroppedNoteOffCount() == 0u);

    const auto report = analyseStream(drainScheduler(scheduler, 4096, 512));
    REQUIRE(report.ons > 0);
    REQUIRE(report.orphanOffs == 0);
    REQUIRE(report.stillSounding == 0);
}

TEST_CASE("allNotesOff releases sounding notes, sends CC123 and centres pitch bend",
          "[scheduler][regression]")
{
    EventScheduler scheduler;
    scheduler.prepare(256);
    scheduler.scheduleNoteOn(60, 0.8f, 3, 0);
    scheduler.scheduleNoteOn(64, 0.8f, 3, 0);
    scheduler.schedulePitchBend(0.5f, 3, 0);
    scheduler.scheduleNoteOn(67, 0.8f, 3, 5000); // still queued: must be discarded
    {
        juce::MidiBuffer midi;
        scheduler.processEvents(0, midi, 512);
    }

    juce::MidiBuffer out;
    scheduler.allNotesOff(out, 7);

    int offs = 0;
    int allOff = 0;
    int centre = 0;
    for (const auto metadata : out)
    {
        const auto m = metadata.getMessage();
        REQUIRE(metadata.samplePosition == 7);
        offs += m.isNoteOff() ? 1 : 0;
        allOff += m.isAllNotesOff() ? 1 : 0;
        centre += (m.isPitchWheel() && m.getPitchWheelValue() == 8192) ? 1 : 0;
    }
    REQUIRE(offs == 2);
    REQUIRE(allOff == 1);
    REQUIRE(centre == 1);
    REQUIRE(scheduler.getQueueSize() == 0);
    REQUIRE_FALSE(scheduler.hasSoundingNotes());

    juce::MidiBuffer again;
    scheduler.allNotesOff(again, 0);
    REQUIRE(again.getNumEvents() == 0); // nothing left to clean up
}

TEST_CASE("Transport stop releases every sounding note and re-centres pitch bend",
          "[host][scheduler][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 512;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureSoundingNotes(processor);

    auto running = collectAll(processor, 40, blockSize);
    REQUIRE(analyseStream(notesOf(running)).stillSounding > 0);
    REQUIRE(lastPitchWheel(running, 1) != 8192);

    playHead.playing = false;
    const auto stopped = collectAll(processor, 1, blockSize, 40 * blockSize);
    running.insert(running.end(), stopped.begin(), stopped.end());

    const auto report = analyseStream(notesOf(running));
    REQUIRE(report.orphanOffs == 0);
    REQUIRE(report.stillSounding == 0);
    REQUIRE(countAllNotesOff(stopped, 1) == 1);
    REQUIRE(lastPitchWheel(running, 1) == 8192);

    // Nothing else is released later: the queue was emptied.
    const auto later = collectAll(processor, 20, blockSize, 41 * blockSize);
    REQUIRE(notesOf(later).empty());

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Switching generator releases sounding notes", "[host][scheduler][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 512;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureSoundingNotes(processor);

    auto events = collectAll(processor, 40, blockSize);
    REQUIRE(analyseStream(notesOf(events)).stillSounding > 0);

    setFloatParam(processor, "noteDensity", 0.0f);
    setChoiceParam(processor, "generatorType", 2); // Markov
    const auto switched = collectAll(processor, 1, blockSize, 40 * blockSize);
    events.insert(events.end(), switched.begin(), switched.end());

    const auto report = analyseStream(notesOf(events));
    REQUIRE(report.orphanOffs == 0);
    REQUIRE(report.stillSounding == 0);
    REQUIRE(countAllNotesOff(switched, 1) == 1);
    REQUIRE(lastPitchWheel(events, 1) == 8192);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Disabling pitch bend re-centres the wheel", "[host][scheduler][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 512;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureSoundingNotes(processor);

    auto events = collectAll(processor, 20, blockSize);
    REQUIRE(lastPitchWheel(events, 1) != 8192);

    setBoolParam(processor, "pitchbendEnable", false);
    setFloatParam(processor, "noteDensity", 0.0f);
    const auto after = collectAll(processor, 1, blockSize, 20 * blockSize);
    events.insert(events.end(), after.begin(), after.end());
    REQUIRE(lastPitchWheel(events, 1) == 8192);
    REQUIRE(countAllNotesOff(after, 1) == 0); // notes keep sounding; only the wheel moves

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("releaseResources releases notes on the next block", "[host][scheduler][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 512;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 240.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureSoundingNotes(processor);

    auto events = collectAll(processor, 40, blockSize);
    REQUIRE(analyseStream(notesOf(events)).stillSounding > 0);

    setFloatParam(processor, "noteDensity", 0.0f); // the restarted clock must not add new notes
    processor.releaseResources(); // no MIDI buffer here: cleanup is emitted by the next block
    processor.prepareToPlay(48000.0, blockSize);
    const auto restarted = collectAll(processor, 1, blockSize, 40 * blockSize);
    events.insert(events.end(), restarted.begin(), restarted.end());

    REQUIRE(analyseStream(notesOf(events)).stillSounding == 0);
    REQUIRE(lastPitchWheel(events, 1) == 8192);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

//==============================================================================
// Realtime MIDI correctness: sample-accurate timing

TEST_CASE("ClockManager reports the in-block sample offset of every sixteenth",
          "[clock][timing][regression]")
{
    constexpr double sampleRate = 48000.0;
    constexpr int blockSize = 1024;
    constexpr double samplesPerSixteenth = 6000.0; // 120 bpm

    ClockManager clock;
    clock.setSampleRate(sampleRate);
    clock.setTempo(120.0);
    clock.start();

    std::vector<int64_t> hits;
    std::set<int> offsets;
    int64_t blockStart = 0;
    clock.onSubdivisionHitAt = [&](int subdivision, int offset)
    {
        REQUIRE(subdivision == 16);
        REQUIRE(offset >= 0);
        REQUIRE(offset < blockSize);
        offsets.insert(offset);
        hits.push_back(blockStart + offset);
    };

    for (int block = 0; block < 100; ++block)
    {
        blockStart = static_cast<int64_t>(block) * blockSize;
        clock.advance(blockSize);
    }

    REQUIRE(hits.size() >= 15);
    for (size_t k = 0; k < hits.size(); ++k)
    {
        INFO("hit " << k);
        REQUIRE(std::abs(static_cast<double>(hits[k]) - static_cast<double>(k) * samplesPerSixteenth) <= 1.0);
    }
    // 6000 is not a multiple of 1024: offsets must differ from block to block.
    REQUIRE(offsets.size() > 4);
}

TEST_CASE("ClockManager restart rewinds the grid and can start mid-bar", "[clock][timing][regression]")
{
    ClockManager clock;
    clock.setSampleRate(48000.0);
    clock.setTempo(120.0);
    clock.start();

    int64_t blockStart = 0;
    std::vector<int64_t> hits;
    clock.onSubdivisionHitAt = [&](int, int offset) { hits.push_back(blockStart + offset); };

    for (int block = 0; block < 10; ++block)
    {
        blockStart = static_cast<int64_t>(block) * 1024;
        clock.advance(1024);
    }
    REQUIRE(hits.size() >= 2);

    // Play edge: the next block starts a fresh grid whose first step lands on sample 0.
    hits.clear();
    clock.restart();
    blockStart = 0;
    clock.advance(1024);
    REQUIRE(hits.size() == 1);
    REQUIRE(hits[0] == 0);
    REQUIRE(clock.getPositionInSamples() == 1024);

    // Starting 2.4 sixteenths into the song: the next sixteenth is 0.6 * 6000 = 3600 samples away.
    hits.clear();
    clock.restart(2.4);
    blockStart = 0;
    for (int block = 0; block < 5; ++block)
    {
        blockStart = static_cast<int64_t>(block) * 1024;
        clock.advance(1024);
    }
    REQUIRE_FALSE(hits.empty());
    REQUIRE(std::abs(static_cast<double>(hits[0]) - 3600.0) <= 1.0);
}

TEST_CASE("ClockManager sample position does not truncate to 32 bits", "[clock][timing][regression]")
{
    ClockManager clock;
    clock.start();
    for (int i = 0; i < 40; ++i)
        clock.advance(100000000); // 4e9 samples in total, past INT32_MAX
    REQUIRE(clock.getPositionInSamples() == static_cast<int64_t>(4000000000LL));
}

TEST_CASE("Generated notes land on the host's 16th grid inside the block",
          "[host][timing][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 1024;
    constexpr double samplesPerSixteenth = 6000.0;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 120.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "tempo", 120.0f);
    setFloatParam(processor, "swingAmount", 0.0f);
    setFloatParam(processor, "timingHumanize", 0.0f);
    setFloatParam(processor, "gateLength", 0.5f);
    setChoiceParam(processor, "voiceMode", 0);
    setChoiceParam(processor, "partCount", 0);

    std::vector<TimedNote> notes;
    std::set<int> offsetsInBlock;
    collectNotes(processor, 120, blockSize, notes);
    int ons = 0;
    for (const auto& n : notes)
    {
        if (!n.on)
            continue;
        const double ideal = std::round(static_cast<double>(n.sample) / samplesPerSixteenth) * samplesPerSixteenth;
        INFO("note-on at " << n.sample);
        REQUIRE(std::abs(static_cast<double>(n.sample) - ideal) <= 1.0);
        offsetsInBlock.insert(static_cast<int>(n.sample % blockSize));
        ++ons;
    }
    REQUIRE(ons >= 15);
    REQUIRE(offsetsInBlock.size() > 4); // not all stamped at the block start

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("A play edge restarts the step counter and the grid", "[host][timing][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 1024;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 120.0;
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "tempo", 120.0f);
    setFloatParam(processor, "swingAmount", 0.0f);
    setFloatParam(processor, "timingHumanize", 0.0f);
    setChoiceParam(processor, "partCount", 0);

    std::vector<TimedNote> scratch;
    collectNotes(processor, 13, blockSize, scratch); // ends mid-grid, step counter well past 0
    REQUIRE(processor.getCurrentStep() >= 2);

    playHead.playing = false;
    collectNotes(processor, 3, blockSize, scratch);

    playHead.playing = true;
    std::vector<TimedNote> restarted;
    collectNotes(processor, 1, blockSize, restarted);

    bool firstOnAtZero = false;
    for (const auto& n : restarted)
        if (n.on)
        {
            firstOnAtZero = (n.sample == 0);
            break;
        }
    REQUIRE(firstOnAtZero);       // the grid restarted on the play edge
    REQUIRE(processor.getCurrentStep() == 1); // and so did the step counter
    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

TEST_CASE("Bar-aligned chords follow the host bar when playback starts mid-bar",
          "[host][timing][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 1024;

    GenerativeMIDIProcessor processor;
    FakePlayHead playHead;
    playHead.playing = true;
    playHead.bpm = 120.0;
    playHead.ppq = 3.9; // 0.1 beat (= 2400 samples) before the next bar
    processor.setPlayHead(&playHead);
    processor.prepareToPlay(48000.0, blockSize);
    configureDenseEuclidean(processor);
    setFloatParam(processor, "tempo", 120.0f);
    setFloatParam(processor, "swingAmount", 0.0f);
    setFloatParam(processor, "timingHumanize", 0.0f);
    setChoiceParam(processor, "partCount", 2); // melody + root + bar chord

    std::vector<TimedNote> notes;
    collectNotes(processor, 12, blockSize, notes);

    int64_t chordSample = -1;
    for (const auto& n : notes)
        if (n.on && n.channel == 3)
        {
            chordSample = n.sample;
            break;
        }
    REQUIRE(chordSample >= 0);
    REQUIRE(std::abs(static_cast<double>(chordSample) - 2400.0) <= 1.0);

    processor.releaseResources();
    processor.setPlayHead(nullptr);
}

//==============================================================================
// Pitch bend scaling

TEST_CASE("Pitch bend range is in semitones and the LFO can bend downwards",
          "[host][expression][regression]")
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr int blockSize = 256;

    auto run = [&](float rangeSemitones, bool lfo)
    {
        GenerativeMIDIProcessor processor;
        FakePlayHead playHead;
        playHead.playing = true;
        playHead.bpm = 120.0;
        processor.setPlayHead(&playHead);
        processor.prepareToPlay(48000.0, blockSize);
        configureDenseEuclidean(processor);
        setFloatParam(processor, "tempo", 120.0f);
        enableStaticExpression(processor);
        setBoolParam(processor, "ccEnable", false);
        setFloatParam(processor, "pitchbendRange", rangeSemitones);
        if (lfo)
        {
            setBoolParam(processor, "modLfoEnable", true);
            setFloatParam(processor, "modLfoRate", 6.0f);
            setFloatParam(processor, "modLfoDepth", 0.5f);
        }
        const auto stats = processBlocksCollectingExpression(processor, 400, blockSize);
        processor.releaseResources();
        processor.setPlayHead(nullptr);
        return stats;
    };

    // Static: 6 semitones of the 24-semitone full scale = +0.25 of the wheel.
    const auto fixed = run(6.0f, false);
    REQUIRE(fixed.bendValues.size() == 1);
    REQUIRE(*fixed.bendValues.begin() == 10240);

    // A small range with a 0.5-depth LFO reaches below centre (negative bend) and above it.
    const auto modulated = run(2.0f, true);
    REQUIRE(modulated.bendValues.size() >= 2);
    REQUIRE(*modulated.bendValues.begin() < 7000);  // clearly below centre (negative bend)
    REQUIRE(*modulated.bendValues.begin() >= 0);
    REQUIRE(*modulated.bendValues.rbegin() > 9000); // and clearly above it
}
