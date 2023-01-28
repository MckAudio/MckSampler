cmake_minimum_required(VERSION 3.15)

project(MCK_SAMPLER VERSION 0.0.1)

if (APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
endif()

find_library(JACK NAMES jack PATHS /usr/lib /usr/lib64 /usr/lib64/pipewire-0.3/jack)

message(STATUS "Found jack: ${JACK}")

add_subdirectory(deps/JUCE)
add_subdirectory(deps/MckHelper)
add_subdirectory(deps/MckJuce/Content)
add_subdirectory(deps/DaisySP)

set_property(TARGET MckBinaryData PROPERTY POSITION_INDEPENDENT_CODE ON)

juce_add_gui_app(MckSamplerApp
    VERSION 0.0.1
    COMPANY_NAME MckAudio
    PRODUCT_NAME "MckSampler"
)

set_target_properties(MckSamplerApp PROPERTIES PUBLIC
  CXX_STANDARD 17 
  CXX_STANDARD_REQUIRED
  )

juce_generate_juce_header(MckSamplerApp)

target_include_directories(MckSamplerApp
    PRIVATE
    /usr/local/include
    /opt/homebrew/include
    ./deps/MckJuce/Source
    ./deps/concurrentqueue
    ./deps/MckHelper/include
    ./deps/DaisySP/Source
)

target_sources(MckSamplerApp
    PRIVATE
    ./deps/MckJuce/Source/BwLookAndFeel.cpp
    ./deps/MckJuce/Source/MckLookAndFeel.cpp
    src/BarControlComponent.hpp
    src/BarMenuComponent.hpp
    src/BarPadComponent.hpp
    src/BarSelectorComponent.hpp
    src/Config.cpp
    src/Config.hpp
    src/ConfigFile.cpp
    src/ConfigFile.hpp
    src/Content.cpp
    src/Content.hpp
    src/Dial.hpp
    src/DrumPad.hpp
    src/LevelMeter.hpp
    src/main.cpp
    src/MainComponent.cpp
    src/MainComponent.hpp
    src/Processing.cpp
    src/Processing.hpp
    src/SampleExplorer.cpp
    src/SampleExplorer.hpp
    src/Types.cpp
    src/Types.hpp
    src/dsp/DelayModule.cpp
    src/dsp/OnePoleFilter.cpp
    src/dsp/ReverbModule.cpp
)

target_compile_definitions(MckSamplerApp
    PRIVATE
    JUCE_DISPLAY_SPLASH_SCREEN=0
    # JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_XINPUT2=1
)

target_link_libraries(MckSamplerApp
    PRIVATE
    nlohmann_json
    MckHelper
    MckBinaryData
    DasySP
    ${JACK}
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)