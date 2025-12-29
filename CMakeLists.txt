cmake_minimum_required(VERSION 3.15)

project(AcidLadderVST VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(ACID_LADDER_ENABLE_VST2 "Build VST2 format (requires VST2 SDK)" OFF)
set(ACID_LADDER_VST2_SDK_PATH "" CACHE PATH "Path to the VST2 SDK (set when ACID_LADDER_ENABLE_VST2=ON)")

# ---- JUCE ----
# Expect JUCE repo at: juce/JUCE (must contain CMakeLists.txt)
add_subdirectory(juce/JUCE)

# ---- Plugin formats ----
set(ACID_LADDER_PLUGIN_FORMATS VST3)
if (ACID_LADDER_ENABLE_VST2)
    if (ACID_LADDER_VST2_SDK_PATH STREQUAL "" AND DEFINED ENV{VST2_SDK_PATH})
        set(ACID_LADDER_VST2_SDK_PATH "$ENV{VST2_SDK_PATH}" CACHE PATH "Path to the VST2 SDK (set from VST2_SDK_PATH env var)" FORCE)
    endif()

    if (ACID_LADDER_VST2_SDK_PATH STREQUAL "")
        message(FATAL_ERROR "ACID_LADDER_ENABLE_VST2=ON requires ACID_LADDER_VST2_SDK_PATH or VST2_SDK_PATH environment variable.")
    endif()

    juce_set_vst2_sdk_path("${ACID_LADDER_VST2_SDK_PATH}")
    list(APPEND ACID_LADDER_PLUGIN_FORMATS VST2)
endif()

# ---- Plugin Target ----
juce_add_plugin(AcidLadderVST
    COMPANY_NAME "MohinemDSP"
    PRODUCT_NAME "Acid Ladder VST"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    PLUGIN_MANUFACTURER_CODE MOHN
    PLUGIN_CODE ACID

    FORMATS ${ACID_LADDER_PLUGIN_FORMATS}
)

# Generates JuceHeader.h for this target
juce_generate_juce_header(AcidLadderVST)

target_sources(AcidLadderVST PRIVATE
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginEditor.cpp
)

target_include_directories(AcidLadderVST PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AcidLadderVST PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)

# ---- Recommended JUCE flags (keeps deps minimal on Linux) ----
target_compile_definitions(AcidLadderVST PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Optional: reasonable warnings (safe defaults)
if (MSVC)
    target_compile_options(AcidLadderVST PRIVATE /W4)
else()
    target_compile_options(AcidLadderVST PRIVATE -Wall -Wextra -Wpedantic)
endif()
