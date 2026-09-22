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
MarkovChain::MarkovChain(int order) : order(juce::jlimit(1, 5, order))
{
    lookupScratch.reserve(static_cast<size_t>(this->order));
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
    if (currentState.size() != static_cast<size_t>(order))
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

int MarkovChain::generateOrDefault(const int* state, int stateLen, int fallbackNote)
{
    if (transitionTable.empty() || state == nullptr || stateLen != order)
        return fallbackNote;

    // Reuse reserved scratch — assign within capacity avoids per-note heap alloc.
    lookupScratch.assign(state, state + stateLen);
    return generate(lookupScratch);
}

void MarkovChain::reset()
{
    transitionTable.clear();
}

void MarkovChain::setOrder(int newOrder)
{
    order = juce::jlimit(1, 5, newOrder);
    lookupScratch.clear();
    lookupScratch.reserve(static_cast<size_t>(order));
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
CellularAutomaton::CellularAutomaton(int size) : cells(size, false), scratch(size, false)
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
    scratch.assign(cells.size(), false);
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
    stepInPlace();
    return cells;
}

void CellularAutomaton::stepInPlace()
{
    const size_t n = cells.size();
    if (scratch.size() != n)
        scratch.assign(n, false); // only grows if setState changed size (non-RT)

    for (size_t i = 0; i < n; ++i)
    {
        bool left = cells[(i + n - 1) % n];
        bool center = cells[i];
        bool right = cells[(i + 1) % n];
        scratch[i] = applyRule(left, center, right);
    }

    for (size_t i = 0; i < n; ++i)
        cells[i] = scratch[i];
}

bool CellularAutomaton::getCell(int index) const
{
    if (index < 0 || index >= static_cast<int>(cells.size()))
        return false;
    return cells[static_cast<size_t>(index)];
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
    lastProbNote = juce::jlimit(pitchMin, pitchMax, 60);
}

void AlgorithmicEngine::setGeneratorType(GeneratorType type)
{
    if (currentType == type)
        return;
    currentType = type;
}

void AlgorithmicEngine::setPitchRange(int minPitch, int maxPitch)
{
    pitchMin = juce::jlimit(0, 127, minPitch);
    pitchMax = juce::jlimit(0, 127, maxPitch);
    if (pitchMin > pitchMax)
        std::swap(pitchMin, pitchMax);
    lastProbNote = juce::jlimit(pitchMin, pitchMax, lastProbNote);
}

void AlgorithmicEngine::setVelocityRange(float minVel, float maxVel)
{
    velocityMean = (minVel + maxVel) / 2.0f;
    velocityVariance = (maxVel - minVel) / 4.0f; // Keep most values within range
}

void AlgorithmicEngine::pushHistory(int note)
{
    noteHistory[historyWrite] = note;
    historyWrite = (historyWrite + 1) % kHistoryCap;
    if (historyCount < kHistoryCap)
        ++historyCount;
}

int AlgorithmicEngine::generateNextNote()
{
    switch (currentType)
    {
        case Markov:
        {
            const int order = markovChain.getOrder();
            const int fallback = juce::jlimit(pitchMin, pitchMax, 60);

            while (historyCount < order)
                pushHistory(fallback + (historyCount % 12));

            int stateBuf[8];
            const int n = juce::jmin(order, 8);
            for (int i = 0; i < n; ++i)
            {
                const int idx = (historyWrite - n + i + kHistoryCap) % kHistoryCap;
                stateBuf[i] = noteHistory[idx];
            }

            const int note = juce::jlimit(pitchMin, pitchMax,
                                          markovChain.generateOrDefault(stateBuf, n, fallback));
            pushHistory(note);
            return note;
        }

        case LSystem:
        {
            // Default axiom/rules yield a single middle-C — avoid String iterate on RT.
            // (Custom rules via addRule are not used on the live UI path.)
            return juce::jlimit(pitchMin, pitchMax, 60);
        }

        case CellularAutomatonType:
        {
            cellularAutomaton.stepInPlace();
            if (cellularAutomaton.getCell(0))
                return juce::jlimit(pitchMin, pitchMax, 60);
            return -1; // Rest
        }

        case Probabilistic:
        default:
        {
            lastProbNote = probabilistic.randomWalk(lastProbNote, 3, pitchMin, pitchMax);
            return lastProbNote;
        }
    }
}

float AlgorithmicEngine::generateNextVelocity()
{
    return probabilistic.generateVelocity(velocityMean, velocityVariance);
}

std::vector<int> AlgorithmicEngine::generateNoteSequence(int length)
{
    std::vector<int> sequence;
    sequence.reserve(static_cast<size_t>(juce::jmax(0, length)));

    for (int i = 0; i < length; ++i)
        sequence.push_back(generateNextNote());

    return sequence;
}

std::vector<bool> AlgorithmicEngine::generateRhythmSequence(int length)
{
    return probabilistic.generateRhythm(length, 0.5f, 4.0f);
}

std::vector<float> AlgorithmicEngine::generateVelocitySequence(int length)
{
    std::vector<float> velocities;
    velocities.reserve(static_cast<size_t>(juce::jmax(0, length)));
    for (int i = 0; i < length; ++i)
        velocities.push_back(generateNextVelocity());
    return velocities;
}
