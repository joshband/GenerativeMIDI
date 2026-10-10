/*
  ==============================================================================
    AlgorithmicEngine.h

    Algorithmic pattern generation engine
    Supports: Markov chains, L-systems, cellular automata, probabilistic generation

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <map>
#include <atomic>
#include <cstdint>

// ============================================================================
// Markov Chain Generator
// ============================================================================
class MarkovChain
{
public:
    MarkovChain(int requestedOrder = 1);

    void addTransition(const std::vector<int>& state, int nextValue, float probability = 1.0f);
    void learn(const std::vector<int>& sequence);
    int generate(const std::vector<int>& currentState);
    /** RT-friendly trained lookup: uses pre-reserved scratch key (no per-note heap). */
    int generateOrDefault(const int* state, int stateLen, int fallbackNote = 60);
    bool hasTransitions() const { return !transitionTable.empty(); }
    void reset();
    void setOrder(int newOrder);
    int getOrder() const { return order; }

private:
    int order;
    std::map<std::vector<int>, std::map<int, float>> transitionTable;
    mutable std::vector<int> lookupScratch; // reserved to `order` — trained RT lookup
    juce::Random random;
};

// ============================================================================
// L-System Generator
// ============================================================================
struct LSystemRule
{
    juce::juce_wchar symbol;
    juce::String replacement;
    float probability = 1.0f;
};

class LSystemEngine
{
public:
    LSystemEngine();

    void setAxiom(const juce::String& axiom);
    void addRule(juce::juce_wchar symbol, const juce::String& replacement, float probability = 1.0f);
    void clearRules();
    juce::String iterate(int generations);
    std::vector<int> toMidiNotes(const juce::String& sequence, int baseNote = 60);
    juce::String getAxiom() const { return axiom; }

private:
    juce::String axiom;
    std::map<juce::juce_wchar, std::vector<LSystemRule>> rules;
    juce::Random random;
};

// ============================================================================
// Cellular Automaton
// ============================================================================
class CellularAutomaton
{
public:
    CellularAutomaton(int size = 32);

    void setRule(int ruleNumber); // Wolfram rule (0-255)
    /** One cell on, the rest off. Does not allocate when the row size is unchanged. */
    void seedCell(int index);
    void setState(const std::vector<bool>& initialState);
    void randomizeState(float density = 0.5f);
    std::vector<bool> step();
    /** Advance one generation in-place (no heap after construction). */
    void stepInPlace();
    bool getCell(int index) const;
    int getSize() const { return static_cast<int>(cells.size()); }
    int getRule() const { return rule; }
    std::vector<bool> getState() const { return cells; }
    void reset();

    /**
        Lock-free snapshot of the published row (up to 64 cells).
        Returns the generation number that matches this snapshot.
    */
    int copyCells(bool* dest, int destCap, int& countOut) const;

private:
    void publish();

    std::vector<bool> cells;
    std::vector<bool> scratch; // same size as cells — used by stepInPlace
    int rule = 30; // Default to Rule 30
    std::vector<bool> initialState;
    juce::Random random;

    // Written on the audio thread, read on the message thread.
    std::atomic<uint64_t> bits { 0 };
    std::atomic<int> sizeBits { 0 };
    std::atomic<int> generation { 0 };

    bool applyRule(bool left, bool center, bool right);
};

// ============================================================================
// Probabilistic Generator
// ============================================================================
class ProbabilisticGenerator
{
public:
    ProbabilisticGenerator();

    // Note generation
    int generateNote(int center, int range, const std::vector<float>& weights);
    float generateVelocity(float mean = 0.7f, float variance = 0.2f);

    // Pattern generation
    std::vector<int> generateScale(int root, const std::vector<int>& intervals);
    std::vector<int> generateMelody(int length, int minNote, int maxNote, float stepProbability = 0.6f);
    std::vector<bool> generateRhythm(int length, float density, float grouping = 1.0f);

