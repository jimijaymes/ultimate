#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace {
float gain(float x){return juce::Decibels::decibelsToGain(x);}
float pole(float hz,float sr){return std::exp(-juce::MathConstants<float>::twoPi*juce::jlimit(1.f,sr*.45f,hz)/sr);}
float knee(float x,float ratio,float width){const float half=width*.5f;if(x<=-half)return 0.f;if(x>=half)return x*(1.f-1.f/ratio);const float a=x+half;return (1.f-1.f/ratio)*a*a/(2.f*width);}
using Coeff=juce::dsp::IIR::Coefficients<float>;
}
juce::AudioProcessorValueTreeState::ParameterLayout UltimateProcessor::layout(){
 using F=juce::AudioParameterFloat;using C=juce::AudioParameterChoice;using B=juce::AudioParameterBool;
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float def){p.add(std::make_unique<F>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi,.1f),def));};
 auto c=[&](const char* id,const char* name,juce::StringArray opts,int def){p.add(std::make_unique<C>(juce::ParameterID{id,1},name,opts,def));};
 auto b=[&](const char* id,const char* name,bool def){p.add(std::make_unique<B>(juce::ParameterID{id,1},name,def));};
 f("input","Input dB",-24,24,0);f("output","Output dB",-24,24,0);f("masterDrive","Global Saturation Drive Offset dB",-24,24,0);f("mix","Master Mix %",0,100,100);
 c("oversampling","Oversampling",{"1x","2x","4x","8x"},2);
 b("preOn","Preamp Enabled",true);b("eqOn","EQ Enabled",true);b("dynOn","Dynamics Enabled",true);b("tapeOn","Tape / Output Enabled",true);
 c("preTransformer","Input Transformer",{"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},1);
 c("circuit","Circuit",{"Off","Discrete Class A","Class AB","Op-Amp","Triode Tube","Pentode Tube"},1);
 f("preDrive","Input Transformer Drive dB",-24,24,0);f("circuitDrive","Circuit Drive dB",-24,24,0);f("character","Saturation Character %",0,100,50);f("bias","Circuit Bias %",-100,100,0);
 c("eqModel","EQ Model",{"Neve 1073","SSL G Series","API 560"},0);
 f("eqLow","Low Gain dB",-15,15,0);f("eqLowFreq","Low Frequency Hz",30,350,100);
 f("eqLowMid","Low Mid Gain dB",-15,15,0);f("eqLowMidFreq","Low Mid Frequency Hz",100,2000,360);
 f("eqHighMid","High Mid Gain dB",-15,15,0);f("eqHighMidFreq","High Mid Frequency Hz",800,9000,3000);
 f("eqHigh","High Gain dB",-15,15,0);f("eqHighFreq","High Frequency Hz",3000,16000,10000);
 f("eqHPF","EQ High Pass Hz",20,300,20);
 for(int i=0;i<10;++i){const auto id=juce::String("apiBand")+juce::String(i);const auto name=juce::String("API 560 Band ")+juce::String(i+1);p.add(std::make_unique<F>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(-12.f,12.f,.1f),0.f));}
 b("optoOn","Optical Compressor Enabled",true);b("fetOn","FET Compressor Enabled",true);
 b("fetFirst","FET Before Optical",false);
 f("optoPeak","Optical Peak Reduction",0,100,40);f("optoGain","Optical Makeup dB",-12,24,0);
 c("optoRatio","Optical Ratio",{"Compress 3:1","Limit 20:1"},0);
 c("optoResponse","Optical Response",{"Enhanced","Classic T4"},0);
 c("optoRelease","Optical Release",{"LA-2A Slow","LA-3A Fast"},0);
 f("fetDrive","FET Drive",0,100,40);f("fetAttack","FET Attack",0,100,50);f("fetRelease","FET Release",0,100,50);
 c("fetRatio","FET Ratio",{"4:1","8:1","12:1","20:1","All Buttons"},0);
 c("fetTiming","FET Timing",{"1176 Classic","Extended"},0);
 f("sidechainHPF","Shared Sidechain HPF Hz",20,400,90);
 c("tape","Tape Model",{"Off","Studer A80 15 IPS","Studer A80 30 IPS","Nakamichi Dragon Metal","Studer J37 Abbey Road 15 IPS","Budget Cassette Ferric","Budget Cassette Metal"},1);
 f("tapeDrive","Tape Drive dB",-24,24,0);
 c("outputTransformer","Output Transformer",{"Off","Neve 10468 Input","Neve LO1166 Output","API 2622 + 2520","API 2520 + 2503","1176 UTC Input","1176 Class A Output","1176 Rev F Output"},0);
 f("outputTransformerDrive","Output Transformer Drive dB",-24,24,0);
 return p;
}
UltimateProcessor::UltimateProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"STATE",layout()){}
float UltimateProcessor::value(const char* id)const{return parameters.getRawParameterValue(id)->load(std::memory_order_relaxed);}
void UltimateProcessor::prepareToPlay(double sr,int block){
 sampleRate=sr;maxBlock=juce::jmax(1,block);stages={};scratch.setSize(2,maxBlock);
 for(auto& bank:eqFilters)for(auto& filter:bank)filter.reset();
 for(int i=0;i<4;++i){oversamplers[(size_t)i]=std::make_unique<juce::dsp::Oversampling<float>>(2,(size_t)i,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,false);oversamplers[(size_t)i]->initProcessing((size_t)maxBlock);oversamplers[(size_t)i]->reset();}
 latency=0;for(auto& os:oversamplers)latency=juce::jmax(latency,(int)std::ceil(os->getLatencyInSamples()));
 delayLength=juce::jmax(1024,latency+maxBlock+16);for(auto& d:dryDelay)d.assign((size_t)delayLength,0);for(auto& d:wetDelay)d.assign((size_t)delayLength,0);writePos=0;setLatencySamples(latency);
}
float UltimateProcessor::transformer(float x,int channel,int slot,float driveDb,float rate){
 const int model=juce::jlimit(0,7,(int)value(slot==0?"preTransformer":"outputTransformer"));if(model==0)return x;
 auto& s=stages[(size_t)slot][(size_t)channel];
 const float hp[]={0,22,12,24,9,28,14,18},lp[]={0,19000,17500,21000,22000,15500,18500,19500};
 const float magnet[]={0,.65f,1.1f,.8f,1.3f,.7f,1.35f,1.f},amp[]={0,.25f,.9f,.8f,1.1f,.2f,1.f,1.15f};
 const float asym[]={0,.035f,.085f,.075f,.045f,.02f,.095f,.045f};
 const float character=value("character")*.01f,bias=value("bias")*.01f,drive=gain(driveDb);
 const float a=pole(hp[model],rate),z=pole(lp[model],rate),leak=pole(hp[model]*.55f,rate);
 s.low=a*s.low+(1-a)*x;s.high=z*s.high+(1-z)*x;
 const float ac=x-s.low;s.flux=leak*s.flux+(1-leak)*ac*(140.f/juce::jmax(10.f,hp[model]));
 const float field=s.flux*magnet[model]*drive*.22f;
 const float core=(std::tanh(field+bias*.12f)-field)*(.04f+.15f*character);
 const float scale=drive*(.65f+amp[model]*character),offset=asym[model]+bias*.1f;
 const float saturated=(std::tanh(x*scale+offset)-std::tanh(offset))/juce::jmax(.1f,scale);
 const float blend=.14f+.36f*character;
 return (1-blend)*x+blend*saturated+core-(x-s.high)*(.015f+.065f*character);
}
float UltimateProcessor::circuit(float x,int channel,float driveDb,float rate){
 const int model=(int)value("circuit");if(model==0)return x;
 auto& s=stages[1][(size_t)channel];const float cap[]={0,18,32,8,25,45};
 const float a=pole(cap[model],rate),bias=value("bias")*.01f,character=value("character")*.01f;
 s.cap=a*s.cap+(1-a)*x;const float drive=gain(driveDb),v=(x-s.cap)*drive;
 float y=v;
 switch(model){
 case 1:y=std::tanh(v+.18f+bias*.18f)-std::tanh(.18f+bias*.18f);break;
 case 2:{const float width=.045f+.025f*(1-character),cross=.003f+.016f*character+.025f*std::abs(bias);const float z=(v-cross*v/std::sqrt(v*v+width*width))/juce::jmax(.65f,1-cross/width);y=std::tanh(z*.88f)/.88f;break;}
 case 3:y=juce::jlimit(-1.05f,1.05f,v+.04f*v*v);break;
 case 4:y=std::tanh(v*.8f+.28f+bias*.25f)-std::tanh(.28f+bias*.25f);break;
 case 5:y=std::tanh(v*1.35f+.45f+bias*.3f)-std::tanh(.45f+bias*.3f);break;
 }
 const float z=pole(model==3?18500.f:12000.f,rate);s.high=z*s.high+(1-z)*y;
 return (y-.09f*character*(y-s.high))/juce::jmax(.1f,drive);
}
float UltimateProcessor::tape(float x,int channel,float driveDb,float rate){
 const int model=(int)value("tape");if(model==0)return x;
 auto& s=stages[4][(size_t)channel];
 const float hp[]={0,32,48,25,36,90,65},top[]={0,17500,20500,21500,14500,7200,11200};
 const float density[]={0,.90f,.75f,.65f,1.18f,2.3f,1.65f};
 const float k[]={0,.82f,.88f,.74f,.68f,.43f,.62f},ceiling[]={0,1.20f,1.24f,1.15f,1.12f,.90f,1.03f};
 const float sensitivity[]={0,.20f,.13f,.28f,.34f,.60f,.43f};
 const float a=pole(hp[model],rate),b=pole(top[model],rate),character=value("character")*.01f;
 s.low=a*s.low+(1-a)*x;s.high=b*s.high+(1-b)*x;const float hf=x-s.high;
 const float level=std::abs(hf),coeff=pole(level>s.env?900.f:85.f,rate);
 s.env=coeff*s.env+(1-coeff)*level;
 const float drive=juce::jmax(.001f,gain(driveDb)*density[model]*.25f);
 const float overload=juce::jmax(0.f,drive*s.env-.55f);
 const float record=x-hf*(1.f-1.f/(1.f+sensitivity[model]*overload))+.025f*character*s.low;
 const float bias=value("bias")*.0012f;
 const auto magnetise=[&](float v){const float mag=std::abs(v);if(mag<=k[model])return v;const float range=ceiling[model]-k[model];return std::copysign(ceiling[model]-range*std::exp(-(mag-k[model])/range),v);};
 const float field=record*drive+bias,equilibrium=magnetise(field);
 const float z=pole(model>=5?360.f:(model==4?500.f:2100.f),rate);
 s.mag=z*s.mag+(1-z)*equilibrium;
 return (.94f*equilibrium+.06f*s.mag-magnetise(bias))/drive+(model>=5?.16f:(model==4?.10f:.045f))*character*s.low;
}
float UltimateProcessor::dynamics(float x,int channel,float rate){
 const bool opto=value("optoOn")>.5f,fet=value("fetOn")>.5f;
 const auto processOpto=[&](float v){
  auto& s=stages[2][(size_t)channel];const float h=1.f/(1.f+juce::MathConstants<float>::twoPi*value("sidechainHPF")/rate);
  const float detector=h*(s.scY+v-s.scX);s.scX=v;s.scY=detector;
  const float level=juce::Decibels::gainToDecibels(juce::jmax(std::abs(detector),1e-8f),-160.f);
  const float ratio=value("optoRatio")>.5f?20.f:3.f;
  const float target=knee(level+value("optoPeak")*.6f-25.f,ratio,10.f);
  const bool classic=value("optoResponse")>.5f,fast=value("optoRelease")>.5f;
  const float rise=pole(1.f/(classic?.010f:.018f),rate),fall=pole(1.f/(fast?.12f:(classic?1.5f:.45f)),rate);
  const float coeff=target>s.env?rise:fall;s.env=coeff*s.env+(1-coeff)*target;
  return v*gain(value("optoGain")-s.env);
 };
 const auto processFet=[&](float v){
  auto& s=stages[3][(size_t)channel];const float drive=value("fetDrive")*.36f,hot=v*gain(drive);
  const float h=1.f/(1.f+juce::MathConstants<float>::twoPi*value("sidechainHPF")/rate);
  const float detector=h*(s.scY+hot-s.scX);s.scX=hot;s.scY=detector;
  const bool extended=value("fetTiming")>.5f;
  const float slowAttack=extended?.030f:.0008f,fastRelease=extended?.025f:.050f,slowRelease=extended?1.65f:1.1f;
  const float attack=slowAttack*std::pow(.000020f/slowAttack,value("fetAttack")*.01f);
  const float release=fastRelease*std::pow(slowRelease/fastRelease,1.f-value("fetRelease")*.01f);
  const float level=std::abs(detector),coeff=pole(1.f/(level>s.env?attack:release),rate);
  s.env=coeff*s.env+(1-coeff)*level;
  const float ratios[]={4.f,8.f,12.f,20.f,30.f};const int index=juce::jlimit(0,4,(int)value("fetRatio"));
  const float db=juce::Decibels::gainToDecibels(juce::jmax(s.env,1e-8f),-160.f);
  const float gr=knee(db+18.f,ratios[index],index==4?12.f:5.f);
  const float makeupTarget=juce::jlimit(0.f,12.f,gr*.6f);
  const float mc=pole(1.f/(makeupTarget>s.makeup?.25f:.65f),rate);
  s.makeup=mc*s.makeup+(1-mc)*makeupTarget;
  return hot*gain(-drive-gr+s.makeup);
 };
 if(value("fetFirst")>.5f){if(fet)x=processFet(x);if(opto)x=processOpto(x);}
 else {if(opto)x=processOpto(x);if(fet)x=processFet(x);}
 return x;
}
void UltimateProcessor::updateEQ(float rate){
 const int model=(int)value("eqModel");
 auto assign=[&](int slot,Coeff::Ptr coeff){for(auto& bank:eqFilters)*bank[(size_t)slot].coefficients=*coeff;};
 auto peak=[&](int slot,float hz,float db,float q){assign(slot,Coeff::makePeakFilter(rate,juce::jlimit(20.f,rate*.45f,hz),q,gain(db)));};
 for(int i=0;i<12;++i)assign(i,Coeff::makePeakFilter(rate,1000.f,.707f,1.f));
 if(model==2){const float hz[]={31.f,63.f,125.f,250.f,500.f,1000.f,2000.f,4000.f,8000.f,16000.f};for(int i=0;i<10;++i){const auto id=juce::String("apiBand")+juce::String(i);peak(i,hz[i],value(id.toRawUTF8()),1.25f);}}
 else {
  const float lo=value("eqLowFreq"),lm=value("eqLowMidFreq"),hm=value("eqHighMidFreq"),hi=value("eqHighFreq");
  assign(0,Coeff::makeLowShelf(rate,lo,.707f,gain(value("eqLow"))));
  peak(1,lm,value("eqLowMid"),model==0?1.0f:1.4f);
  peak(2,hm,value("eqHighMid"),model==0?1.1f:1.7f);
  assign(3,Coeff::makeHighShelf(rate,hi,.707f,gain(value("eqHigh"))));
  if(value("eqHPF")>20.1f)assign(4,Coeff::makeHighPass(rate,value("eqHPF")));
 }
}
void UltimateProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;const int channels=juce::jmin(2,buffer.getNumChannels()),n=buffer.getNumSamples();if(channels==0||n==0)return;
 for(int c=channels;c<buffer.getNumChannels();++c)buffer.clear(c,0,n);
 const int index=juce::jlimit(0,3,(int)value("oversampling"));auto& os=*oversamplers[(size_t)index];
 const int osLatency=(int)std::round(os.getLatencyInSamples()),padding=juce::jmax(0,latency-osLatency);
 const float in=gain(value("input")),out=gain(value("output")),mix=value("mix")*.01f;
 const float master=value("masterDrive");
 const bool pre=value("preOn")>.5f,eq=value("eqOn")>.5f,dyn=value("dynOn")>.5f,tapeOn=value("tapeOn")>.5f;
 const float rate=(float)sampleRate*(float)os.getOversamplingFactor();
 if(eq)updateEQ(rate);
 for(int offset=0;offset<n;offset+=maxBlock){
  const int count=juce::jmin(maxBlock,n-offset);
  for(int c=0;c<channels;++c){const float* src=buffer.getReadPointer(c,offset);float* dst=scratch.getWritePointer(c);for(int i=0;i<count;++i)dst[i]=src[i]*in;}
  juce::dsp::AudioBlock<float> base(scratch);auto slice=base.getSubBlock(0,(size_t)count);
  juce::dsp::AudioBlock<const float> input(slice);auto up=os.processSamplesUp(input);
  for(int c=0;c<channels;++c){float* samples=up.getChannelPointer((size_t)c);for(size_t i=0;i<up.getNumSamples();++i){
   float x=samples[i];
   if(pre){x=transformer(x,c,0,master+value("preDrive"),rate);x=circuit(x,c,master+value("circuitDrive"),rate);}
   if(eq){const int bands=(int)value("eqModel")==2?10:5;for(int b=0;b<bands;++b)x=eqFilters[(size_t)c][(size_t)b].processSample(x);}
   if(dyn)x=dynamics(x,c,rate);
   if(tapeOn){x=tape(x,c,master+value("tapeDrive"),rate);x=transformer(x,c,5,master+value("outputTransformerDrive"),rate);}
   samples[i]=x;
  }}
  os.processSamplesDown(slice);
  for(int c=0;c<channels;++c){float* dst=buffer.getWritePointer(c,offset);const float* wet=scratch.getReadPointer(c);auto& dryRing=dryDelay[(size_t)c];auto& wetRing=wetDelay[(size_t)c];for(int i=0;i<count;++i){
   const int write=(writePos+i)%delayLength;
   const int rd=(write+delayLength-latency)%delayLength,rw=(write+delayLength-padding)%delayLength;
   const float dryInput=dst[i]*in;
   const float dry=latency==0?dryInput:dryRing[(size_t)rd];
   const float processed=padding==0?wet[i]:wetRing[(size_t)rw];
   dryRing[(size_t)write]=dryInput;wetRing[(size_t)write]=wet[i];
   dst[i]=(dry*(1.f-mix)+processed*mix)*out;
  }}
  writePos=(writePos+count)%delayLength;
 }
}
juce::AudioProcessorEditor* UltimateProcessor::createEditor(){return new UltimateEditor(*this);}
void UltimateProcessor::getStateInformation(juce::MemoryBlock& b){auto xml=parameters.copyState().createXml();copyXmlToBinary(*xml,b);}
void UltimateProcessor::setStateInformation(const void* data,int bytes){if(auto xml=getXmlFromBinary(data,bytes))if(xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new UltimateProcessor();}
