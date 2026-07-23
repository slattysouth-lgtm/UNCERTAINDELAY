cmake_minimum_required(VERSION 3.22)

project(UncertainVoice VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Édition free / payante (pattern habituel, comme RC_TRIAL / US_FREE) -------
option(UV_FREE "Build l'édition gratuite" OFF)

# --- JUCE via FetchContent ----------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Plugin -------------------------------------------------------------------
if(UV_FREE)
    set(UV_PRODUCT "Uncertain Voice Free")
    set(UV_CODE    Uvf1)
else()
    set(UV_PRODUCT "Uncertain Voice")
    set(UV_CODE    Uvc1)
endif()

juce_add_plugin(UncertainVoice
    PRODUCT_NAME              "${UV_PRODUCT}"
    COMPANY_NAME              "Uncertain"
    BUNDLE_ID                 "fr.uncertain.voice"
    FORMATS                   VST3
    PLUGIN_MANUFACTURER_CODE  Uncr
    PLUGIN_CODE               ${UV_CODE}
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
)

juce_generate_juce_header(UncertainVoice)

target_sources(UncertainVoice PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

# Réduit les dépendances (utile en cross-compilation MinGW) : pas de navigateur,
# pas de curl.
target_compile_definitions(UncertainVoice PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<BOOL:${UV_FREE}>:UV_FREE=1>
)

target_link_libraries(UncertainVoice PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
