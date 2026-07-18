# ============================================================
#  ATLAS by Boogie Labs — a JUCE synth plugin
#  Build recipe. You shouldn't need to edit this to get started.
# ============================================================
cmake_minimum_required(VERSION 3.22)

project(Atlas VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------
# Download JUCE automatically (first configure takes a few min)
# ------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.14
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Auto-install the plugin after building (turned OFF on CI servers)
option(ATLAS_COPY_PLUGIN "Copy plugin to system folder after build" ON)

# ------------------------------------------------------------
# The plugin
# ------------------------------------------------------------
juce_add_plugin(Atlas
    COMPANY_NAME              "Boogie Labs"
    PLUGIN_MANUFACTURER_CODE  Boog
    PLUGIN_CODE               Atls
    IS_SYNTH                  TRUE
    NEEDS_MIDI_INPUT          TRUE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    COPY_PLUGIN_AFTER_BUILD   ${ATLAS_COPY_PLUGIN}   # auto-install to your plugin folder
    FORMATS                   VST3 AU Standalone   # AU is skipped on Windows
    PRODUCT_NAME              "Atlas")

target_sources(Atlas PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Atlas PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Atlas PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp                 # filter + delay line
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
