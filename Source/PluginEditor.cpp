#include "PluginEditor.h"

InfiniteEditor::InfiniteEditor (InfiniteProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (780, 560);

    title.setFont (juce::Font (juce::FontOptions (32.f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, juce::Colours::white);
    subtitle.setFont (juce::Font (juce::FontOptions (16.f)));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xffff7a59));
    addAndMakeVisible (title);
    addAndMakeVisible (subtitle);

    category.addItemList ({ "Bass", "Lead", "Pad", "Pluck", "Keys", "Tout" }, 1);
    category.setSelectedId (currentCat() + 1, juce::dontSendNotification);
    category.onChange = [this]
    {
        proc.loadSeed (category.getSelectedId() - 1, currentSeed());
    };
    addAndMakeVisible (category);

    prevBtn.onClick = [this] { proc.loadSeed (currentCat(), juce::jmax (0, currentSeed() - 1)); };
    nextBtn.onClick = [this] { proc.loadSeed (currentCat(), currentSeed() + 1); };
    genBtn.onClick  = [this] { proc.loadSeed (currentCat(), juce::Random::getSystemRandom().nextInt (1000000)); };
    saveBtn.onClick = [this] { proc.saveFavorite(); refreshFavorites(); };
    for (auto* b : { &prevBtn, &nextBtn, &genBtn, &saveBtn }) addAndMakeVisible (b);

    favorites.setTextWhenNothingSelected ("Mes favoris");
    favorites.onChange = [this]
    {
        const int id = favorites.getSelectedId();
        if (id > 0)
        {
            proc.loadFavorite (id - 1);
            favorites.setSelectedId (0, juce::dontSendNotification);
        }
    };
    addAndMakeVisible (favorites);
    refreshFavorites();

    juce::StringArray waves { "Sine", "Saw", "Square", "Tri" };
    w1.addItemList (waves, 1);  w2.addItemList (waves, 1);
    w1a = std::make_unique<ComboAtt> (proc.apvts, "osc1Wave", w1);
    w2a = std::make_unique<ComboAtt> (proc.apvts, "osc2Wave", w2);
    w1l.setText ("Osc 1", juce::dontSendNotification);
    w2l.setText ("Osc 2", juce::dontSendNotification);
    for (auto* l : { &w1l, &w2l }) { l->setColour (juce::Label::textColourId, juce::Colours::lightgrey); addAndMakeVisible (l); }
    addAndMakeVisible (w1);  addAndMakeVisible (w2);

    for (auto& pi : floatParams())
    {
        Knob k;
        k.slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
        k.slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
        k.slider->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffff7a59));
        k.label = std::make_unique<juce::Label>();
        k.label->setText (pi.name, juce::dontSendNotification);
        k.label->setJustificationType (juce::Justification::centred);
        k.label->setFont (juce::Font (juce::FontOptions (12.f)));
        k.label->setColour (juce::Label::textColourId, juce::Colours::lightgrey);
        k.att = std::make_unique<SliderAtt> (proc.apvts, pi.id, *k.slider);
        addAndMakeVisible (*k.slider);
        addAndMakeVisible (*k.label);
        knobs.push_back (std::move (k));
    }

    startTimerHz (5);
    timerCallback();
}

InfiniteEditor::~InfiniteEditor() { stopTimer(); }

int InfiniteEditor::currentSeed() const { return (int) proc.apvts.state.getProperty ("seed", 1); }
int InfiniteEditor::currentCat()  const { return (int) proc.apvts.state.getProperty ("catSel", 5); }

void InfiniteEditor::refreshFavorites()
{
    favorites.clear (juce::dontSendNotification);
    auto names = proc.getFavoriteNames();
    for (int i = 0; i < names.size(); ++i) favorites.addItem (names[i], i + 1);
}

void InfiniteEditor::timerCallback()
{
    auto t = proc.getTitle(), i = proc.getInfo();
    if (t != lastTitle) { lastTitle = t; title.setText (t, juce::dontSendNotification); }
    if (i != lastInfo)  { lastInfo = i;  subtitle.setText (i, juce::dontSendNotification); }
    const int c = currentCat() + 1;
    if (category.getSelectedId() != c) category.setSelectedId (c, juce::dontSendNotification);
}

void InfiniteEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff15131a));
    g.setColour (juce::Colour (0xff2a2733));
    g.drawHorizontalLine (92, 16.f, (float) getWidth() - 16.f);
}

void InfiniteEditor::resized()
{
    auto r = getLocalBounds().reduced (16);

    auto top = r.removeFromTop (76);
    category.setBounds (top.removeFromRight (130).withHeight (28));
    title.setBounds (top.removeFromTop (44));
    subtitle.setBounds (top);

    r.removeFromTop (14);
    auto row = r.removeFromTop (32);
    prevBtn.setBounds (row.removeFromLeft (44));  row.removeFromLeft (6);
    nextBtn.setBounds (row.removeFromLeft (44));  row.removeFromLeft (6);
    genBtn.setBounds  (row.removeFromLeft (110)); row.removeFromLeft (6);
    saveBtn.setBounds (row.removeFromLeft (140)); row.removeFromLeft (10);
    favorites.setBounds (row);

    r.removeFromTop (14);
    auto wr = r.removeFromTop (28);
    w1l.setBounds (wr.removeFromLeft (50));  w1.setBounds (wr.removeFromLeft (130));
    wr.removeFromLeft (24);
    w2l.setBounds (wr.removeFromLeft (50));  w2.setBounds (wr.removeFromLeft (130));

    r.removeFromTop (12);
    const int cols = 7;
    const int rows = ((int) knobs.size() + cols - 1) / cols;
    const int cw = r.getWidth() / cols, ch = r.getHeight() / rows;
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int x = r.getX() + ((int) i % cols) * cw;
        const int y = r.getY() + ((int) i / cols) * ch;
        knobs[i].label->setBounds (x, y, cw, 16);
        knobs[i].slider->setBounds (x, y + 16, cw, ch - 16);
    }
}
