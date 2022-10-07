cmake_minimum_required(VERSION 3.15)

project(MCK_SAMPLER VERSION 0.0.1)

add_subdirectory(deps/JUCE)

add_subdirectory(deps/MckJuce/Content)
set_property(TARGET MckBinaryData PROPERTY POSITION_INDEPENDENT_CODE ON)

juce_add_gui_app(MckSamplerApp
    VERSION 0.0.1
    COMPANY_NAME MckAudio
    PRODUCT_NAME "MckSampler"
)

juce_generate_juce_header(MckSamplerApp)

target_include_directories(MckSamplerApp
PRIVATE
./deps/MckJuce/Source)

target_sources(MckSamplerApp
    PRIVATE
    ./deps/MckJuce/Source/BwLookAndFeel.cpp
    ./deps/MckJuce/Source/MckLookAndFeel.cpp
    src/BarControlComponent.hpp
    src/BarMenuComponent.hpp
    src/BarPadComponent.hpp
    src/BarSelectorComponent.hpp
    src/Content.hpp
    src/Dial.hpp
    src/Main.cpp
    src/MainComponent.hpp
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
    MckBinaryData
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)