#include "PluginEditor.h"
#include "Presets.h"
#include "BinaryData.h"

namespace
{
constexpr float centers[] {149, 433, 573, 711, 850, 990, 1127, 1411};
constexpr const char* colors[6][3] {
    {"SUB", "LOWER", "UPPER"}, {"BITE", "TOP", "ROOF"}, {"PUSH", "SPANK", "WALL"},
    {"TIGHT", "LARGE", "CHAMBER"}, {"SLAP", "EIGHT", "QUARTER"}, {"STEREO", "WIDE", "SPREADER"}};
}

void ArtworkLook::drawLinearSlider(juce::Graphics& g, int, int, int, int,
                                  float, float, float, juce::Slider::SliderStyle, juce::Slider& slider)
{
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(float(slider.getWidth()) / 98.f, float(slider.getHeight()) / 410.f));
    // Reconstruct only the original thumb's footprint from a clean strip of the
    // same textured rail. The surrounding panel and source PNG remain untouched.
    const int sourceX = int(slider.getProperties()["artworkCenter"]) - 49;
    const int oldTop = int(slider.getProperties()["artworkThumbY"]) - 465 - 63;
    g.drawImage(artwork, 0, oldTop, 98, 126, sourceX, 477, 98, 21);
    const float y = juce::jmap(float(slider.getValue()), -10.f, 10.f, 360.f, 46.f);
    juce::Path thumb;
    thumb.startNewSubPath(28, y - 59); thumb.lineTo(73, y - 59);
    thumb.lineTo(85, y - 36); thumb.lineTo(84, y + 35);
    thumb.lineTo(74, y + 55); thumb.lineTo(19, y + 55);
    thumb.lineTo(10, y + 32); thumb.lineTo(10, y - 36); thumb.closeSubPath();
    juce::DropShadow(juce::Colours::black.withAlpha(.7f), 7, {3, 5}).drawForPath(g, thumb);
    g.reduceClipRegion(thumb);
    g.drawImage(artwork, 10, int(y - 60), 79, 120, 109, 561, 79, 120);
    g.restoreState();
}

VocalEditor::VocalEditor(VocalProcessor& p)
    : AudioProcessorEditor(p), processor(p),
      artwork(juce::ImageCache::getFromMemory(BinaryData::clavocalsreference_png,
                                             BinaryData::clavocalsreference_pngSize)), look(artwork)
{
    for (size_t i = 0; i < faders.size(); ++i)
    {
        auto& slider = faders[i];
        constexpr int originalThumbY[] {620, 725, 589, 668, 620, 743, 563, 668};
        slider.getProperties().set("artworkCenter", int(centers[i]));
        slider.getProperties().set("artworkThumbY", originalThumbY[i]);
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel(&look);
        slider.setDoubleClickReturnValue(true, 0);
        slider.setPopupDisplayEnabled(true, true, this);
        slider.setName(VocalProcessor::faderIDs[i]);
        slider.setTooltip(slider.getName() + ": -10 to +10. Double-click resets to 0; Shift for fine adjustment.");
        addAndMakeVisible(slider);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            p.parameters, VocalProcessor::faderIDs[i], slider);
    }
    for (size_t i = 0; i < modes.size(); ++i)
    {
        modes[i] = std::make_unique<Hotspot>(juce::String(VocalProcessor::faderIDs[i + 1]) + " color");
        modes[i]->setTooltip("Cycle: Bypass / " + juce::String(colors[i][0]) + " / " + colors[i][1] + " / " + colors[i][2]);
        modes[i]->onClick = [this, i]
        {
            auto* param = processor.parameters.getParameter(VocalProcessor::modeIDs[i]);
            const auto next = (int(processor.parameters.getRawParameterValue(VocalProcessor::modeIDs[i])->load()) + 1) % 4;
            param->beginChangeGesture(); param->setValueNotifyingHost(param->convertTo0to1(float(next))); param->endChangeGesture();
            repaint();
        };
        addAndMakeVisible(*modes[i]);
    }
    for (auto* b : {&setup, &load, &save, &ab, &undo, &redo, &inputMeter, &outputMeter})
    {
        addAndMakeVisible(b); b->setTooltip(b->getName());
    }
    setup.onClick = [this] { showPresets(); };
    load.onClick = [this] { filePreset(false); };
    save.onClick = [this] { filePreset(true); };
    ab.onClick = [this] { toggleAB(); };
    undo.onClick = [this] { processor.undo.undo(); };
    redo.onClick = [this] { processor.undo.redo(); };
    inputMeter.onClick = [this] { processor.inputClip = false; };
    outputMeter.onClick = [this] { processor.outputClip = false; };
    processor.getStateInformation(otherState);
    setResizable(true, true);
    getConstrainer()->setFixedAspectRatio(1557.0 / 1010.0);
    setResizeLimits(779, 505, 1557, 1010);
    setSize(1090, 707);
    startTimerHz(30);
}

VocalEditor::~VocalEditor()
{
    stopTimer();
    for (auto& s : faders) s.setLookAndFeel(nullptr);
}

void VocalEditor::resized()
{
    const float sx = float(getWidth()) / 1557, sy = float(getHeight()) / 1010;
    auto place = [sx, sy](juce::Component& c, float x, float y, float w, float h)
    { c.setBounds(juce::Rectangle<float>(x * sx, y * sy, w * sx, h * sy).toNearestInt()); };
    for (size_t i = 0; i < faders.size(); ++i) place(faders[i], centers[i] - 49, 465, 98, 410);
    for (size_t i = 0; i < modes.size(); ++i) place(*modes[i], centers[i + 1] - 43, 354, 86, 55);
    place(setup, 507, 22, 706, 30); place(undo, 344, 22, 53, 30); place(redo, 404, 22, 53, 30);
    place(ab, 1220, 22, 78, 30); place(load, 1305, 22, 73, 30); place(save, 1385, 22, 76, 30);
    place(inputMeter, 229, 470, 82, 369); place(outputMeter, 1247, 470, 81, 369);
}

