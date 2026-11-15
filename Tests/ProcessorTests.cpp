#include "PluginProcessor.h"
#include "Presets.h"
#include <iostream>
#include <cstdlib>

void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}

int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceRuntime;
    VocalProcessor p;
    require(p.getParameters().size() == 14, "14 automatable parameters");
    require(p.getNumPrograms() == 1 + int(std::size(vocal::presets)), "factory bank exposed to host");
    for (int program = 1; program < p.getNumPrograms(); ++program)
    {
        p.setCurrentProgram(program);
        const auto expected = vocal::presets[program - 1].settings();
        const auto actual = p.settings();
        require(std::abs(actual.input - expected.input) < .001f && std::abs(actual.output - expected.output) < .001f, "preset maps input/output");
        for (size_t i = 0; i < 6; ++i)
        {
            require(std::abs(actual.amount[i] - expected.amount[i]) < .001f, "preset maps fader");
            require(actual.mode[i] == expected.mode[i], "preset maps color");
        }
        require(p.getCurrentProgram() == program, "host current program");
    }
    auto* bass = p.parameters.getParameter("bass");
    bass->setValueNotifyingHost(bass->convertTo0to1(5.f));
    auto* mode = p.parameters.getParameter("bassMode");
    mode->setValueNotifyingHost(mode->convertTo0to1(2.f));
    p.parameters.state.setProperty("presetName", "Roundtrip test", nullptr);
    juce::MemoryBlock state;
    p.getStateInformation(state);
    p.resetParameters();
    p.setStateInformation(state.getData(), int(state.getSize()));
    require(std::abs(p.parameters.getRawParameterValue("bass")->load() - 5) < .001f, "state restores fader");
    require(int(p.parameters.getRawParameterValue("bassMode")->load()) == 2, "state restores mode");
    require(p.parameters.state.getProperty("presetName").toString() == "Roundtrip test", "state restores preset label");
    const char junk[] = "invalid preset"; p.setStateInformation(junk, sizeof(junk));
    require(int(p.parameters.getRawParameterValue("bassMode")->load()) == 2, "invalid state ignored");

    for (int in : {1, 2})
    {
        for (int out : {1, 2})
        {
            juce::AudioProcessor::BusesLayout layout;
            layout.inputBuses.add(in == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
            layout.outputBuses.add(out == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
            if (in == 2 && out == 1) { require(!p.isBusesLayoutSupported(layout), "reject stereo to mono"); continue; }
            require(p.setBusesLayout(layout), "supported bus layout");
            p.resetParameters(); p.prepareToPlay(48000, 512);
            for (int size : {0, 1, 127, 512, 4096})
            {
                juce::AudioBuffer<float> buffer(out, size); buffer.clear();
                for (int n = 0; n < size; ++n) buffer.setSample(0, n, .1f);
                if (in == 2) for (int n = 0; n < size; ++n) buffer.setSample(1, n, -.2f);
                juce::MidiBuffer midi; p.processBlock(buffer, midi);
                for (int n = 0; n < size; ++n)
                {
                    require(std::abs(buffer.getSample(0, n) - .1f) < 1.e-6f, "dry passthrough");
                    if (out == 2) require(std::abs(buffer.getSample(1, n) - (in == 1 ? .1f : -.2f)) < 1.e-6f, "second channel routing");
                }
            }
        }
    }
    if (argc > 1)
    {
        p.setCurrentProgram(2);
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        require(editor != nullptr, "editor construction");
        const auto image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
        require(image.isValid() && image.getWidth() == editor->getWidth(), "editor rendering");
        juce::File file = juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]);
        juce::FileOutputStream stream(file);
        require(stream.openedOk(), "screenshot file");
        juce::PNGImageFormat format;
        require(format.writeImageToStream(image, stream), "PNG screenshot");
    }
    std::cout << "PASS: JUCE preset mapping, parameter/state roundtrip, invalid state, mono/stereo/mono-to-stereo and 0..4096 sample blocks\n";
}
