#pragma once
#include "PluginProcessor.h"
#include <vector>
class UltimateEditor final : public juce::AudioProcessorEditor {
public:
 explicit UltimateEditor(UltimateProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 struct Dial {juce::Slider slider;juce::Label label;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;};
 struct Choice {juce::ComboBox combo;juce::Label label;std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;};
 struct Toggle {juce::ToggleButton button;std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;};
 std::vector<std::unique_ptr<Dial>> dials;
 std::vector<std::unique_ptr<Choice>> choices;
 std::vector<std::unique_ptr<Toggle>> toggles;
 UltimateProcessor& processor;
 std::array<juce::TextButton,5> tabs;
 int activeTab=0;
 void updateTab();
 void updateEqModel();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UltimateEditor)
};
