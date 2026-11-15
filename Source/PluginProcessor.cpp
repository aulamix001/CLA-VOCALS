#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

int VocalProcessor::getNumPrograms() { return 1 + int(std::size(vocal::presets)); }

const juce::String VocalProcessor::getProgramName(int index)
{
    return index > 0 && index < getNumPrograms() ? vocal::presets[index - 1].name : "Default";
}

void VocalProcessor::setCurrentProgram(int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    const auto s = index == 0 ? vocal::Settings{} : vocal::presets[index - 1].settings();
    undo.beginNewTransaction(getProgramName(index));
    const auto assign = [this](const char* id, float value)
    {
        auto* p = parameters.getParameter(id);
        p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(value)); p->endChangeGesture();
    };
    assign("input", s.input); assign("output", s.output);
    for (size_t i = 0; i < 6; ++i)
    {
        assign(faderIDs[i + 1], s.amount[i]); assign(modeIDs[i], float(s.mode[i]));
    }
    parameters.state.setProperty("presetName", getProgramName(index), &undo);
    parameters.state.setProperty("program", index, &undo);
    currentProgram = index;
}

juce::AudioProcessorValueTreeState::ParameterLayout VocalProcessor::makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout result;
    const char* names[] {"Input sensitivity", "Bass", "Treble", "Compress", "Reverb", "Delay", "Pitch", "Output"};
    for (size_t k = 0; k < 8; ++k)
        result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{faderIDs[k], 1},
            names[k], juce::NormalisableRange<float>{-10, 10, .01f}, 0));
    const juce::StringArray choices[] {
        {"Bypass", "Sub", "Lower", "Upper"}, {"Bypass", "Bite", "Top", "Roof"},
        {"Bypass", "Push", "Spank", "Wall"}, {"Bypass", "Tight", "Large", "Chamber"},
        {"Bypass", "Slap", "Eight", "Quarter"}, {"Bypass", "Stereo", "Wide", "Spreader"}};
    for (size_t k = 0; k < 6; ++k)
        result.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{modeIDs[k], 1},
            juce::String(names[k + 1]) + " color", choices[k], 0));
    return result;
}

VocalProcessor::VocalProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, &undo, "VocalSuiteState", makeLayout())
{
    for (size_t i = 0; i < 8; ++i) faders[i] = parameters.getRawParameterValue(faderIDs[i]);
    for (size_t i = 0; i < 6; ++i) modes[i] = parameters.getRawParameterValue(modeIDs[i]);
}

bool VocalProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    const auto in = layout.getMainInputChannelSet(), out = layout.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() && (out == in || out == juce::AudioChannelSet::stereo()))
           || (in == juce::AudioChannelSet::stereo() && out == in);
}

vocal::Settings VocalProcessor::settings() const
{
    vocal::Settings s;
    s.input = faders.front()->load(); s.output = faders.back()->load();
    for (size_t i = 0; i < 6; ++i) { s.amount[i] = faders[i + 1]->load(); s.mode[i] = int(modes[i]->load()); }
    return s;
}

void VocalProcessor::prepareToPlay(double sr, int)
{
    engine.prepare(sr, settings());
    inputClip = false; outputClip = false;
    for (auto& p : inputPeak) p = 0;
    for (auto& p : outputPeak) p = 0;
    setLatencySamples(0); // All spatial modules retain a direct, undelayed signal.
}

void VocalProcessor::copyMonoToStereo(juce::AudioBuffer<float>& b)
{
    if (getTotalNumInputChannels() == 1 && b.getNumChannels() > 1)
        b.copyFrom(1, 0, b, 0, 0, b.getNumSamples());
}

void VocalProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals guard;
    copyMonoToStereo(b);
    const auto n = b.getNumSamples();
    if (n == 0) return;
    for (int c = 0; c < std::min(2, b.getNumChannels()); ++c)
    {
        const float peak = b.getMagnitude(c, 0, n) * vocal::gain(faders.front()->load());
        inputPeak[size_t(c)].store(std::max(peak, inputPeak[size_t(c)].load() * .94f));
        if (peak >= 1.f) inputClip = true;
    }
    auto s = settings();
    if (auto* head = getPlayHead())
        if (auto pos = head->getPosition())
            if (auto bpm = pos->getBpm()) s.bpm = *bpm;
    if (b.getNumChannels() >= 2)
        engine.process(b.getWritePointer(0), b.getWritePointer(1), n, s);
    else
    {
        // Fixed stack chunks permit mono operation without allocating in the callback.
        std::array<float, 256> right{};
        for (int offset = 0; offset < n; offset += int(right.size()))
        {
            const int length = std::min(int(right.size()), n - offset);
            std::copy_n(b.getReadPointer(0, offset), length, right.data());
            engine.process(b.getWritePointer(0, offset), right.data(), length, s);
            for (int k = 0; k < length; ++k) b.getWritePointer(0, offset)[k] = (b.getReadPointer(0, offset)[k] + right[size_t(k)]) * .5f;
        }
    }
    for (int c = 0; c < std::min(2, b.getNumChannels()); ++c)
    {
        const float peak = b.getMagnitude(c, 0, n);
        outputPeak[size_t(c)].store(std::max(peak, outputPeak[size_t(c)].load() * .94f));
        if (peak >= 1.f) outputClip = true;
    }
}

void VocalProcessor::processBlockBypassed(juce::AudioBuffer<float>& b, juce::MidiBuffer&)
{
    copyMonoToStereo(b);
}

void VocalProcessor::getStateInformation(juce::MemoryBlock& block)
{
    if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, block);
}

void VocalProcessor::setStateInformation(const void* data, int length)
{
    if (auto xml = getXmlFromBinary(data, length))
        if (xml->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
            currentProgram = juce::jlimit(0, getNumPrograms() - 1, int(parameters.state.getProperty("program", 0)));
        }
}

void VocalProcessor::resetParameters()
{
    for (auto* p : getParameters())
    {
        p->beginChangeGesture(); p->setValueNotifyingHost(p->getDefaultValue()); p->endChangeGesture();
    }
}

juce::AudioProcessorEditor* VocalProcessor::createEditor() { return new VocalEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VocalProcessor(); }
