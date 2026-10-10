/*
  ==============================================================================
    PolyrhythmEngine.h

    Polyrhythmic and polymeter sequencing engine
    Supports multiple simultaneous time divisions and phase relationships

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <memory>
#include <vector>

struct PolyrhythmLayer
{
    int division = 4;           // Time division (e.g., 4 = quarter notes vs 16th clock)
    int length = 16;            // Pattern length in steps
    float phase = 0.0f;         // Phase offset (0.0 - 1.0)
    bool enabled = true;

    int pitchOffset = 0;              // Layer transpose in semitones (-24..24)
    float velocityMultiplier = 1.0f;  // Layer velocity scale (0..2)

    std::vector<bool> pattern;
    std::vector<float> velocities;
    std::vector<int> pitches;   // MIDI note numbers

    void resize(int newLength)
    {
        length = newLength;
        pattern.resize(static_cast<size_t>(length), false);
        velocities.resize(static_cast<size_t>(length), 0.8f);
        pitches.resize(static_cast<size_t>(length), 60); // Middle C
    }
};

class PolyrhythmEngine
{
public:
    /**
     * Thread model
     * ------------
     * Layer configuration is an immutable Snapshot. Message/host threads never edit a
     * published snapshot: every mutator (addLayer, setStep, loadFromValueTree, ...) copies
     * the current one, edits the copy and publishes it with an atomic pointer swap
     * (serialised by a writer-only lock the audio thread never takes). The audio thread
     * (processTick / shouldEmitOnThisTick / advanceStep) pins the current snapshot with a
     * single hazard pointer, only reads it, and never allocates, locks or frees. Replaced
     * snapshots are retired and freed later on a writer thread, once no longer pinned.
     * Live playback counters (step, tick) live outside the snapshot as atomics.
     */
    static constexpr int kMaxLayers = 32;

    PolyrhythmEngine();
    ~PolyrhythmEngine();

    // Layer management (message thread). Returns -1 if kMaxLayers is reached.
    int addLayer();
    void removeLayer(int layerIndex);
    /** Read-only view of the current snapshot. Valid until the next mutation of the engine;
        use on the message thread only, and do not hold across edits. */
    const PolyrhythmLayer* getLayer(int layerIndex) const;
    int getNumLayers() const;
    /** Make layer `layerIndex` audible if it has no active steps (seeds a quarter pattern). */
    void ensureLayerAudible(int layerIndex);
    /** Current playback step of a layer (safe from any thread). */
    int getCurrentStep(int layerIndex) const;

    // Layer configuration
    void setLayerDivision(int layerIndex, int division);
    void setLayerLength(int layerIndex, int length);
    void setLayerPhase(int layerIndex, float phase);
    void setLayerEnabled(int layerIndex, bool enabled);
    void setLayerPitchOffset(int layerIndex, int semitones);
    void setLayerVelocityMultiplier(int layerIndex, float multiplier);

    /**
     * On each clock tick (clockSubdivision usually 16 = sixteenths):
     * returns true when this layer should emit at currentStep (caller plays, then advanceStep).
     * Mapping: division 4 advances every 4 sixteenth ticks; division 16 every tick.
     */
    bool shouldEmitOnThisTick(int layerIndex, int clockSubdivision);
    void advanceStep(int layerIndex);

    /**
     * Audio-thread entry point: one clock tick for every enabled layer. For each layer
     * that is due, calls onEmit(const PolyrhythmLayer&, int step) then advances it.
     * Lock-free, allocation-free.
     */
    template <typename EmitFn>
    void processTick(int clockSubdivision, EmitFn&& onEmit)
    {
        const Snapshot* snap = pin();
        applyPendingReset(*snap);
        const int n = static_cast<int>(snap->layers.size());
        for (int i = 0; i < n; ++i)
        {
            const auto& layer = snap->layers[static_cast<size_t>(i)];
            if (!layer.enabled || layer.length <= 0 || !tickDue(i, layer, clockSubdivision))
                continue;
            onEmit(layer, play[static_cast<size_t>(i)].step.load(std::memory_order_relaxed) % layer.length);
            stepForward(i, layer);
        }
        unpin();
    }

    // Pattern editing
    void setStep(int layerIndex, int stepIndex, bool active, float velocity = 0.8f, int pitch = 60);
    void clearLayer(int layerIndex);
    void randomizeLayer(int layerIndex, float density);

    // Playback
    void advance(int layerIndex, int subdivisions); // legacy: emit-boundary then advanceStep
    void reset();
    void resetLayer(int layerIndex);

    /** Lock-free and safe from the audio thread: every layer restarts at its phase on the next
        processTick. Call it on the transport play edge so layers line up with the new grid. */
    void requestRestart() noexcept { restartRequested.store(true, std::memory_order_release); }

    // Time signature
    void setTimeSignature(int numerator, int denominator);
    void setTempo(double bpm);

    /** ValueTree type used in session / preset XML (child of APVTS root). */
    static constexpr const char* kStateTreeType = "PolyrhythmLayers";

    /** Serialize editable layer fields (not live playback counters). */
    juce::ValueTree toValueTree() const;

    /** Replace all layers from a PolyrhythmLayers tree. No-op if type mismatches. */
    void loadFromValueTree(const juce::ValueTree& tree);

private:
    struct Snapshot
    {
        std::vector<PolyrhythmLayer> layers;
        unsigned loadId = 0; // bumped when playback positions must restart (load, remove)
    };

    struct PlayState
    {
        std::atomic<int> step { 0 };
        std::atomic<int> tick { 0 };
    };

    const Snapshot* pin() noexcept;
    void unpin() noexcept { hazard.store(nullptr, std::memory_order_seq_cst); }
    void applyPendingReset(const Snapshot& snap) noexcept;
    bool tickDue(int index, const PolyrhythmLayer& layer, int clockSubdivision) noexcept;
    void stepForward(int index, const PolyrhythmLayer& layer) noexcept;
    void resetPlayState(int index, const PolyrhythmLayer& layer) noexcept;

    /** Copy current snapshot, let fn edit it, publish. fn returns false to abort. */
    template <typename Fn> bool mutate(Fn&& fn);
    template <typename Fn> void editLayer(int layerIndex, Fn&& fn);
    void publish(std::unique_ptr<Snapshot> next);

    std::atomic<const Snapshot*> current { nullptr };
    std::atomic<const Snapshot*> hazard { nullptr };       // audio thread's pinned snapshot
    std::vector<std::unique_ptr<const Snapshot>> retired;  // guarded by writeLock
    mutable juce::CriticalSection writeLock;               // writers only, never the audio thread
    std::array<PlayState, kMaxLayers> play;
    unsigned appliedLoadId = 0;                            // audio thread only
    std::atomic<bool> restartRequested { false };
    int timeSignatureNum = 4;
    int timeSignatureDenom = 4;
    double tempo = 120.0;

    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PolyrhythmEngine)
};