void VocalEditor::paintMeter(juce::Graphics& g, float x,
                            const std::array<std::atomic<float>, 2>& peaks, bool clipped)
{
    g.setColour(juce::Colour(0xff141a12)); g.fillRect(x, 482.f, 65.f, 346.f);
    for (int ch = 0; ch < 2; ++ch)
    {
        const float db = juce::Decibels::gainToDecibels(peaks[size_t(ch)].load(), -60.f);
        for (int cell = 0; cell < 30; ++cell)
        {
            const bool lit = db >= -60.f + float(cell) * 2;
            const auto color = cell > 27 ? juce::Colour(0xffec3e29) : juce::Colour(0xff91ed12);
            g.setColour(lit ? color : color.withMultipliedBrightness(.16f));
            g.fillRect(x + float(ch) * 43, 821.f - float(cell) * 11, 19.f, 8.f);
        }
    }
    g.setColour(clipped ? juce::Colours::red : juce::Colour(0xff652925));
    g.fillRect(x, 485.f, 19.f, 10.f); g.fillRect(x + 43, 485.f, 19.f, 10.f);
}

void VocalEditor::paint(juce::Graphics& g)
{
    g.addTransform(juce::AffineTransform::scale(float(getWidth()) / 1557, float(getHeight()) / 1010));
    g.drawImage(artwork, juce::Rectangle<float>(0, 0, 1557, 1010));
    for (size_t i = 0; i < 6; ++i)
    {
        const int selected = int(processor.parameters.getRawParameterValue(VocalProcessor::modeIDs[i])->load());
        const float x = centers[i + 1];
        g.setColour(juce::Colour(0xff242727)); g.fillRect(x - 62, 255.f, 124.f, 78.f);
        g.setFont(juce::Font(juce::FontOptions(18.f, juce::Font::bold | juce::Font::italic)));
        for (int row = 0; row < 3; ++row)
        {
            const int index = 3 - row;
            auto color = index == 1 ? juce::Colour(0xffa5dc31) : (index == 2 ? juce::Colour(0xff54c6ee) : juce::Colour(0xffe84843));
            g.setColour(selected == index ? color : juce::Colour(0xff737773));
            g.drawText(colors[i][index - 1], int(x - 62), 255 + row * 25, 124, 24, juce::Justification::centred);
        }
        if (selected == 0) { g.setColour(juce::Colours::black.withAlpha(.55f)); g.fillRect(x - 32, 364.f, 64.f, 36.f); }
    }
    paintMeter(g, 238, processor.inputPeak, processor.inputClip.load());
    paintMeter(g, 1256, processor.outputPeak, processor.outputClip.load());
    g.setColour(juce::Colours::black); g.fillRect(508, 24, 704, 25);
    g.setColour(juce::Colour(0xffb9bcba)); g.setFont(juce::FontOptions(17.f));
    g.drawText(juce::String(isB ? "B" : "A") + " : " + processor.parameters.state.getProperty("presetName", "Default").toString(),
               512, 23, 698, 28, juce::Justification::centred);
}

void VocalEditor::showPresets()
{
    juce::PopupMenu menu;
    menu.addItem(1, "Default / all modules bypassed");
    menu.addSeparator();
    juce::String current;
    juce::PopupMenu group;
    int id = 100;
    for (const auto& preset : vocal::presets)
    {
        if (current != preset.category)
        {
            if (current.isNotEmpty()) menu.addSubMenu(current, group);
            group.clear(); current = preset.category;
        }
        group.addItem(id++, preset.name);
    }
    if (current.isNotEmpty()) menu.addSubMenu(current, group);
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(setup),
        [safe = juce::Component::SafePointer<VocalEditor>(this)](int selected)
        {
            if (!safe) return;
            if (selected == 1) safe->processor.setCurrentProgram(0);
            else if (selected >= 100) safe->applyPreset(selected - 100);
        });
}

void VocalEditor::applyPreset(int index)
{
    if (index < 0 || index >= int(std::size(vocal::presets))) return;
    processor.setCurrentProgram(index + 1);
}

void VocalEditor::toggleAB()
{
    juce::MemoryBlock current;
    processor.getStateInformation(current);
    processor.setStateInformation(otherState.getData(), int(otherState.getSize()));
    otherState = std::move(current); isB = !isB;
}

void VocalEditor::filePreset(bool saving)
{
    chooser = std::make_unique<juce::FileChooser>(saving ? "Save Vocal Suite preset" : "Load Vocal Suite preset",
                                                juce::File{}, "*.vocalpreset");
    const auto chooserFlags = saving ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
                              : juce::FileBrowserComponent::openMode;
    chooser->launchAsync(chooserFlags | juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<VocalEditor>(this), saving](const juce::FileChooser& fc)
        {
            if (!safe) return;
            auto file = fc.getResult();
            if (file == juce::File{}) return;
            if (saving)
            {
                file = file.withFileExtension(".vocalpreset");
                juce::MemoryBlock data; safe->processor.getStateInformation(data);
                if (!file.replaceWithData(data.getData(), data.getSize()))
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset", "Could not save this file.");
            }
            else if (file.getSize() <= 1024 * 1024)
            {
                juce::MemoryBlock data;
                if (file.loadFileAsData(data)) safe->processor.setStateInformation(data.getData(), int(data.getSize()));
            }
        });
}
