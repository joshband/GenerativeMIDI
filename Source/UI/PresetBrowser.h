#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Core/PresetManager.h"

/**
 * @class PresetBrowser
 * @brief UI component for browsing, loading, and managing presets
 *
 * Features:
 * - Category filtering
 * - Preset list display
 * - Save/Load/Delete buttons
 * - Preset information display
 * - Keyboard navigation (arrow keys)
 */
class PresetBrowser : public juce::Component,
                      public juce::ListBoxModel,
                      private juce::Timer
{
public:
    PresetBrowser(PresetManager& manager);
    ~PresetBrowser() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g,
                         int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    // Preset operations
    void refreshPresetList();
    void showSavePresetDialog();
    void showDeleteConfirmation();

    // Category filtering
    void setCategory(const juce::String& category);

private:
    PresetManager& presetManager;

    // UI Components
    juce::ListBox presetListBox;
    juce::ComboBox categorySelector;
    juce::TextButton saveButton;
    juce::TextButton deleteButton;
    juce::TextButton importButton;
    juce::TextButton exportButton;
    juce::TextButton prevButton;
    juce::TextButton nextButton;
    juce::Label presetNameLabel;
    juce::Label presetInfoLabel;

    // Current state
    juce::String currentCategory;
    juce::Array<int> filteredPresetIndices;

    // Timer for auto-refresh
    void timerCallback() override;

    // Helper functions
    void updatePresetInfo();
    void filterPresetsByCategory();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};

/**
 * @class PresetSaveDialog
 * @brief Dialog for saving new presets with metadata
 */
class PresetSaveDialog : public juce::Component
{
public:
    PresetSaveDialog();

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::String getPresetName() const { return nameEditor.getText(); }
    juce::String getAuthor() const { return authorEditor.getText(); }
    juce::String getCategory() const { return categoryBox.getText(); }
    juce::String getDescription() const { return descriptionEditor.getText(); }

private:
    juce::Label nameLabel;
    juce::TextEditor nameEditor;
    juce::Label authorLabel;
    juce::TextEditor authorEditor;
    juce::Label categoryLabel;
    juce::ComboBox categoryBox;
    juce::Label descriptionLabel;
    juce::TextEditor descriptionEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetSaveDialog)
};