    // Brownian motion / random walk
    int randomWalk(int current, int step, int minValue, int maxValue);
    float randomWalkFloat(float current, float step, float minValue, float maxValue);

private:
    juce::Random random;

    float gaussianRandom(float mean, float stddev);
};

// ============================================================================
// Main Algorithmic Engine
// ============================================================================
class AlgorithmicEngine
{
public:
    enum GeneratorType
    {
        Markov,
        LSystem,
        CellularAutomatonType,
        Probabilistic
    };

    AlgorithmicEngine();
    ~AlgorithmicEngine() = default;

    // Generator selection
    void setGeneratorType(GeneratorType type);
    GeneratorType getGeneratorType() const { return currentType; }

    // Access to specific generators
    MarkovChain& getMarkovChain() { return markovChain; }
    LSystemEngine& getLSystem() { return lSystem; }
    CellularAutomaton& getCellularAutomaton() { return cellularAutomaton; }
    ProbabilisticGenerator& getProbabilistic() { return probabilistic; }

    // Generate sequence (may allocate — prefer generateNext* on audio thread)
    std::vector<int> generateNoteSequence(int length);
    std::vector<bool> generateRhythmSequence(int length);
    std::vector<float> generateVelocitySequence(int length);

    /** Realtime-safe single-note / velocity (no heap on the hot path). */
    int generateNextNote();
    float generateNextVelocity();

    int getHistoryCount() const { return historyCount; }
    /** Notes pushed into the ring. Keeps climbing after the ring is full. */
    uint32_t getHistorySerial() const { return historySerial.load(std::memory_order_acquire); }
    int getHistoryNoteFromNewest(int age) const;
    int getLastProbNote() const { return lastProbNote; }

    // Set parameter ranges
    void setPitchRange(int minPitch, int maxPitch);
    void setVelocityRange(float minVel, float maxVel);

    /** Untrained contour walk. Order is how long a direction holds, step is the interval, surprise is a jump. */
    void setMarkovControls(int order, int stepSemitones, float surprise);
    /** Grammar, rewrite depth, and semitone gap between symbols. Rebuilds a fixed note tape. */
    void setLSystemControls(int grammar, int generation, int interval);
    /** Wolfram rule, which cell starts on, and which cell gates the note. */
    void setCellularControls(int rule, int seed, int listen);

private:
    static constexpr int kHistoryCap = 128;

    void pushHistory(int note);
    void rebuildLSystemNotes();
    int nextUntrainedMarkovNote();

    GeneratorType currentType = Probabilistic;

    MarkovChain markovChain;
    LSystemEngine lSystem;
    CellularAutomaton cellularAutomaton;
    ProbabilisticGenerator probabilistic;

    int noteHistory[kHistoryCap] {};
    int historyCount = 0;
    int historyWrite = 0;
    std::atomic<uint32_t> historySerial { 0 };
    int lastProbNote = 60;

    // Parameter ranges
    int pitchMin = 48;
    int pitchMax = 84;
    float velocityMean = 0.7f;
    float velocityVariance = 0.2f;

    juce::Random random;

    int markovStep = 2;
    float markovSurprise = 0.0f;
    int markovDirection = 1;
    int markovRun = 0;

    int lsystemGrammar = 0;
    int lsystemGeneration = 4;
    int lsystemInterval = 2;
    int lsystemBuiltMin = -1;
    int lsystemBuiltMax = -1;
    int lsystemNoteCount = 0;
    int lsystemCursor = 0;
    static constexpr int kLSystemNotes = 96;
    int lsystemNotes[kLSystemNotes] {};

    int cellularSeed = -1;
    int cellularListen = 16;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlgorithmicEngine)
};

/** Fixed grammars shared by the audio tape and the pattern view. */
namespace LSystemCatalog
{
    constexpr int kCount = 4;
    constexpr int kMaxSymbols = 96;

    const char* name(int grammarIndex);
    void expand(int grammarIndex, int generations, char* dest, int capacity, int& outLength);
}
