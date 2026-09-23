#include "PresetBrowser.h"
#include "CustomLookAndFeel.h"

namespace
{
    void styleActionButton(juce::TextButton& b)
    {
        b.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN));
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED).darker(0.2f));
        b.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
        b.setColour(juce::TextButton::textColourOnId, juce::Colour(CustomLookAndFeel::AETHER_CYAN));
    }

    void styleEditorField(juce::TextEditor& e)
    {
        e.setColour(juce::TextEditor::backgroundColourId, juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN));
        e.setColour(juce::TextEditor::outlineColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
        e.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(CustomLookAndFeel::AETHER_CYAN));
        e.setColour(juce::TextEditor::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
        e.setColour(juce::TextEditor::highlightedTextColourId, juce::Colour(CustomLookAndFeel::ABYSS_NAVY));
        e.setColour(juce::TextEditor::highlightColourId, juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.45f));
    }

    void styleSectionLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.8f));
    }
}

PresetBrowser::PresetBrowser(PresetManager& manager)
    : presetManager(manager),
      currentCategory("All")
{
    // Preset list
    addAndMakeVisible(presetListBox);
    presetListBox.setModel(this);
#if JUCE_IOS
    presetListBox.setRowHeight(40);
#else
    presetListBox.setRowHeight(34);
#endif
    presetListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).darker(0.2f));
    presetListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));

    // Category selector
    addAndMakeVisible(categoryHeaderLabel);
    styleSectionLabel(categoryHeaderLabel, "CATEGORY");

    addAndMakeVisible(categorySelector);
    categorySelector.addItem("All", 1);
    categorySelector.setSelectedId(1);
    categorySelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN));
    categorySelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
    categorySelector.setColour(juce::ComboBox::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    categorySelector.setColour(juce::ComboBox::arrowColourId, juce::Colour(CustomLookAndFeel::AETHER_CYAN));
    categorySelector.onChange = [this]()
    {
        setCategory(categorySelector.getText());
    };

    addAndMakeVisible(libraryHeaderLabel);
    styleSectionLabel(libraryHeaderLabel, "LIBRARY");

    addAndMakeVisible(actionsHeaderLabel);
    styleSectionLabel(actionsHeaderLabel, "ACTIONS");

    // Buttons
    addAndMakeVisible(saveButton);
    saveButton.setButtonText("Save");
    saveButton.onClick = [this]() { showSavePresetDialog(); };

    addAndMakeVisible(deleteButton);
    deleteButton.setButtonText("Delete");
    deleteButton.onClick = [this]() { showDeleteConfirmation(); };

    addAndMakeVisible(importButton);
    importButton.setButtonText("Import");
    importButton.onClick = [this]()
    {
        auto chooser = std::make_shared<juce::FileChooser>("Import Preset", juce::File(), "*.gmpreset");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile() && presetManager.importPreset(file))
                {
                    refreshPresetList();
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                        "Success", "Preset imported successfully!");
                }
                else if (file != juce::File())
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                        "Error", "Failed to import preset.");
                }
            });
    };

    addAndMakeVisible(exportButton);
    exportButton.setButtonText("Export");
    exportButton.onClick = [this]()
    {
        int selectedRow = presetListBox.getSelectedRow();
        if (selectedRow < 0 || selectedRow >= filteredPresetIndices.size())
            return;

        int presetIndex = filteredPresetIndices[selectedRow];

        auto chooser = std::make_shared<juce::FileChooser>("Export Preset",
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
            "*.gmpreset");

        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser, presetIndex](const juce::FileChooser& fc)
            {
                auto file = fc.getResult().withFileExtension("gmpreset");
                if (file != juce::File() && presetManager.exportPreset(presetIndex, file))
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                        "Success", "Preset exported successfully!");
                }
                else if (file != juce::File())
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                        "Error", "Failed to export preset.");
                }
            });
    };

    addAndMakeVisible(prevButton);
    prevButton.setButtonText("<");
    prevButton.onClick = [this]()
    {
        presetManager.loadPreviousPreset();
        updatePresetInfo();
        refreshPresetList();
    };

    addAndMakeVisible(nextButton);
    nextButton.setButtonText(">");
    nextButton.onClick = [this]()
    {
        presetManager.loadNextPreset();
        updatePresetInfo();
        refreshPresetList();
    };

    styleActionButton(saveButton);
    styleActionButton(deleteButton);
    styleActionButton(importButton);
    styleActionButton(exportButton);
    styleActionButton(prevButton);
    styleActionButton(nextButton);

    // Labels
    addAndMakeVisible(presetNameLabel);
    presetNameLabel.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    presetNameLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    presetNameLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(presetInfoLabel);
    presetInfoLabel.setFont(juce::FontOptions(12.0f));
    presetInfoLabel.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    presetInfoLabel.setJustificationType(juce::Justification::centredLeft);

    setTitle("Preset Manager");
    presetListBox.setTitle("Preset List");
    categorySelector.setTitle("Preset Category");
    saveButton.setTitle("Save Preset");
    deleteButton.setTitle("Delete Preset");
    importButton.setTitle("Import Preset");
    exportButton.setTitle("Export Preset");
    prevButton.setTitle("Previous Preset");
    nextButton.setTitle("Next Preset");

    // Initialize
    refreshPresetList();
    updatePresetInfo();

    // Start timer for auto-refresh (check every 2 seconds)
    startTimer(2000);
}

