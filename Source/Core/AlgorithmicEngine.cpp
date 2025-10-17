/*
  ==============================================================================
    AlgorithmicEngine.cpp

    Algorithmic pattern generation implementation

  ==============================================================================
*/

#include "AlgorithmicEngine.h"
#include <cmath>

// ============================================================================
// Markov Chain Implementation
// ============================================================================
MarkovChain::MarkovChain(int order) : order(order)
{
}

void MarkovChain::addTransition(const std::vector<int>& state, int nextValue, float probability)
{
    if (state.size() != order)
        return;

    transitionTable[state][nextValue] = probability;
}

void MarkovChain::learn(const std::vector<int>& sequence)
{
    if (sequence.size() <= order)
        return;

    // Count transitions
    std::map<std::vector<int>, std::map<int, int>> counts;

    for (size_t i = 0; i <= sequence.size() - order - 1; ++i)
    {
        std::vector<int> state(sequence.begin() + i, sequence.begin() + i + order);
        int next = sequence[i + order];
        counts[state][next]++;
    }

    // Convert counts to probabilities
    for (const auto& [state, nextMap] : counts)
    {
        int total = 0;
        for (const auto& [next, count] : nextMap)
            total += count;

        for (const auto& [next, count] : nextMap)
        {
            float probability = static_cast<float>(count) / total;
            addTransition(state, next, probability);
        }
    }
}

int MarkovChain::generate(const std::vector<int>& currentState)
{
    if (currentState.size() != order)
        return 60; // Default middle C

    auto it = transitionTable.find(currentState);
    if (it == transitionTable.end() || it->second.empty())
        return 60;

    // Weighted random selection
    float r = random.nextFloat();
    float cumulative = 0.0f;

    for (const auto& [value, probability] : it->second)
    {
        cumulative += probability;
        if (r <= cumulative)
            return value;
    }

    // Fallback to last option
    return it->second.rbegin()->first;
}

void MarkovChain::reset()
{
    transitionTable.clear();
}

void MarkovChain::setOrder(int newOrder)
{
    order = juce::jlimit(1, 5, newOrder);
    reset();
}

// ============================================================================
// L-System Implementation
// ============================================================================
LSystemEngine::LSystemEngine() : axiom("A")
{
}

void LSystemEngine::setAxiom(const juce::String& ax)
{
    axiom = ax;
}

void LSystemEngine::addRule(char symbol, const juce::String& replacement, float probability)
{
    LSystemRule rule;
    rule.symbol = symbol;
    rule.replacement = replacement;
    rule.probability = juce::jlimit(0.0f, 1.0f, probability);
    rules[symbol].push_back(rule);
}

void LSystemEngine::clearRules()
{
    rules.clear();
}

juce::String LSystemEngine::iterate(int generations)
{
    juce::String current = axiom;

    for (int gen = 0; gen < generations; ++gen)
    {
        juce::String next;

        for (auto c : current)
        {
            auto it = rules.find(c);
            if (it != rules.end() && !it->second.empty())
            {
                // Stochastic rule selection
                float r = random.nextFloat();
                float cumulative = 0.0f;
                bool replaced = false;

                for (const auto& rule : it->second)
                {
                    cumulative += rule.probability;
                    if (r <= cumulative)
                    {
                        next += rule.replacement;
                        replaced = true;
                        break;
                    }
                }

                if (!replaced)
                    next += c;
            }
            else
            {
                next += c;
            }
        }

        current = next;
    }

    return current;
}

