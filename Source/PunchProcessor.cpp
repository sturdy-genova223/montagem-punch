#include "PunchProcessor.h"
#include "PluginEditor.h"

PunchProcessor::PunchProcessor()
    : AudioProcessor(BusesProperties()
                          .withInput("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PunchProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "amount", "Amount", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));

    return { params.begin(), params.end() };
}

void PunchProcessor::prepareToPlay(double sampleRate, int)
{
    lastSampleRate = sampleRate;

    for (int ch = 0; ch < 2; ++ch)
    {
        fastEnv[ch] = 0.0f;
        slowEnv[ch] = 0.0f;
    }

    amountSmoothed.reset(sampleRate, 0.03);
    amountSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("amount"));
}

void PunchProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    amountSmoothed.setTargetValue(*apvts.getRawParameterValue("amount"));
    const float amount01 = amountSmoothed.skip(buffer.getNumSamples());

    const float fastAttackCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.002));
    const float fastReleaseCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.020));
    const float slowAttackCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.050));
    const float slowReleaseCoef = std::exp(-1.0f / (float)(lastSampleRate * 0.300));

    const float boostDb = juce::jmap(amount01, 0.0f, 1.0f, 0.0f, 6.0f);
    const float cutDb = juce::jmap(amount01, 0.0f, 1.0f, 0.0f, 4.0f);
    const float ratioMax = 1.8f;
    const float ceiling = juce::Decibels::decibelsToGain(-0.3f);

    float presenceForUI = 0.0f;

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float x = samples[i];
            const float ax = std::abs(x);

            const float fCoef = ax > fastEnv[ch] ? fastAttackCoef : fastReleaseCoef;
            fastEnv[ch] = fCoef * fastEnv[ch] + (1.0f - fCoef) * ax;

            const float sCoef = ax > slowEnv[ch] ? slowAttackCoef : slowReleaseCoef;
            slowEnv[ch] = sCoef * slowEnv[ch] + (1.0f - sCoef) * ax;

            const float ratio = fastEnv[ch] / (slowEnv[ch] + 1.0e-6f);
            const float presence = juce::jlimit(0.0f, 1.0f, (ratio - 1.0f) / (ratioMax - 1.0f));

            const float gainDb = presence * boostDb - (1.0f - presence) * cutDb;
            const float gain = juce::Decibels::decibelsToGain(gainDb);

            float y = x * gain;

            const float ay = std::abs(y);
            if (ay > ceiling)
            {
                const float over = (ay - ceiling) / (1.0f - ceiling);
                const float compressed = ceiling + (1.0f - ceiling) * std::tanh(over);
                y = (y < 0.0f ? -compressed : compressed);
            }

            samples[i] = y;

            if (ch == 0)
                presenceForUI = presence;
        }
    }

    lastTransientPresence.store(presenceForUI, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* PunchProcessor::createEditor()
{
    return new PunchEditor(*this);
}

void PunchProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
}

void PunchProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
