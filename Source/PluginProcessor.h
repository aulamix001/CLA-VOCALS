#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "VocalDSP.h"

class VocalProcessor final : public juce::AudioProcessor
{
public:
    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;
    VocalProcessor();
    const juce::String getName() const override { return "Vocal Suite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8; }
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    vocal::Settings settings() const;
    void resetParameters();
    static constexpr const char* faderIDs[] {"input", "bass", "treble", "compress", "reverb", "delay", "pitch", "output"};
    static constexpr const char* modeIDs[] {"bassMode", "trebleMode", "compressMode", "reverbMode", "delayMode", "pitchMode"};
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState parameters;
    std::array<std::atomic<float>, 2> inputPeak{}, outputPeak{};
    std::atomic<bool> inputClip{false}, outputClip{false};
private:
    void copyMonoToStereo(juce::AudioBuffer<float>&);
    vocal::Engine engine;
    std::array<std::atomic<float>*, 8> faders{};
    std::array<std::atomic<float>*, 6> modes{};
    std::atomic<int> currentProgram{0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalProcessor)
};
