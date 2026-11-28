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
 const juce::StringArray tabNames{"PREAMP","EQUALISER","DYNAMICS","TAPE / OUTPUT","MASTER"};
 for(int i=0;i<5;++i){
  tabs[(size_t)i].setButtonText(tabNames[i]);
  tabs[(size_t)i].setClickingTogglesState(false);
  tabs[(size_t)i].onClick=[this,i]{activeTab=i;updateTab();};
  addAndMakeVisible(tabs[(size_t)i]);
 }
 choices[2]->combo.onChange=[this]{updateEqModel();};
 setSize(1120,570);
 updateTab();
}
void UltimateEditor::updateEqModel(){
 const bool api=choices[2]->combo.getSelectedItemIndex()==2;
 for(int i=4;i<=12;++i){dials[(size_t)i]->slider.setVisible(activeTab==1&&!api);dials[(size_t)i]->label.setVisible(activeTab==1&&!api);}
 for(int i=25;i<=34;++i){dials[(size_t)i]->slider.setVisible(activeTab==1&&api);dials[(size_t)i]->label.setVisible(activeTab==1&&api);}
 repaint();
}
void UltimateEditor::updateTab(){
 for(int i=0;i<5;++i)tabs[(size_t)i].setColour(juce::TextButton::buttonColourId,i==activeTab?gold:panel);
 const auto visibleDial=[this](int i,bool visible){dials[(size_t)i]->slider.setVisible(visible);dials[(size_t)i]->label.setVisible(visible);};
 const auto visibleChoice=[this](int i,bool visible){choices[(size_t)i]->combo.setVisible(visible);choices[(size_t)i]->label.setVisible(visible);};
 for(int i=0;i<(int)dials.size();++i)visibleDial(i,
  activeTab==0?i<4:activeTab==1?(i>=4&&i<=12||i>=25):activeTab==2?(i>=13&&i<=18):activeTab==3?(i==19||i==20):(i>=21&&i<=24));
 for(int i=0;i<(int)choices.size();++i)visibleChoice(i,
  activeTab==0?i<=1:activeTab==1?i==2:activeTab==2?(i>=3&&i<=7):activeTab==3?(i==8||i==9):i==10);
 for(int i=0;i<(int)toggles.size();++i)toggles[(size_t)i]->button.setVisible(
  activeTab==0?i==0:activeTab==1?i==1:activeTab==2?(i==2||i>=4):activeTab==3?i==3:false);
 updateEqModel();
 resized();
 repaint();
}
void UltimateEditor::paint(juce::Graphics& g){
 g.fillAll(bg);g.setColour(ink);
 g.setFont(juce::Font(juce::FontOptions(29.f,juce::Font::bold)));
 g.drawText("ULTIMATE / CHANNEL STRIP",25,14,1000,40,juce::Justification::centredLeft);
 g.setColour(panel);g.fillRoundedRectangle(18.f,120.f,1084.f,432.f,12.f);
 const char* headings[]={"INPUT TRANSFORMER / PREAMP","EQUALISER","OPTO + FET DYNAMICS","TAPE + OUTPUT TRANSFORMER","MASTER / ROUTING"};
 g.setColour(gold);g.setFont(juce::Font(juce::FontOptions(18.f,juce::Font::bold)));
 g.drawText(headings[activeTab],38,134,800,28,juce::Justification::centredLeft);
 if(activeTab==1&&choices[2]->combo.getSelectedItemIndex()==2){
  g.setColour(blue);g.setFont(13.f);
  g.drawText("API 560 / GRAPHIC EQ  -  10 BANDS",40,255,700,22,juce::Justification::centredLeft);
 }
}
void UltimateEditor::resized(){
 for(int i=0;i<5;++i)tabs[(size_t)i].setBounds(20+i*219,70,210,38);
 auto dial=[&](int i,int x,int y,int w=118,int h=128){
  auto& k=*dials[(size_t)i];
  k.label.setBounds(x,y,w,23);k.label.setMinimumHorizontalScale(.8f);
  k.slider.setBounds(x,y+25,w,h-25);
 };
 auto choice=[&](int i,int x,int y,int w){
  auto& k=*choices[(size_t)i];k.label.setBounds(x,y,w,22);
  k.combo.setBounds(x,y+26,w,32);k.label.setMinimumHorizontalScale(.8f);
 };
 if(activeTab==0){
  for(int i=0;i<4;++i)dial(i,40+i*145,245);
  choice(0,40,178,470);choice(1,535,178,470);
  toggles[0]->button.setBounds(830,137,170,30);
 }else if(activeTab==1){
  choice(2,40,177,440);
  toggles[1]->button.setBounds(850,140,160,30);
  const bool api=choices[2]->combo.getSelectedItemIndex()==2;
  if(api){
   for(int i=25;i<=34;++i){
    auto& k=*dials[(size_t)i];const int x=42+(i-25)*104;
    k.label.setBounds(x,301,90,22);
    k.slider.setSliderStyle(juce::Slider::LinearVertical);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,88,21);
    k.slider.setBounds(x+8,326,76,181);
   }
  }else{
   for(int i=4;i<=12;++i){
    auto& k=*dials[(size_t)i];
    k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,76,20);
    dial(i,43+((i-4)%5)*205,270+((i-4)/5)*130,158,120);
   }
  }
 }else if(activeTab==2){
  for(int i=13;i<=18;++i)dial(i,38+(i-13)*171,246,150,135);
  choice(3,40,176,180);choice(4,239,176,180);choice(5,438,176,180);
  choice(6,637,176,180);choice(7,836,176,180);
  toggles[2]->button.setBounds(40,425,160,28);
  toggles[4]->button.setBounds(218,425,135,28);
  toggles[5]->button.setBounds(370,425,135,28);
  toggles[6]->button.setBounds(523,425,150,28);
 }else if(activeTab==3){
  choice(8,40,186,490);choice(9,550,186,490);
  dial(19,42,290,165,150);dial(20,255,290,175,150);
  toggles[3]->button.setBounds(835,140,180,30);
 }else{
  for(int i=21;i<=24;++i)dial(i,45+i%21*195,270,160,145);
  choice(10,40,184,390);
 }
}
