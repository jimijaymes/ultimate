#include "PluginEditor.h"
namespace {const juce::Colour bg(0xff111820),panel(0xff222c35),ink(0xffe7e8e2),gold(0xffd5ad67),blue(0xff75c2cb);}
UltimateEditor::UltimateEditor(UltimateProcessor& p):AudioProcessorEditor(p),processor(p){
 struct Knob {const char* id;const char* title;};
 const Knob knobs[]={
 {"preDrive","TRANSFORMER DRIVE"},{"circuitDrive","CIRCUIT DRIVE"},{"character","CHARACTER"},{"bias","BIAS"},
 {"eqLow","LOW"},{"eqLowFreq","LOW FREQ"},{"eqLowMid","LOW MID"},{"eqLowMidFreq","LM FREQ"},{"eqHighMid","HIGH MID"},{"eqHighMidFreq","HM FREQ"},{"eqHigh","HIGH"},{"eqHighFreq","HIGH FREQ"},{"eqHPF","HPF"},
 {"optoPeak","OPTO REDUCTION"},{"optoGain","OPTO MAKEUP"},{"fetDrive","FET DRIVE"},{"fetAttack","FET ATTACK"},{"fetRelease","FET RELEASE"},{"sidechainHPF","SIDECHAIN HPF"},
 {"tapeDrive","TAPE DRIVE"},{"outputTransformerDrive","OUTPUT XFMR DRIVE"},
 {"input","INPUT"},{"output","OUTPUT"},{"masterDrive","GLOBAL DRIVE"},{"mix","MIX"},
 {"apiBand0","31 Hz"},{"apiBand1","63 Hz"},{"apiBand2","125 Hz"},{"apiBand3","250 Hz"},{"apiBand4","500 Hz"},{"apiBand5","1 kHz"},{"apiBand6","2 kHz"},{"apiBand7","4 kHz"},{"apiBand8","8 kHz"},{"apiBand9","16 kHz"}};
 for(const auto& def:knobs){auto k=std::make_unique<Dial>();k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,18);k->slider.setColour(juce::Slider::rotarySliderFillColourId,gold);k->slider.setColour(juce::Slider::textBoxTextColourId,ink);k->slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);addAndMakeVisible(k->slider);k->label.setText(def.title,juce::dontSendNotification);k->label.setJustificationType(juce::Justification::centred);k->label.setColour(juce::Label::textColourId,ink);addAndMakeVisible(k->label);k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,def.id,k->slider);dials.push_back(std::move(k));}
 struct Option {const char* id;const char* title;juce::StringArray items;};
 const Option options[]={
 {"preTransformer","INPUT TRANSFORMER",{"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"}},
 {"circuit","CIRCUIT",{"Off","Discrete Class A","Class AB","Op-Amp","Triode Tube","Pentode Tube"}},
 {"eqModel","EQ MODEL",{"Neve 1073","SSL G Series","API 560"}},
 {"optoRatio","OPTO RATIO",{"Compress 3:1","Limit 20:1"}},
 {"optoResponse","OPTO RESPONSE",{"Enhanced","Classic T4"}},
 {"optoRelease","OPTO RELEASE",{"LA-2A Slow","LA-3A Fast"}},
 {"fetRatio","FET RATIO",{"4:1","8:1","12:1","20:1","All Buttons"}},
 {"fetTiming","FET TIMING",{"1176 Classic","Extended"}},
 {"tape","TAPE MODEL",{"Off","Studer A80 15 IPS","Studer A80 30 IPS","Nakamichi Dragon Metal","Studer J37 Abbey Road 15 IPS","Budget Cassette Ferric","Budget Cassette Metal"}},
 {"outputTransformer","OUTPUT TRANSFORMER",{"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"}},
 {"oversampling","OVERSAMPLING",{"1x","2x","4x","8x"}}};
 for(const auto& def:options){auto k=std::make_unique<Choice>();k->label.setText(def.title,juce::dontSendNotification);k->label.setColour(juce::Label::textColourId,ink);addAndMakeVisible(k->label);for(int i=0;i<def.items.size();++i)k->combo.addItem(def.items[i],i+1);k->combo.setColour(juce::ComboBox::backgroundColourId,panel);k->combo.setColour(juce::ComboBox::textColourId,ink);addAndMakeVisible(k->combo);k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,def.id,k->combo);choices.push_back(std::move(k));}
 struct Switch {const char* id;const char* title;};
 const Switch switches[]={{"preOn","PREAMP ON"},{"eqOn","EQ ON"},{"dynOn","DYNAMICS ON"},{"tapeOn","TAPE / OUTPUT ON"},{"optoOn","OPTO ON"},{"fetOn","FET ON"},{"fetFirst","FET FIRST"}};
 for(const auto& def:switches){auto k=std::make_unique<Toggle>();k->button.setButtonText(def.title);k->button.setColour(juce::ToggleButton::textColourId,ink);addAndMakeVisible(k->button);k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,def.id,k->button);toggles.push_back(std::move(k));}
 setSize(1260,1190);
}
void UltimateEditor::paint(juce::Graphics& g){
 g.fillAll(bg);g.setColour(ink);g.setFont(juce::Font(juce::FontOptions(33.f,juce::Font::bold)));g.drawText("ULTIMATE  /  CHANNEL STRIP",26,14,1000,46,juce::Justification::centredLeft);
 const char* headings[]={"01  PREAMP / TRANSFORMER & CIRCUIT","02  THREE-MODEL EQUALISER","03  OPTO / FET DYNAMICS","04  TAPE & OUTPUT TRANSFORMER","MASTER  /  ROUTING & LEVEL"};
 const int tops[]={76,255,640,856,1025},heights[]={169,375,206,159,150};
 for(int i=0;i<5;++i){g.setColour(panel);g.fillRoundedRectangle(18.f,(float)tops[i],1224.f,(float)heights[i],10.f);g.setColour(i==4?blue:gold);g.drawRoundedRectangle(18.f,(float)tops[i],1224.f,(float)heights[i],10.f,1.2f);g.setFont(juce::Font(juce::FontOptions(15.f,juce::Font::bold)));g.drawText(headings[i],35,tops[i]+8,800,23,juce::Justification::centredLeft);}
 g.setColour(ink);g.setFont(juce::Font(juce::FontOptions(12.f)));g.drawText("API 560: 10 graphic bands below. Neve / SSL: four bands above.",34,604,900,22,juce::Justification::centredLeft);
}
void UltimateEditor::resized(){
 auto dial=[&](int i,int x,int y){auto& k=*dials[(size_t)i];k.slider.setBounds(x,y,95,85);k.label.setBounds(x-13,y+83,121,20);};
 for(int i=0;i<4;++i)dial(i,38+i*113,117);
 for(int i=4;i<=12;++i)dial(i,36+(i-4)*130,303);
 for(int i=13;i<=18;++i)dial(i,36+(i-13)*135,685);
 for(int i=19;i<=20;++i)dial(i,38+(i-19)*145,899);
 for(int i=21;i<=24;++i)dial(i,36+(i-21)*140,1063);
 for(int i=25;i<=34;++i)dial(i,36+(i-25)*119,489);
 auto choice=[&](int i,int x,int y,int w){auto& k=*choices[(size_t)i];k.label.setBounds(x,y,w,20);k.combo.setBounds(x,y+21,w,28);};
 choice(0,550,120,300);choice(1,870,120,300);choice(2,850,268,320);
 for(int i=3;i<=7;++i)choice(i,840,664+(i-3)*38,320);
 choice(8,345,886,390);choice(9,770,886,390);choice(10,660,1060,270);
 toggles[0]->button.setBounds(1050,80,170,30);
 toggles[1]->button.setBounds(1050,257,150,30);
 toggles[2]->button.setBounds(1050,641,160,30);
 toggles[3]->button.setBounds(1050,857,180,30);
 toggles[4]->button.setBounds(36,795,130,28);
 toggles[5]->button.setBounds(190,795,130,28);
 toggles[6]->button.setBounds(345,795,160,28);
}
