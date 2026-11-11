#pragma once
#include "PluginProcessor.h"

class InfiniteEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit InfiniteEditor (InfiniteProcessor&);
    ~InfiniteEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct Knob
    {
        std::unique_ptr<juce::Slider> slider;
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<SliderAtt> att;
    };

    void timerCallback() override;
    void refreshFavorites();
    int  currentSeed() const;
    int  currentCat() const;

    InfiniteProcessor& proc;

    juce::Label title, subtitle, w1l, w2l;
    juce::ComboBox category, favorites, w1, w2;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, genBtn { "Generer" }, saveBtn { "Sauver en favori" };
    std::vector<Knob> knobs;
    std::unique_ptr<ComboAtt> w1a, w2a;
    juce::String lastTitle, lastInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfiniteEditor)
};
