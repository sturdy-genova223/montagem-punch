#include "PluginEditor.h"

PunchEditor::PunchEditor(PunchProcessor& p)
    : juce::AudioProcessorEditor(&p), processorRef(p)
{
    setLookAndFeel(&lookAndFeel);

    titleLabel.setText("MONTAGEM PUNCH", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("TRANSIENT SHAPE  /  ONE-KNOB PUNCH", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    subtitleLabel.setColour(juce::Label::textColourId, PunchLookAndFeel::textDim);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::plain)));
    addAndMakeVisible(subtitleLabel);

    amountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    amountSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                                      juce::MathConstants<float>::pi * 2.8f, true);
    addAndMakeVisible(amountSlider);

    amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.apvts, "amount", amountSlider);

    amountValueLabel.setJustificationType(juce::Justification::centred);
    amountValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    amountValueLabel.setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    addAndMakeVisible(amountValueLabel);

    footerLabel.setText("AMOUNT", juce::dontSendNotification);
    footerLabel.setJustificationType(juce::Justification::centred);
    footerLabel.setColour(juce::Label::textColourId, PunchLookAndFeel::textDim);
    footerLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(footerLabel);

    amountSlider.onValueChange = [this] { updateValueLabel(); repaint(); };
    updateValueLabel();

    brandLabel.setText("@montagem.punch", juce::dontSendNotification);
    brandLabel.setJustificationType(juce::Justification::centredRight);
    brandLabel.setColour(juce::Label::textColourId, PunchLookAndFeel::textDim.withAlpha(0.5f));
    brandLabel.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::plain)));
    addAndMakeVisible(brandLabel);

    setResizable(false, false);
    setSize(480, 360);

    startTimerHz(30);
}

PunchEditor::~PunchEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void PunchEditor::updateValueLabel()
{
    const int pct = (int)std::round(amountSlider.getValue() * 100.0);
    amountValueLabel.setText(juce::String(pct) + "%", juce::dontSendNotification);
}

void PunchEditor::timerCallback()
{
    const float target = processorRef.lastTransientPresence.load(std::memory_order_relaxed);
    // Fast rise, slow fall so the meter reads as a punchy flash rather than
    // a jittery needle -- purely cosmetic smoothing, separate from the DSP.
    const float coef = target > displayedPresence ? 0.6f : 0.08f;
    displayedPresence += coef * (target - displayedPresence);
    repaint();
}

void PunchEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGradient(PunchLookAndFeel::bg.brighter(0.03f), bounds.getCentre(),
                                     PunchLookAndFeel::bg.darker(0.15f), bounds.getBottomLeft(), true);
    g.setGradientFill(bgGradient);
    g.fillAll();

    const float amount = (float)amountSlider.getValue();
    const auto colour = PunchLookAndFeel::red.interpolatedWith(PunchLookAndFeel::green, amount);

    // Live transient meter: a horizontal track that fills based on the
    // processor's real-time detector reading -- unlike the Finisher/Widener
    // decorations (which only reflect the knob position), this one reacts
    // to actual incoming audio, confirming the detector is working.
    auto knobArea = amountSlider.getBounds().toFloat();
    const float meterWidth = 220.0f;
    const float meterHeight = 6.0f;
    const float meterX = bounds.getCentreX() - meterWidth * 0.5f;
    const float meterY = knobArea.getBottom() + 14.0f;

    g.setColour(PunchLookAndFeel::bgLighter);
    g.fillRoundedRectangle(meterX, meterY, meterWidth, meterHeight, meterHeight * 0.5f);

    const float fillWidth = juce::jlimit(0.0f, meterWidth, meterWidth * displayedPresence);
    if (fillWidth > 0.5f)
    {
        g.setColour(colour);
        g.fillRoundedRectangle(meterX, meterY, fillWidth, meterHeight, meterHeight * 0.5f);
    }
}

void PunchEditor::resized()
{
    auto area = getLocalBounds().reduced(16);

    titleLabel.setBounds(area.removeFromTop(36));
    subtitleLabel.setBounds(area.removeFromTop(20));

    area.removeFromTop(8);
    brandLabel.setBounds(area.removeFromBottom(14));
    footerLabel.setBounds(area.removeFromBottom(18));
    amountValueLabel.setBounds(area.removeFromBottom(28));
    area.removeFromBottom(14); // clearance for the meter drawn in paint()

    const int knobSize = 180;
    juce::Rectangle<int> knobArea(0, 0, knobSize, knobSize);
    knobArea.setCentre(area.getCentre());
    amountSlider.setBounds(knobArea);
}
