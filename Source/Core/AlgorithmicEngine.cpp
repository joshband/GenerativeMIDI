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
MarkovChain::MarkovChain(int requestedOrder) : order(juce::jlimit(1, 5, requestedOrder))
{
    lookupScratch.reserve(static_cast<size_t>(this->order));
}

void MarkovChain::addTransition(const std::vector<int>& state, int nextValue, float probability)
{
    if (state.size() != static_cast<size_t>(order))
        return;

    transitionTable[state][nextValue] = probability;
}

void MarkovChain::learn(const std::vector<int>& sequence)
{
    const auto stateLength = static_cast<size_t>(order);
    if (sequence.size() <= stateLength)
        return;

    // Count transitions
    std::map<std::vector<int>, std::map<int, int>> counts;

    for (size_t i = 0; i <= sequence.size() - stateLength - 1; ++i)
    {
        const auto offset = static_cast<std::ptrdiff_t>(i);
        std::vector<int> state(sequence.begin() + offset,
                               sequence.begin() + offset + static_cast<std::ptrdiff_t>(stateLength));
        int next = sequence[i + stateLength];
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
            float probability = static_cast<float>(count) / static_cast<float>(total);
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
    // Fibonacci word. iterate() and the pattern view both read these rules.
    addRule('A', "AB");
    addRule('B', "A");
}

void LSystemEngine::setAxiom(const juce::String& ax)
{
    axiom = ax;
}

void LSystemEngine::addRule(juce::juce_wchar symbol, const juce::String& replacement, float probability)
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

namespace LSystemCatalog
{
    struct Rule
    {
        char symbol;
        const char* replacement;
    };

    struct Grammar
    {
        const char* label;
        const char* axiom;
        Rule rules[2];
        int ruleCount;
    };

    static const Grammar& grammarAt(int index)
    {
        static const Grammar grammars[] = {
            { "Fib", "A", { { 'A', "AB" }, { 'B', "A" } }, 2 },
            { "Thue", "A", { { 'A', "AB" }, { 'B', "BA" } }, 2 },
            { "Cantor", "A", { { 'A', "ABA" }, { 'B', "BBB" } }, 2 },
            { "Koch", "A", { { 'A', "A+B" }, { 'B', "A-B" } }, 2 },
        };

        const int clamped = juce::jlimit(0, kCount - 1, index);
        return grammars[clamped];
    }

    const char* name(int grammarIndex)
    {
        return grammarAt(grammarIndex).label;
    }

    void expand(int grammarIndex, int generations, char* dest, int capacity, int& outLength)
    {
        outLength = 0;
        if (dest == nullptr || capacity <= 1)
            return;

        const Grammar& grammar = grammarAt(grammarIndex);
        char buffers[2][kMaxSymbols];
        int length = 0;
        for (const char* symbol = grammar.axiom; *symbol != '\0' && length < kMaxSymbols - 1; ++symbol)
            buffers[0][length++] = *symbol;
        buffers[0][length] = '\0';

        int source = 0;
        const int steps = juce::jlimit(0, 8, generations);
        for (int generation = 0; generation < steps; ++generation)
        {
            const int destination = 1 - source;
            int written = 0;
            for (int i = 0; i < length && written < kMaxSymbols - 1; ++i)
            {
                const char* replacement = nullptr;
                for (int ruleIndex = 0; ruleIndex < grammar.ruleCount; ++ruleIndex)
                {
                    if (grammar.rules[ruleIndex].symbol == buffers[source][i])
                    {
                        replacement = grammar.rules[ruleIndex].replacement;
                        break;
                    }
                }

                if (replacement == nullptr)
                {
                    buffers[destination][written++] = buffers[source][i];
                    continue;
                }

                for (const char* symbol = replacement; *symbol != '\0' && written < kMaxSymbols - 1; ++symbol)
                    buffers[destination][written++] = *symbol;
            }

            buffers[destination][written] = '\0';
            length = written;
            source = destination;
        }

        const int copy = juce::jmin(length, capacity - 1);
        for (int i = 0; i < copy; ++i)
            dest[i] = buffers[source][i];
        dest[copy] = '\0';
        outLength = copy;
    }
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
CellularAutomaton::CellularAutomaton(int size)
    : cells(static_cast<size_t>(size), false), scratch(static_cast<size_t>(size), false)
{
    // A single center cell. All-off is a fixed point of rule 30, so a blank
    // row never moves.
    if (!cells.empty())
        cells[cells.size() / 2] = true;
    initialState = cells;
    publish();
}

void CellularAutomaton::setRule(int ruleNumber)
{
    rule = juce::jlimit(0, 255, ruleNumber);
}

void CellularAutomaton::seedCell(int index)
{
    if (cells.empty())
        return;

    const int onIndex = juce::jlimit(0, static_cast<int>(cells.size()) - 1, index);
    if (initialState.size() != cells.size())
        initialState.assign(cells.size(), false);

    for (size_t i = 0; i < cells.size(); ++i)
    {
        const bool on = static_cast<int>(i) == onIndex;
        cells[i] = on;
        initialState[i] = on;
    }

    publish();
}

void CellularAutomaton::setState(const std::vector<bool>& state)
{
    cells = state;
    scratch.assign(cells.size(), false);
    initialState = state;
    publish();
}

void CellularAutomaton::randomizeState(float density)
{
    for (size_t i = 0; i < cells.size(); ++i)
        cells[i] = random.nextFloat() < density;
    initialState = cells;
    publish();
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

    publish();
}

void CellularAutomaton::publish()
{
    uint64_t mask = 0;
    const int n = juce::jmin(64, static_cast<int>(cells.size()));
    for (int i = 0; i < n; ++i)
        if (cells[static_cast<size_t>(i)])
            mask |= (uint64_t { 1 } << static_cast<unsigned>(i));

    bits.store(mask, std::memory_order_relaxed);
    sizeBits.store(n, std::memory_order_relaxed);
    generation.fetch_add(1, std::memory_order_release);
}

int CellularAutomaton::copyCells(bool* dest, int destCap, int& countOut) const
{
    countOut = 0;
    if (dest == nullptr || destCap <= 0)
        return generation.load(std::memory_order_acquire);

    for (;;)
    {
        const int g1 = generation.load(std::memory_order_acquire);
        const uint64_t mask = bits.load(std::memory_order_acquire);
        const int n = juce::jmin(destCap, sizeBits.load(std::memory_order_acquire));
        const int g2 = generation.load(std::memory_order_acquire);
        if (g1 != g2)
            continue;

        for (int i = 0; i < n; ++i)
            dest[i] = (mask & (uint64_t { 1 } << static_cast<unsigned>(i))) != 0;
        countOut = n;
        return g1;
    }
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
    publish();
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
    std::vector<bool> rhythm(static_cast<size_t>(length), false);

    for (int i = 0; i < length; ++i)
    {
        // Apply grouping bias
        float bias = std::fmod(i, grouping) < 1.0f ? 1.5f : 1.0f;
        rhythm[static_cast<size_t>(i)] = random.nextFloat() < (density * bias);
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

void AlgorithmicEngine::setMarkovControls(int order, int stepSemitones, float surprise)
{
    const int clampedOrder = juce::jlimit(1, 4, order);
    if (clampedOrder != markovChain.getOrder())
        markovChain.setOrder(clampedOrder);

    markovStep = juce::jlimit(1, 12, stepSemitones);
    markovSurprise = juce::jlimit(0.0f, 1.0f, surprise);
}

void AlgorithmicEngine::setLSystemControls(int grammar, int generation, int interval)
{
    const int clampedGrammar = juce::jlimit(0, LSystemCatalog::kCount - 1, grammar);
    const int clampedGeneration = juce::jlimit(0, 6, generation);
    const int clampedInterval = juce::jlimit(1, 12, interval);
    if (clampedGrammar == lsystemGrammar
        && clampedGeneration == lsystemGeneration
        && clampedInterval == lsystemInterval
        && pitchMin == lsystemBuiltMin
        && pitchMax == lsystemBuiltMax
        && lsystemNoteCount > 0)
        return;

    lsystemGrammar = clampedGrammar;
    lsystemGeneration = clampedGeneration;
    lsystemInterval = clampedInterval;
    rebuildLSystemNotes();
}

void AlgorithmicEngine::setCellularControls(int rule, int seed, int listen)
{
    cellularAutomaton.setRule(rule);
    const int clampedSeed = juce::jlimit(0, 31, seed);
    if (clampedSeed != cellularSeed)
    {
        cellularSeed = clampedSeed;
        cellularAutomaton.seedCell(clampedSeed);
    }

    const int size = juce::jmax(1, cellularAutomaton.getSize());
    cellularListen = juce::jlimit(0, size - 1, listen);
}

void AlgorithmicEngine::rebuildLSystemNotes()
{
    char symbols[LSystemCatalog::kMaxSymbols];
    int length = 0;
    LSystemCatalog::expand(lsystemGrammar, lsystemGeneration, symbols, LSystemCatalog::kMaxSymbols, length);

    lsystemNoteCount = 0;
    lsystemCursor = 0;
    int cursor = pitchMin + juce::jmax(0, pitchMax - pitchMin) / 2;

    for (int i = 0; i < length && lsystemNoteCount < kLSystemNotes; ++i)
    {
        int emitted = -1;
        switch (symbols[i])
        {
            case 'A': emitted = cursor; break;
            case 'B': emitted = cursor + lsystemInterval; break;
            case 'C': emitted = cursor + lsystemInterval * 2; break;
            case 'D': emitted = cursor + lsystemInterval * 3; break;
            case '+': cursor = juce::jlimit(0, 127, cursor + 12); break;
            case '-': cursor = juce::jlimit(0, 127, cursor - 12); break;
            case '[': cursor = juce::jlimit(0, 127, cursor + 1); break;
            case ']': cursor = juce::jlimit(0, 127, cursor - 1); break;
            default: break;
        }

        if (emitted >= 0)
            lsystemNotes[lsystemNoteCount++] = juce::jlimit(pitchMin, pitchMax, emitted);
    }

    if (lsystemNoteCount == 0)
        lsystemNotes[lsystemNoteCount++] = juce::jlimit(pitchMin, pitchMax, cursor);

    lsystemBuiltMin = pitchMin;
    lsystemBuiltMax = pitchMax;
}

int AlgorithmicEngine::nextUntrainedMarkovNote()
{
    const int order = juce::jmax(1, markovChain.getOrder());
    const int span = juce::jmax(0, pitchMax - pitchMin);
    const int center = pitchMin + span / 2;
    const int last = historyCount > 0 ? getHistoryNoteFromNewest(0) : center;

    int note = last;
    if (markovSurprise > 0.0f && random.nextFloat() < markovSurprise)
    {
        note = pitchMin + (span > 0 ? random.nextInt(span + 1) : 0);
        markovRun = 0;
    }
    else
    {
        if (markovRun >= order)
        {
            markovDirection = -markovDirection;
            markovRun = 0;
        }

        note = last + markovDirection * juce::jmax(1, markovStep);
        if (note > pitchMax)
        {
            note = pitchMax;
            markovDirection = -1;
            markovRun = 0;
        }
        else if (note < pitchMin)
        {
            note = pitchMin;
            markovDirection = 1;
            markovRun = 0;
        }

        ++markovRun;
    }

    note = juce::jlimit(pitchMin, pitchMax, note);
    pushHistory(note);
    return note;
}

int AlgorithmicEngine::getHistoryNoteFromNewest(int age) const
{
    if (age < 0 || age >= historyCount)
        return -1;
    const int idx = (historyWrite - 1 - age + kHistoryCap) % kHistoryCap;
    return noteHistory[idx];
}

void AlgorithmicEngine::pushHistory(int note)
{
    noteHistory[historyWrite] = note;
    historyWrite = (historyWrite + 1) % kHistoryCap;
    if (historyCount < kHistoryCap)
        ++historyCount;
    historySerial.fetch_add(1, std::memory_order_release);
}

int AlgorithmicEngine::generateNextNote()
{
    switch (currentType)
    {
        case Markov:
        {
            if (!markovChain.hasTransitions())
                return nextUntrainedMarkovNote();

            const int order = markovChain.getOrder();
            const int fallback = juce::jlimit(pitchMin, pitchMax, 60);
            const int span = juce::jmax(0, pitchMax - pitchMin);

            if (markovSurprise > 0.0f && random.nextFloat() < markovSurprise)
            {
                const int jumped = pitchMin + (span > 0 ? random.nextInt(span + 1) : 0);
                pushHistory(jumped);
                return jumped;
            }

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
            if (lsystemNoteCount <= 0)
                rebuildLSystemNotes();

            const int note = lsystemNotes[lsystemCursor];
            lsystemCursor = (lsystemCursor + 1) % juce::jmax(1, lsystemNoteCount);
            return note;
        }

        case CellularAutomatonType:
        {
            cellularAutomaton.stepInPlace();
            const int size = cellularAutomaton.getSize();
            if (size <= 0)
                return -1;

            const int index = juce::jlimit(0, size - 1, cellularListen);
            if (!cellularAutomaton.getCell(index))
                return -1;

            // Listen gates the note. Pitch reads the live row with that cell
            // as the origin, so rule, seed, and listen move the pitch as the
            // automaton evolves. A fixed map of the listen index repeats one note.
            int weighted = 0;
            for (int i = 0; i < size; ++i)
                if (cellularAutomaton.getCell((index + i) % size))
                    weighted += i + 1;

            const int maxWeight = size * (size + 1) / 2;
            const int span = juce::jmax(0, pitchMax - pitchMin);
            return pitchMin + (weighted * span) / juce::jmax(1, maxWeight);
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