PresetBrowser::~PresetBrowser()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PresetBrowser::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bg(
        juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).brighter(0.04f),
        bounds.getCentreX(), bounds.getY(),
        juce::Colour(CustomLookAndFeel::ABYSS_NAVY),
        bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillAll();

    // Outer brass frame
    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.85f));
    g.drawRoundedRectangle(bounds.reduced(3.0f), 8.0f, 2.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE).withAlpha(0.35f));
    g.drawRoundedRectangle(bounds.reduced(5.5f), 7.0f, 1.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.12f));
    g.drawRoundedRectangle(bounds.reduced(8.0f), 6.0f, 1.0f);

    // Title plate
    auto titlePlate = juce::Rectangle<float>(bounds.getX() + 18.0f, 10.0f,
                                             bounds.getWidth() - 36.0f, 28.0f);
    juce::ColourGradient plateGrad(
        juce::Colour(CustomLookAndFeel::BRASS_AGED).brighter(0.12f),
        titlePlate.getCentreX(), titlePlate.getY(),
        juce::Colour(CustomLookAndFeel::BRASS_AGED).darker(0.15f),
        titlePlate.getCentreX(), titlePlate.getBottom(), false);
    g.setGradientFill(plateGrad);
    g.fillRoundedRectangle(titlePlate, 4.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    g.setFont(juce::FontOptions(16.0f).withStyle("Bold"));
    g.drawText("PRESET MANAGER", titlePlate, juce::Justification::centred);

    // Library list frame
    if (!libraryBounds.isEmpty())
    {
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.55f));
        g.drawRoundedRectangle(libraryBounds.toFloat(), 5.0f, 1.2f);
    }

    // Category chip chrome
    if (!categoryBounds.isEmpty())
    {
        g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.4f));
        g.drawRoundedRectangle(categoryBounds.toFloat().expanded(2.0f), 4.0f, 1.0f);
    }
}

void PresetBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(14);
    bounds.removeFromTop(42); // Title plate

    // Category
    categoryHeaderLabel.setBounds(bounds.removeFromTop(16));
    auto categoryArea = bounds.removeFromTop(32);
    categoryBounds = categoryArea;
    categorySelector.setBounds(categoryArea.reduced(2, 2));
    bounds.removeFromTop(8);

    // Navigation + current name
    auto navArea = bounds.removeFromTop(34);
#if JUCE_IOS
    const int buttonWidth = 44;
#else
    const int buttonWidth = 40;
#endif
    prevButton.setBounds(navArea.removeFromLeft(buttonWidth));
    navArea.removeFromLeft(6);
    nextButton.setBounds(navArea.removeFromLeft(buttonWidth));
    navArea.removeFromLeft(10);
    presetNameLabel.setBounds(navArea);
    bounds.removeFromTop(6);

    // Info
    auto infoArea = bounds.removeFromTop(44);
    presetInfoLabel.setBounds(infoArea.reduced(2, 0));
    bounds.removeFromTop(6);

    // Library
    libraryHeaderLabel.setBounds(bounds.removeFromTop(16));
#if JUCE_IOS
    const int actionH = 44;
#else
    const int actionH = 38;
#endif
    actionsHeaderLabel.setBounds(bounds.removeFromBottom(16));
    auto buttonArea = bounds.removeFromBottom(actionH);
    bounds.removeFromBottom(6);

    libraryBounds = bounds;
    presetListBox.setBounds(bounds.reduced(3));

    // Bottom actions
    int btnWidth = (buttonArea.getWidth() - 18) / 4;
    saveButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(6);
    deleteButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(6);
    importButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(6);
    exportButton.setBounds(buttonArea);
}

int PresetBrowser::getNumRows()
{
    return filteredPresetIndices.size();
}

