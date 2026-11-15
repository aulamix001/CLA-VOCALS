cmake_minimum_required(VERSION 3.22)
project(VocalSuite VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    # Ship self-contained binaries without requiring the VC++ runtime installer.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
option(VOCAL_DSP_ONLY "Build portable DSP tests without JUCE" OFF)
include(CTest)
if(BUILD_TESTING)
    add_executable(VocalDSPTests Tests/DspTests.cpp)
    target_include_directories(VocalDSPTests PRIVATE Source)
    add_test(NAME VocalDSP COMMAND VocalDSPTests)
endif()
if(VOCAL_DSP_ONLY)
    return()
endif()
if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Windows requires a 64-bit toolchain. Select -A x64.")
endif()
set(JUCE_DIR "" CACHE PATH "Optional local JUCE 8.0.6 checkout")
if(JUCE_DIR)
    add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 51a8a6d7aeae7326956d747737ccf1575e61e209 GIT_SHALLOW FALSE)
    FetchContent_MakeAvailable(JUCE)
endif()
juce_add_binary_data(VocalArtwork SOURCES Assets/cla-vocals-reference.png)
juce_add_plugin(VocalSuite
    COMPANY_NAME "AulaMix"
    BUNDLE_ID "com.aulamix.vocalsuite"
    PLUGIN_MANUFACTURER_CODE AuMx
    PLUGIN_CODE VcS1
    PRODUCT_NAME "Vocal Suite"
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Dynamics Reverb)
target_sources(VocalSuite PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(VocalSuite PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VocalSuite PRIVATE VocalArtwork juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
if(BUILD_TESTING)
    juce_add_console_app(VocalProcessorTests PRODUCT_NAME "Vocal Processor Tests")
    target_include_directories(VocalProcessorTests PRIVATE Source "${JUCE_MODULES_DIR}")
    target_sources(VocalProcessorTests PRIVATE Tests/ProcessorTests.cpp)
    target_link_libraries(VocalProcessorTests PRIVATE VocalSuite)
    add_test(NAME VocalProcessor COMMAND VocalProcessorTests)
endif()
