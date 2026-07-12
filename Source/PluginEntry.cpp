#include "PunchProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PunchProcessor();
}