void PresetBrowser::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                     int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= filteredPresetIndices.size())
        return;

    int presetIndex = filteredPresetIndices[rowNumber];
    const auto& preset = presetManager.getPreset(presetIndex);

    auto row = juce::Rectangle<float>(0.0f, 0.0f,
                                      static_cast<float>(width),
                                      static_cast<float>(height));

    if (rowIsSelected)
    {
        juce::ColourGradient sel(
            juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.45f),
            row.getX(), row.getCentreY(),
            juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.18f),
            row.getRight(), row.getCentreY(), false);
        g.setGradientFill(sel);
        g.fillRect(row);

        g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.7f));
        g.fillRect(0.0f, 0.0f, 3.0f, static_cast<float>(height));
    }
    else if (rowNumber % 2 == 0)
    {
        g.setColour(juce::Colour(CustomLookAndFeel::ABYSS_NAVY).withAlpha(0.55f));
        g.fillRect(row);
    }
    else
    {
        g.setColour(juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).withAlpha(0.65f));
        g.fillRect(row);
    }

    // Factory / user rail
    g.setColour(preset.isFactory ? juce::Colour(CustomLookAndFeel::AMBER_TESLA)
                                 : juce::Colour(CustomLookAndFeel::BRASS_AGED));
    g.fillRect(rowIsSelected ? 3.0f : 0.0f, 0.0f, 4.0f, static_cast<float>(height));

    // Category chip
    auto chip = juce::Rectangle<float>(static_cast<float>(width - 108), 6.0f, 98.0f,
                                       static_cast<float>(height - 12));
    g.setColour(juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN).brighter(0.12f));
    g.fillRoundedRectangle(chip, 3.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.65f));
    g.drawRoundedRectangle(chip, 3.0f, 1.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    g.setFont(juce::FontOptions(10.5f));
    g.drawText(preset.category, chip.reduced(4.0f, 0.0f), juce::Justification::centred);

    // Preset name
    g.setColour(rowIsSelected ? juce::Colour(CustomLookAndFeel::AETHER_CYAN)
                              : juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    g.setFont(juce::Font(14.0f, preset.isFactory ? juce::Font::bold : juce::Font::plain));
    g.drawText(preset.name, 14, 0, width - 130, height, juce::Justification::centredLeft);
}

void PresetBrowser::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= filteredPresetIndices.size())
        return;

    // Single-click loads so editor header stays in sync with selection
    const int presetIndex = filteredPresetIndices[row];
    presetManager.loadPreset(presetIndex);
    updatePresetInfo();
}

void PresetBrowser::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= filteredPresetIndices.size())
        return;

    int presetIndex = filteredPresetIndices[row];
    presetManager.loadPreset(presetIndex);
    updatePresetInfo();
}

void PresetBrowser::refreshPresetList()
{
    // Update category list
    categorySelector.clear();
    categorySelector.addItem("All", 1);
    int itemId = 2;
    for (const auto& category : presetManager.getCategories())
    {
        categorySelector.addItem(category, itemId++);
    }

    // Select current category
    if (currentCategory == "All")
        categorySelector.setSelectedId(1);
    else
        categorySelector.setText(currentCategory);

    // Filter presets
    filterPresetsByCategory();

    // Update list
    presetListBox.updateContent();
    presetListBox.repaint();
}

void PresetBrowser::showSavePresetDialog()
{
    auto* dialog = new PresetSaveDialog();
    dialog->setSize(420, 320);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(dialog);
    options.dialogTitle = "Save Preset";
    options.componentToCentreAround = this;
    options.dialogBackgroundColour = juce::Colour(CustomLookAndFeel::ABYSS_NAVY);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;

    options.launchAsync();
}

void PresetBrowser::showDeleteConfirmation()
{
    int selectedRow = presetListBox.getSelectedRow();
    if (selectedRow < 0 || selectedRow >= filteredPresetIndices.size())
        return;

    int presetIndex = filteredPresetIndices[selectedRow];
    const auto& preset = presetManager.getPreset(presetIndex);

    if (preset.isFactory)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
            "Cannot Delete", "Factory presets cannot be deleted.");
        return;
    }

    bool confirmed = juce::NativeMessageBox::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Delete Preset",
        "Are you sure you want to delete preset '" + preset.name + "'?",
        this,
        nullptr
    );

    if (confirmed)
    {
        presetManager.deletePreset(presetIndex);
        refreshPresetList();
        updatePresetInfo();
    }
}

void PresetBrowser::setCategory(const juce::String& category)
{
    currentCategory = category;
    filterPresetsByCategory();
    presetListBox.updateContent();
}

