#pragma once
#include "PunchLookAndFeel.h"
#include "PunchProcessor.h"

class PunchEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PunchEditor(PunchProcessor&);
    ~PunchEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PunchProcessor& processorRef;
    PunchLookAndFeel lookAndFeel;

    juce::Slider amountSlider;
    juce::Label amountValueLabel;
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label footerLabel;
    juce::Label brandLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;

    // Smoothed copy of the processor's live transient-presence reading, so
    // the meter animates instead of jittering sample-block to sample-block.
    float displayedPresence = 0.0f;

    void updateValueLabel();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PunchEditor)
};
