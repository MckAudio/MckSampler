cmake_minimum_required(VERSION 3.15)

project(MCK_SAMPLER VERSION 0.0.1)

add_subdirectory(deps/JUCE)

juce_add_plugin(MckSamplerApp
    VERSION 0.0.1
    COMPANY_NAME MckAudio
    PLUGIN_MANUFACTURER_CODE MckA
    PLUGIN_CODE McSr
    FORMATS Standalone
    PRODUCT_NAME "MckSampler"
    LV2URI https://github.com/MckAudio/MckSampler
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(MckSamplerApp)

target_sources(MckSamplerApp
    PRIVATE
)

target_compile_definitions(MckSamplerApp
    PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MckSamplerApp
    PRIVATE
    #MckBinaryData
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)