#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
class UltimateProcessor final : public juce::AudioProcessor {
public:
 UltimateProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "Ultimate Channel Strip"; }
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 juce::AudioProcessorValueTreeState parameters;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 struct Stage {float low=0,high=0,flux=0,cap=0,mag=0,hf=0,env=0,slow=0,fast=0,makeup=0,scX=0,scY=0;};
 std::array<std::array<Stage,2>,6> stages{};
 std::array<std::array<juce::dsp::IIR::Filter<float>,12>,2> eqFilters;
 std::array<std::unique_ptr<juce::dsp::Oversampling<float>>,4> oversamplers;
 juce::AudioBuffer<float> scratch;
 std::array<std::vector<float>,2> dryDelay,wetDelay;
 int delayLength=0,writePos=0,latency=0,maxBlock=512;
 double sampleRate=44100;
 float value(const char*) const;
 void updateEQ(float);
 float transformer(float,int,int,float,float);
 float circuit(float,int,float,float);
 float tape(float,int,float,float);
 float dynamics(float,int,float);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UltimateProcessor)
};
