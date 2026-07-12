cmake_minimum_required(VERSION 3.22)
project(MontagemPunch VERSION 0.1.0)

add_subdirectory(libs/JUCE)

if(APPLE)
    set(PUNCH_FORMATS VST3 AU Standalone)
else()
    set(PUNCH_FORMATS VST3 Standalone)
endif()

juce_add_plugin(MontagemPunch
    COMPANY_NAME "montagem.punch"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Mtgf
    PLUGIN_CODE Pnch
    FORMATS ${PUNCH_FORMATS}
    PRODUCT_NAME "Montagem Punch")

target_sources(MontagemPunch PRIVATE
    Source/PunchProcessor.cpp
    Source/PluginEditor.cpp
    Source/PluginEntry.cpp)

target_compile_definitions(MontagemPunch PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(MontagemPunch
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