std::vector<int> LSystemEngine::toMidiNotes(const juce::String& sequence, int baseNote)
{
    std::vector<int> notes;
    int currentNote = baseNote;

    for (auto c : sequence)
    {
        switch (c)
        {
            case 'A': notes.push_back(currentNote); break;
            case 'B': notes.push_back(currentNote + 2); break;
            case 'C': notes.push_back(currentNote + 4); break;
            case 'D': notes.push_back(currentNote + 5); break;
            case 'E': notes.push_back(currentNote + 7); break;
            case 'F': notes.push_back(currentNote + 9); break;
            case 'G': notes.push_back(currentNote + 11); break;
            case '+': currentNote = juce::jlimit(0, 127, currentNote + 12); break; // Octave up
            case '-': currentNote = juce::jlimit(0, 127, currentNote - 12); break; // Octave down
            case '[': currentNote = juce::jlimit(0, 127, currentNote + 1); break;  // Semitone up
            case ']': currentNote = juce::jlimit(0, 127, currentNote - 1); break;  // Semitone down
            default: break;
        }
    }

    return notes;
}

// ============================================================================
// Cellular Automaton Implementation
// ============================================================================
CellularAutomaton::CellularAutomaton(int size) : cells(size, false)
{
    initialState = cells;
}

void CellularAutomaton::setRule(int ruleNumber)
{
    rule = juce::jlimit(0, 255, ruleNumber);
}

void CellularAutomaton::setState(const std::vector<bool>& state)
{
    cells = state;
    initialState = state;
}

void CellularAutomaton::randomizeState(float density)
{
    for (size_t i = 0; i < cells.size(); ++i)
        cells[i] = random.nextFloat() < density;
    initialState = cells;
}

std::vector<bool> CellularAutomaton::step()
{
    std::vector<bool> nextCells(cells.size());

    for (size_t i = 0; i < cells.size(); ++i)
    {
        bool left = cells[(i - 1 + cells.size()) % cells.size()];
        bool center = cells[i];
        bool right = cells[(i + 1) % cells.size()];

        nextCells[i] = applyRule(left, center, right);
    }

    cells = nextCells;
    return cells;
}

void CellularAutomaton::reset()
{
    cells = initialState;
}

bool CellularAutomaton::applyRule(bool left, bool center, bool right)
{
    int index = (left ? 4 : 0) + (center ? 2 : 0) + (right ? 1 : 0);
    return (rule >> index) & 1;
}

// ============================================================================
// Probabilistic Generator Implementation
// ============================================================================
ProbabilisticGenerator::ProbabilisticGenerator()
{
}

int ProbabilisticGenerator::generateNote(int center, int range, const std::vector<float>& weights)
{
    if (weights.empty())
        return center + random.nextInt(range * 2 + 1) - range;

    float r = random.nextFloat();
    float cumulative = 0.0f;
    float totalWeight = 0.0f;

    for (float w : weights)
        totalWeight += w;

    for (size_t i = 0; i < weights.size(); ++i)
    {
        cumulative += weights[i] / totalWeight;
        if (r <= cumulative)
            return center - range + static_cast<int>(i);
    }

    return center;
}

float ProbabilisticGenerator::generateVelocity(float mean, float variance)
{
    float vel = gaussianRandom(mean, variance);
    return juce::jlimit(0.0f, 1.0f, vel);
}

std::vector<int> ProbabilisticGenerator::generateScale(int root, const std::vector<int>& intervals)
{
    std::vector<int> scale;
    int note = root;

    scale.push_back(note);
    for (int interval : intervals)
    {
        note += interval;
        if (note <= 127)
            scale.push_back(note);
    }

    return scale;
}

std::vector<int> ProbabilisticGenerator::generateMelody(int length, int minNote, int maxNote, float stepProbability)
{
    std::vector<int> melody;
    int currentNote = minNote + random.nextInt(maxNote - minNote + 1);

    for (int i = 0; i < length; ++i)
    {
        melody.push_back(currentNote);

        if (random.nextFloat() < stepProbability)
        {
            // Step motion (1-2 semitones)
            int step = random.nextBool() ? random.nextInt(3) : -random.nextInt(3);
            currentNote = juce::jlimit(minNote, maxNote, currentNote + step);
        }
        else
        {
            // Leap motion (3-7 semitones)
            int leap = random.nextInt(5) + 3;
            if (random.nextBool())
                leap = -leap;
            currentNote = juce::jlimit(minNote, maxNote, currentNote + leap);
        }
    }

    return melody;
}