void PresetBrowser::timerCallback()
{
    // Auto-refresh to catch external preset file changes
    int numPresets = presetManager.getNumPresets();

    // Only refresh if preset count changed
    static int lastPresetCount = 0;
    if (numPresets != lastPresetCount)
    {
        presetManager.scanUserPresets();
        refreshPresetList();
        lastPresetCount = numPresets;
    }
}

void PresetBrowser::updatePresetInfo()
{
    int currentIndex = presetManager.getCurrentPresetIndex();

    if (currentIndex >= 0 && currentIndex < presetManager.getNumPresets())
    {
        const auto& preset = presetManager.getPreset(currentIndex);
        presetNameLabel.setText(preset.name, juce::dontSendNotification);

        juce::String info;
        info << "Author: " << preset.author << "\n";
        info << "Description: " << preset.description;
        presetInfoLabel.setText(info, juce::dontSendNotification);

        // Select in list if visible
        for (int i = 0; i < filteredPresetIndices.size(); ++i)
        {
            if (filteredPresetIndices[i] == currentIndex)
            {
                presetListBox.selectRow(i);
                break;
            }
        }
    }
    else
    {
        presetNameLabel.setText("No Preset Loaded", juce::dontSendNotification);
        presetInfoLabel.setText("", juce::dontSendNotification);
    }
}

void PresetBrowser::filterPresetsByCategory()
{
    filteredPresetIndices.clear();

    for (int i = 0; i < presetManager.getNumPresets(); ++i)
    {
        const auto& preset = presetManager.getPreset(i);

        if (currentCategory == "All" || preset.category == currentCategory)
            filteredPresetIndices.add(i);
    }
}

// PresetSaveDialog Implementation

PresetSaveDialog::PresetSaveDialog()
{
    setSize(420, 320);

    auto styleFieldLabel = [](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions(13.0f).withStyle("Bold"));
        label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::GOLD_TEMPLE));
    };

    // Name
    addAndMakeVisible(nameLabel);
    styleFieldLabel(nameLabel, "Preset Name");

    addAndMakeVisible(nameEditor);
    nameEditor.setMultiLine(false);
    nameEditor.setReturnKeyStartsNewLine(false);
    styleEditorField(nameEditor);

    // Author
    addAndMakeVisible(authorLabel);
    styleFieldLabel(authorLabel, "Author");

    addAndMakeVisible(authorEditor);
    authorEditor.setMultiLine(false);
    authorEditor.setText(juce::SystemStats::getFullUserName());
    styleEditorField(authorEditor);

    // Category
    addAndMakeVisible(categoryLabel);
    styleFieldLabel(categoryLabel, "Category");

    addAndMakeVisible(categoryBox);
    categoryBox.setEditableText(true);
    categoryBox.addItem("Euclidean", 1);
    categoryBox.addItem("Polyrhythm", 2);
    categoryBox.addItem("Algorithmic", 3);
    categoryBox.addItem("Rhythmic", 4);
    categoryBox.addItem("Ambient", 5);
    categoryBox.addItem("Experimental", 6);
    categoryBox.setSelectedId(1);
    categoryBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(CustomLookAndFeel::STEEL_OBSIDIAN));
    categoryBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(CustomLookAndFeel::BRASS_AGED));
    categoryBox.setColour(juce::ComboBox::textColourId, juce::Colour(CustomLookAndFeel::COPPER_STEAM));
    categoryBox.setColour(juce::ComboBox::arrowColourId, juce::Colour(CustomLookAndFeel::AETHER_CYAN));

    // Description
    addAndMakeVisible(descriptionLabel);
    styleFieldLabel(descriptionLabel, "Description");

    addAndMakeVisible(descriptionEditor);
    descriptionEditor.setMultiLine(true);
    descriptionEditor.setReturnKeyStartsNewLine(true);
    styleEditorField(descriptionEditor);
}

void PresetSaveDialog::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(CustomLookAndFeel::ABYSS_NAVY));
    g.setColour(juce::Colour(CustomLookAndFeel::BRASS_AGED).withAlpha(0.7f));
    g.drawRoundedRectangle(bounds.reduced(4.0f), 6.0f, 1.5f);
    g.setColour(juce::Colour(CustomLookAndFeel::AETHER_CYAN).withAlpha(0.12f));
    g.drawRoundedRectangle(bounds.reduced(7.0f), 5.0f, 1.0f);
}

void PresetSaveDialog::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Name
    nameLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(4);
    nameEditor.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(12);

    // Author
    authorLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(4);
    authorEditor.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(12);

    // Category
    categoryLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(4);
    categoryBox.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(12);

    // Description
    descriptionLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(4);
    descriptionEditor.setBounds(bounds);
}
