#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// One-button transient shaper, third piece of the Montagem chain (Finisher
// for drive/loudness, Widener for stereo space, this for punch/dynamics).
// A single "Amount" macro boosts the attack portion of each hit and pulls
// down the sustain portion, widening the contrast between transient and
// body -- classic transient-designer technique, driven by one knob instead
// of separate attack/sustain controls.
//
// NOTE: this is the public / portfolio version. The tuned constants
// (envelope time constants, boost/cut curve, detector sensitivity) used in
// the shipped build are simplified/omitted here -- this file demonstrates
// the JUCE architecture, not the production calibration.
class PunchProcessor : public juce::AudioProcessor
{
public:
    PunchProcessor();
    ~PunchProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Montagem Punch"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // Exposed for the UI: 0 (pure sustain) .. 1 (mid-transient), read each
    // block so the editor can visualise the detector reacting to real audio.
    std::atomic<float> lastTransientPresence { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Fast envelope tracks the instantaneous leading edge of a hit; slow
    // envelope tracks the settled body. Comparing the two (fast > slow)
    // identifies "we're in a transient right now" without any lookahead.
    float fastEnv[2] = { 0.0f, 0.0f };
    float slowEnv[2] = { 0.0f, 0.0f };

    double lastSampleRate = 44100.0;

    juce::SmoothedValue<float> amountSmoothed;
};