std::vector<bool> ProbabilisticGenerator::generateRhythm(int length, float density, float grouping)
{
    std::vector<bool> rhythm(length, false);

    for (int i = 0; i < length; ++i)
    {
        // Apply grouping bias
        float bias = std::fmod(i, grouping) < 1.0f ? 1.5f : 1.0f;
        rhythm[i] = random.nextFloat() < (density * bias);
    }

    return rhythm;
}

int ProbabilisticGenerator::randomWalk(int current, int step, int minValue, int maxValue)
{
    int direction = random.nextBool() ? 1 : -1;
    int newValue = current + (direction * random.nextInt(step + 1));
    return juce::jlimit(minValue, maxValue, newValue);
}

float ProbabilisticGenerator::randomWalkFloat(float current, float step, float minValue, float maxValue)
{
    float direction = random.nextBool() ? 1.0f : -1.0f;
    float newValue = current + (direction * random.nextFloat() * step);
    return juce::jlimit(minValue, maxValue, newValue);
}

float ProbabilisticGenerator::gaussianRandom(float mean, float stddev)
{
    // Box-Muller transform
    float u1 = random.nextFloat();
    float u2 = random.nextFloat();
    float z0 = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * juce::MathConstants<float>::pi * u2);
    return mean + z0 * stddev;
}

// ============================================================================
// Main Algorithmic Engine Implementation
// ============================================================================
AlgorithmicEngine::AlgorithmicEngine()
{
}

void AlgorithmicEngine::setGeneratorType(GeneratorType type)
{
    currentType = type;
}

std::vector<int> AlgorithmicEngine::generateNoteSequence(int length)
{
    std::vector<int> sequence;

    switch (currentType)
    {
        case Markov:
        {
            int order = markovChain.getOrder();

            if (noteHistory.size() < order)
            {
                // Initialize history
                for (int i = 0; i < length && noteHistory.size() < order; ++i)
                {
                    int note = 60 + (i % 12);
                    noteHistory.push_back(note);
                }
            }

            for (int i = 0; i < length; ++i)
            {
                std::vector<int> state;
                if (noteHistory.size() >= order)
                    state = std::vector<int>(noteHistory.end() - order, noteHistory.end());
                else
                    state = std::vector<int>(noteHistory.begin(), noteHistory.end());

                int note = markovChain.generate(state);
                sequence.push_back(note);
                noteHistory.push_back(note);

                if (noteHistory.size() > 100)
                    noteHistory.pop_front();
            }
            break;
        }

        case LSystem:
        {
            auto lstring = lSystem.iterate(3);
            sequence = lSystem.toMidiNotes(lstring, 60);
            if (sequence.size() > length)
                sequence.resize(length);
            break;
        }

        case CellularAutomatonType:
        {
            auto state = cellularAutomaton.step();
            for (size_t i = 0; i < std::min(state.size(), static_cast<size_t>(length)); ++i)
            {
                if (state[i])
                    sequence.push_back(60 + (i % 24));
                else
                    sequence.push_back(-1); // Rest
            }
            break;
        }

        case Probabilistic:
        default:
        {
            sequence = probabilistic.generateMelody(length, 48, 84);
            break;
        }
    }

    return sequence;
}

std::vector<bool> AlgorithmicEngine::generateRhythmSequence(int length)
{
    return probabilistic.generateRhythm(length, 0.5f, 4.0f);
}

std::vector<float> AlgorithmicEngine::generateVelocitySequence(int length)
{
    std::vector<float> velocities;
    for (int i = 0; i < length; ++i)
        velocities.push_back(probabilistic.generateVelocity());
    return velocities;
}
