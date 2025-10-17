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
#include <deque>

// ============================================================================
// Markov Chain Generator
// ============================================================================
class MarkovChain
{
public:
    MarkovChain(int order = 1);

    void addTransition(const std::vector<int>& state, int nextValue, float probability = 1.0f);
    void learn(const std::vector<int>& sequence);
    int generate(const std::vector<int>& currentState);
    void reset();
    void setOrder(int newOrder);
    int getOrder() const { return order; }

private:
    int order;
    std::map<std::vector<int>, std::map<int, float>> transitionTable;
    juce::Random random;
};

// ============================================================================
// L-System Generator
// ============================================================================
struct LSystemRule
{
    char symbol;
    juce::String replacement;
    float probability = 1.0f;
};

class LSystemEngine
{
public:
    LSystemEngine();

    void setAxiom(const juce::String& axiom);
    void addRule(char symbol, const juce::String& replacement, float probability = 1.0f);
    void clearRules();
    juce::String iterate(int generations);
    std::vector<int> toMidiNotes(const juce::String& sequence, int baseNote = 60);

private:
    juce::String axiom;
    std::map<char, std::vector<LSystemRule>> rules;
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
    void setState(const std::vector<bool>& initialState);
    void randomizeState(float density = 0.5f);
    std::vector<bool> step();
    std::vector<bool> getState() const { return cells; }
    void reset();

private:
    std::vector<bool> cells;
    int rule = 30; // Default to Rule 30
    std::vector<bool> initialState;
    juce::Random random;

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

    // Generate sequence
    std::vector<int> generateNoteSequence(int length);
    std::vector<bool> generateRhythmSequence(int length);
    std::vector<float> generateVelocitySequence(int length);

private:
    GeneratorType currentType = Probabilistic;

    MarkovChain markovChain;
    LSystemEngine lSystem;
    CellularAutomaton cellularAutomaton;
    ProbabilisticGenerator probabilistic;

    std::deque<int> noteHistory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlgorithmicEngine)
};
