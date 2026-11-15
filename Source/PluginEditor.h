#pragma once
#include "PluginProcessor.h"

class ArtworkLook final : public juce::LookAndFeel_V4
{
public:
    explicit ArtworkLook(juce::Image source) : artwork(source) {}
    int getSliderThumbRadius(juce::Slider&) override { return 0; }
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override
    {
        juce::Slider::SliderLayout layout;
        const float scale = float(slider.getHeight()) / 410.f;
        layout.sliderBounds = {0, juce::roundToInt(46 * scale), slider.getWidth(), juce::roundToInt(314 * scale)};
        return layout;
    }
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override;
private:
    juce::Image artwork;
};

class Hotspot final : public juce::Button
{
public:
    explicit Hotspot(const juce::String& name) : Button(name) {}
    void paintButton(juce::Graphics&, bool, bool) override {}
};

class VocalEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VocalEditor(VocalProcessor&);
    ~VocalEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { repaint(); }
    void showPresets();
    void applyPreset(int index);
    void filePreset(bool save);
    void toggleAB();
    void paintMeter(juce::Graphics&, float x, const std::array<std::atomic<float>, 2>&, bool);
    VocalProcessor& processor;
    juce::Image artwork;
    ArtworkLook look;
    std::array<juce::Slider, 8> faders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> attachments;
    std::array<std::unique_ptr<Hotspot>, 6> modes;
    Hotspot setup{"Preset menu"}, load{"Load preset"}, save{"Save preset"}, ab{"Compare A/B"},
            undo{"Undo"}, redo{"Redo"}, inputMeter{"Reset input clip"}, outputMeter{"Reset output clip"};
    juce::TooltipWindow tooltips{this, 600};
    std::unique_ptr<juce::FileChooser> chooser;
    juce::MemoryBlock otherState;
    bool isB = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalEditor)
};
