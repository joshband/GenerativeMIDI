#include "PresetBrowser.h"
#include "CustomLookAndFeel.h"

PresetBrowser::PresetBrowser(PresetManager& manager)
    : presetManager(manager),
      currentCategory("All")
{
    // Preset list
    addAndMakeVisible(presetListBox);
    presetListBox.setModel(this);
    presetListBox.setRowHeight(30);
    presetListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff1a1a1a));
    presetListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff3a3a3a));

    // Category selector
    addAndMakeVisible(categorySelector);
    categorySelector.addItem("All", 1);
    categorySelector.setSelectedId(1);
    categorySelector.onChange = [this]()
    {
        setCategory(categorySelector.getText());
    };

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
        const auto& preset = presetManager.getPreset(presetIndex);

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

    // Labels
    addAndMakeVisible(presetNameLabel);
    presetNameLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    presetNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff));
    presetNameLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(presetInfoLabel);
    presetInfoLabel.setFont(juce::Font(12.0f));
    presetInfoLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    presetInfoLabel.setJustificationType(juce::Justification::centredLeft);

    // Initialize
    refreshPresetList();
    updatePresetInfo();

    // Start timer for auto-refresh (check every 2 seconds)
    startTimer(2000);
}

PresetBrowser::~PresetBrowser()
{
    stopTimer();
}

void PresetBrowser::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0f0f0f));

    // Title
    g.setColour(juce::Colour(0xff00d4ff));
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("Preset Browser", 10, 5, getWidth() - 20, 30, juce::Justification::centred);

    // Section dividers
    g.setColour(juce::Colour(0xff3a3a3a));
    g.drawLine(10.0f, 40.0f, static_cast<float>(getWidth() - 10), 40.0f, 1.0f);
}

void PresetBrowser::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    bounds.removeFromTop(45); // Title area

    // Category selector
    auto categoryArea = bounds.removeFromTop(30);
    categorySelector.setBounds(categoryArea.reduced(5));
    bounds.removeFromTop(5);

    // Navigation buttons
    auto navArea = bounds.removeFromTop(30);
    int buttonWidth = 40;
    prevButton.setBounds(navArea.removeFromLeft(buttonWidth));
    navArea.removeFromLeft(5);
    nextButton.setBounds(navArea.removeFromLeft(buttonWidth));
    navArea.removeFromLeft(10);
    presetNameLabel.setBounds(navArea);
    bounds.removeFromTop(5);

    // Preset info
    auto infoArea = bounds.removeFromTop(40);
    presetInfoLabel.setBounds(infoArea);
    bounds.removeFromTop(5);

    // Preset list (takes remaining space)
    auto listHeight = bounds.getHeight() - 45; // Leave room for buttons
    presetListBox.setBounds(bounds.removeFromTop(listHeight));
    bounds.removeFromTop(5);

    // Bottom buttons
    auto buttonArea = bounds.removeFromTop(35);
    int btnWidth = (buttonArea.getWidth() - 15) / 4;
    saveButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(5);
    deleteButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(5);
    importButton.setBounds(buttonArea.removeFromLeft(btnWidth));
    buttonArea.removeFromLeft(5);
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

    // Background
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff2a4a5a));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colour(0xff1a1a1a));
    else
        g.fillAll(juce::Colour(0xff151515));

    // Factory/User indicator
    g.setColour(preset.isFactory ? juce::Colour(0xffffaa00) : juce::Colour(0xff00d4ff));
    g.fillRect(0, 0, 4, height);

    // Preset name
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(14.0f, preset.isFactory ? juce::Font::bold : juce::Font::plain));
    g.drawText(preset.name, 10, 0, width - 120, height, juce::Justification::centredLeft);

    // Category tag
    g.setColour(juce::Colour(0xff666666));
    g.setFont(juce::Font(11.0f));
    g.drawText(preset.category, width - 110, 0, 100, height, juce::Justification::centredRight);
}

void PresetBrowser::listBoxItemClicked(int row, const juce::MouseEvent&)
{
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
    dialog->setSize(400, 300);

    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(dialog);
    options.dialogTitle = "Save Preset";
    options.componentToCentreAround = this;
    options.dialogBackgroundColour = juce::Colour(0xff1a1a1a);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;

    auto* window = options.launchAsync();

    if (window != nullptr)
    {
        window->setUsingNativeTitleBar(true);
    }
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
    int currentFilteredCount = filteredPresetIndices.size();

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
    setSize(400, 300);

    // Name
    addAndMakeVisible(nameLabel);
    nameLabel.setText("Preset Name:", juce::dontSendNotification);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(nameEditor);
    nameEditor.setMultiLine(false);
    nameEditor.setReturnKeyStartsNewLine(false);

    // Author
    addAndMakeVisible(authorLabel);
    authorLabel.setText("Author:", juce::dontSendNotification);
    authorLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(authorEditor);
    authorEditor.setMultiLine(false);
    authorEditor.setText(juce::SystemStats::getFullUserName());

    // Category
    addAndMakeVisible(categoryLabel);
    categoryLabel.setText("Category:", juce::dontSendNotification);
    categoryLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(categoryBox);
    categoryBox.setEditableText(true);
    categoryBox.addItem("Euclidean", 1);
    categoryBox.addItem("Polyrhythm", 2);
    categoryBox.addItem("Algorithmic", 3);
    categoryBox.addItem("Rhythmic", 4);
    categoryBox.addItem("Ambient", 5);
    categoryBox.addItem("Experimental", 6);
    categoryBox.setSelectedId(1);

    // Description
    addAndMakeVisible(descriptionLabel);
    descriptionLabel.setText("Description:", juce::dontSendNotification);
    descriptionLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    addAndMakeVisible(descriptionEditor);
    descriptionEditor.setMultiLine(true);
    descriptionEditor.setReturnKeyStartsNewLine(true);
}

void PresetSaveDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void PresetSaveDialog::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Name
    nameLabel.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(5);
    nameEditor.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(15);

    // Author
    authorLabel.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(5);
    authorEditor.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(15);

    // Category
    categoryLabel.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(5);
    categoryBox.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(15);

    // Description
    descriptionLabel.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(5);
    descriptionEditor.setBounds(bounds);
}
