#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include "PitchTracker.h"
#include "KeyDetector.h"

/*  Uncertain Voice — validation build v0.
    Ne modifie PAS l'audio : il passe le signal tel quel et l'analyse en
    parallèle pour valider le tracker + la détection de gamme (SPEC §10, étape 1).
    La correction viendra après.                                              */
class UncertainVoiceProcessor : public juce::AudioProcessor
{
public:
    UncertainVoiceProcessor();
    ~UncertainVoiceProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    // ---- interface pour l'éditeur (thread message) ----
    static constexpr int kTrace = 512;

    struct TraceSnapshot
    {
        std::array<float, kTrace> midi {};   // NaN = non-voisé
        int  count = 0;
    };
    TraceSnapshot getTrace() const;

    KeyDetector::Key getKey() const;
    void resetAnalysis();

private:
    // analyse
    double hostSr = 44100.0;
    int    decim  = 1;
    int    window = 1024;
    int    hop    = 256;

    PitchTracker tracker;
    KeyDetector  keyDet;

    // one-pole anti-alias avant décimation
    float lpState = 0.0f, lpCoeff = 0.0f;
    int   decCount = 0;

    // buffer circulaire des échantillons décimés
    std::vector<float> ring;
    int  ringPos = 0, ringFilled = 0, hopCount = 0;
    std::vector<float> linWin;

    // trace partagée (écrite audio-thread, lue message-thread)
    std::array<std::atomic<float>, kTrace> traceMidi;
    std::atomic<int> traceWrite { 0 };

    // gamme partagée
    std::atomic<int>   keyRoot { 9 };
    std::atomic<bool>  keyMinor { true };
    std::atomic<float> keyConf { 0.0f };
    juce::SpinLock     keyPcsLock;
    std::array<int, 7> keyPcs { {9,11,0,2,4,5,7} };

    void analyseWindow();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UncertainVoiceProcessor)
};
