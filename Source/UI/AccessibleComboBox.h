/*
  ==============================================================================
    AccessibleComboBox.h

    ComboBox whose accessibility value can select an item by its exact text.
    JUCE's stock handler is read-only, so System Events cannot set a popup
    without opening the menu.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AccessibleComboBox : public juce::ComboBox
{
public:
    using juce::ComboBox::ComboBox;

    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<Handler>(*this);
    }

private:
    class Handler : public juce::AccessibilityHandler
    {
    public:
        explicit Handler(AccessibleComboBox& combo)
            : juce::AccessibilityHandler(combo,
                                         juce::AccessibilityRole::comboBox,
                                         actionsFor(combo),
                                         { std::make_unique<ValueInterface>(combo) }),
              comboBox(combo)
        {
        }

        juce::AccessibleState getCurrentState() const override
        {
            auto state = juce::AccessibilityHandler::getCurrentState().withExpandable();
            return comboBox.isPopupActive() ? state.withExpanded() : state.withCollapsed();
        }

        juce::String getTitle() const override { return comboBox.getTitle(); }
        juce::String getHelp() const override { return comboBox.getTooltip(); }

    private:
        class ValueInterface : public juce::AccessibilityTextValueInterface
        {
        public:
            explicit ValueInterface(AccessibleComboBox& combo) : comboBox(combo) {}

            bool isReadOnly() const override { return false; }

            juce::String getCurrentValueAsString() const override
            {
                return comboBox.getText();
            }

            void setValueAsString(const juce::String& newValue) override
            {
                const auto wanted = newValue.trim();
                for (int i = 0; i < comboBox.getNumItems(); ++i)
                {
                    if (comboBox.getItemText(i) == wanted)
                    {
                        comboBox.setSelectedItemIndex(i, juce::sendNotificationSync);
                        return;
                    }
                }
            }

        private:
            AccessibleComboBox& comboBox;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueInterface)
        };

        static juce::AccessibilityActions actionsFor(AccessibleComboBox& combo)
        {
            return juce::AccessibilityActions()
                .addAction(juce::AccessibilityActionType::press, [&combo] { combo.showPopup(); })
                .addAction(juce::AccessibilityActionType::showMenu, [&combo] { combo.showPopup(); });
        }

        AccessibleComboBox& comboBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Handler)
    };
};
