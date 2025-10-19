/*
  ==============================================================================
    ModulationMatrix.h

    Routes modulation sources to parameter destinations
    Manages modulation connections and depth/amount

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ModulationSource.h"
#include <vector>
#include <memory>

// ============================================================================
// Modulation Connection
// ============================================================================
struct ModulationConnection
{
    int sourceIndex = -1;           // Index into modulation sources array
    juce::String parameterID;       // Target parameter ID
    float depth = 0.5f;             // Modulation depth/amount (0.0 - 1.0)
    bool enabled = true;

    bool isValid() const
    {
        return sourceIndex >= 0 && parameterID.isNotEmpty();
    }
};

// ============================================================================
// Modulation Matrix
// ============================================================================
class ModulationMatrix
{
public:
    ModulationMatrix()
    {
        // Create default modulation sources
        addSource(std::make_unique<LFOModulationSource>());
        addSource(std::make_unique<LFOModulationSource>());
        addSource(std::make_unique<RandomModulationSource>());
        addSource(std::make_unique<EnvelopeModulationSource>());
    }

    ~ModulationMatrix() = default;

    // Source management
    int addSource(std::unique_ptr<ModulationSource> source)
    {
        sources.push_back(std::move(source));
        return sources.size() - 1;
    }

    ModulationSource* getSource(int index)
    {
        if (index >= 0 && index < sources.size())
            return sources[index].get();
        return nullptr;
    }

    int getNumSources() const { return sources.size(); }

    // Connection management
    int addConnection(int sourceIndex, const juce::String& parameterID, float depth = 0.5f)
    {
        ModulationConnection conn;
        conn.sourceIndex = sourceIndex;
        conn.parameterID = parameterID;
        conn.depth = depth;
        conn.enabled = true;

        connections.push_back(conn);
        return connections.size() - 1;
    }

    void removeConnection(int connectionIndex)
    {
        if (connectionIndex >= 0 && connectionIndex < connections.size())
            connections.erase(connections.begin() + connectionIndex);
    }

    void removeConnectionsForParameter(const juce::String& parameterID)
    {
        connections.erase(
            std::remove_if(connections.begin(), connections.end(),
                [&parameterID](const ModulationConnection& conn) {
                    return conn.parameterID == parameterID;
                }),
            connections.end());
    }

    ModulationConnection* getConnection(int index)
    {
        if (index >= 0 && index < connections.size())
            return &connections[index];
        return nullptr;
    }

    int getNumConnections() const { return connections.size(); }

    // Get all connections (for iteration)
    const std::vector<ModulationConnection>& getConnections() const { return connections; }

    // Get all connections for a specific parameter
    std::vector<int> getConnectionsForParameter(const juce::String& parameterID) const
    {
        std::vector<int> indices;
        for (int i = 0; i < connections.size(); ++i)
        {
            if (connections[i].parameterID == parameterID && connections[i].enabled)
                indices.push_back(i);
        }
        return indices;
    }

    // Calculate total modulation value for a parameter
    float calculateModulation(const juce::String& parameterID) const
    {
        float totalModulation = 0.0f;

        for (const auto& conn : connections)
        {
            if (conn.parameterID == parameterID && conn.enabled && conn.isValid())
            {
                auto* source = const_cast<ModulationMatrix*>(this)->getSource(conn.sourceIndex);
                if (source && source->getEnabled())
                {
                    float modValue = source->getNormalizedValue();
                    totalModulation += modValue * conn.depth;
                }
            }
        }

        return juce::jlimit(-1.0f, 1.0f, totalModulation);
    }

    // Update all modulation sources
    void advance(double timeSeconds)
    {
        for (auto& source : sources)
        {
            if (source && source->getEnabled())
                source->advance(timeSeconds);
        }
    }

    // Reset all sources
    void reset()
    {
        for (auto& source : sources)
        {
            if (source)
                source->reset();
        }
    }

    // Trigger envelopes
    void triggerEnvelopes()
    {
        for (auto& source : sources)
        {
            if (source && source->getType() == ModulationSourceType::Envelope)
            {
                auto* env = dynamic_cast<EnvelopeModulationSource*>(source.get());
                if (env)
                    env->trigger();
            }
        }
    }

    // Serialization
    std::unique_ptr<juce::XmlElement> toXml() const
    {
        auto xml = std::make_unique<juce::XmlElement>("ModulationMatrix");

        // Save connections
        for (const auto& conn : connections)
        {
            auto connXml = xml->createNewChildElement("Connection");
            connXml->setAttribute("sourceIndex", conn.sourceIndex);
            connXml->setAttribute("parameterID", conn.parameterID);
            connXml->setAttribute("depth", conn.depth);
            connXml->setAttribute("enabled", conn.enabled);
        }

        return xml;
    }

    void fromXml(const juce::XmlElement& xml)
    {
        connections.clear();

        for (auto* connXml : xml.getChildWithTagNameIterator("Connection"))
        {
            ModulationConnection conn;
            conn.sourceIndex = connXml->getIntAttribute("sourceIndex", -1);
            conn.parameterID = connXml->getStringAttribute("parameterID");
            conn.depth = static_cast<float>(connXml->getDoubleAttribute("depth", 0.5));
            conn.enabled = connXml->getBoolAttribute("enabled", true);

            if (conn.isValid())
                connections.push_back(conn);
        }
    }

private:
    std::vector<std::unique_ptr<ModulationSource>> sources;
    std::vector<ModulationConnection> connections;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMatrix)
};
