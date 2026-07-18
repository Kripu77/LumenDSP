cmake_minimum_required(VERSION 3.24)

project(LumenDSP
    VERSION 0.1.0
    DESCRIPTION "Premium open-source NAM amp modeler for modern fusion tones"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LUMENDSP_COPY_PLUGIN_AFTER_BUILD "Copy plugin artefacts after build" OFF)

include(FetchContent)

set(LUMENDSP_JUCE_GIT_TAG "8.0.6" CACHE STRING "JUCE git tag (8.x required for macOS 15+ SDK; CMake plugin API matches JUCE 7 workflow)")
set(LUMENDSP_NAM_GIT_TAG "v0.5.4" CACHE STRING "NeuralAmpModelerCore git tag")

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${LUMENDSP_JUCE_GIT_TAG}
    GIT_SHALLOW    TRUE
)

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(juce)

FetchContent_Declare(
    neural_amp_modeler_core
    GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG        ${LUMENDSP_NAM_GIT_TAG}
    GIT_SHALLOW    TRUE
    GIT_SUBMODULES_RECURSE TRUE
)

set(FETCHCONTENT_QUIET OFF)
FetchContent_GetProperties(neural_amp_modeler_core)
if(NOT neural_amp_modeler_core_POPULATED)
    FetchContent_Populate(neural_amp_modeler_core)
endif()

set(LUMENDSP_NAM_ROOT "${neural_amp_modeler_core_SOURCE_DIR}")
set(LUMENDSP_NAM_EIGEN_DIR "${LUMENDSP_NAM_ROOT}/Dependencies/eigen")
set(LUMENDSP_NAM_NLOHMANN_DIR "${LUMENDSP_NAM_ROOT}/Dependencies/nlohmann")

file(GLOB LUMENDSP_NAM_SOURCES_TOP CONFIGURE_DEPENDS
    "${LUMENDSP_NAM_ROOT}/NAM/*.cpp"
)
file(GLOB LUMENDSP_NAM_SOURCES_NESTED CONFIGURE_DEPENDS
    "${LUMENDSP_NAM_ROOT}/NAM/*/*.cpp"
)

add_library(NeuralAmpModelerCore OBJECT
    ${LUMENDSP_NAM_SOURCES_TOP}
    ${LUMENDSP_NAM_SOURCES_NESTED}
)

target_include_directories(NeuralAmpModelerCore
    SYSTEM PUBLIC
        "${LUMENDSP_NAM_ROOT}"
        "${LUMENDSP_NAM_EIGEN_DIR}"
        "${LUMENDSP_NAM_NLOHMANN_DIR}"
)

target_compile_features(NeuralAmpModelerCore PUBLIC cxx_std_20)

target_compile_definitions(NeuralAmpModelerCore
    PUBLIC
        NAM_SAMPLE_FLOAT
        NAM_ENABLE_A2_FAST
    PRIVATE
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
)

if(APPLE)
    target_compile_options(NeuralAmpModelerCore PRIVATE -stdlib=libc++)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(NeuralAmpModelerCore PUBLIC stdc++fs)
endif()

juce_add_plugin(LumenDSP
    COMPANY_NAME "LumenDSP"
    COMPANY_WEBSITE "https://github.com/Kripu77/LumenDSP"
    COMPANY_EMAIL ""
    BUNDLE_ID "com.lumendsp.LumenDSP"
    PLUGIN_MANUFACTURER_CODE LmDp
    PLUGIN_CODE LmNm
    FORMATS Standalone VST3
    PRODUCT_NAME "LumenDSP"
    PLUGIN_NAME "LumenDSP"
    DESCRIPTION "NAM amp modeler for modern fusion tones"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD ${LUMENDSP_COPY_PLUGIN_AFTER_BUILD}
    VST3_CATEGORIES Fx Distortion
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "LumenDSP needs microphone access for guitar input."
)

target_sources(LumenDSP
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/audio/NamEngine.cpp
        Source/audio/IrConvolver.cpp
        Source/audio/NoiseGate.cpp
        Source/audio/ThreeBandEq.cpp
        Source/audio/LevelMeter.cpp
        Source/audio/AudioPipeline.cpp
        Source/presets/PresetManager.cpp
        Source/presets/FactoryContentInstaller.cpp
        Source/ui/LumenLookAndFeel.cpp
        Source/ui/KnobComponent.cpp
        Source/ui/LedMeterComponent.cpp
        Source/ui/FileSlotComponent.cpp
        Source/ui/PresetBarComponent.cpp
        Source/ui/TopChromeComponent.cpp
        Source/ui/SectionNavComponent.cpp
        Source/ui/InputStageComponent.cpp
        Source/ui/PedalModuleComponent.cpp
        Source/ui/AmpFaceplateComponent.cpp
        Source/ui/EqRackComponent.cpp
        Source/ui/CabStageComponent.cpp
)

set(LUMENDSP_FACTORY_CONTENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources/FactoryContent")


target_include_directories(LumenDSP
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_compile_definitions(LumenDSP
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
    PRIVATE
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
)

target_link_libraries(LumenDSP
    PRIVATE
        NeuralAmpModelerCore
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(LumenDSP)

if(TARGET LumenDSP_Standalone)
    if(APPLE)
        add_custom_command(TARGET LumenDSP_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:LumenDSP_Standalone>/../Resources/FactoryContent"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${LUMENDSP_FACTORY_CONTENT_DIR}"
                    "$<TARGET_FILE_DIR:LumenDSP_Standalone>/../Resources/FactoryContent"
            COMMENT "Embedding factory content in standalone app bundle"
            VERBATIM
        )
    else()
        add_custom_command(TARGET LumenDSP_Standalone POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:LumenDSP_Standalone>/FactoryContent"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${LUMENDSP_FACTORY_CONTENT_DIR}"
                    "$<TARGET_FILE_DIR:LumenDSP_Standalone>/FactoryContent"
            COMMENT "Copying factory content next to standalone executable"
            VERBATIM
        )
    endif()
endif()

if(TARGET LumenDSP_VST3)
    if(APPLE)
        add_custom_command(TARGET LumenDSP_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:LumenDSP_VST3>/../Resources/FactoryContent"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${LUMENDSP_FACTORY_CONTENT_DIR}"
                    "$<TARGET_FILE_DIR:LumenDSP_VST3>/../Resources/FactoryContent"
            COMMENT "Embedding factory content in VST3 bundle"
            VERBATIM
        )
    else()
        add_custom_command(TARGET LumenDSP_VST3 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:LumenDSP_VST3>/FactoryContent"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${LUMENDSP_FACTORY_CONTENT_DIR}"
                    "$<TARGET_FILE_DIR:LumenDSP_VST3>/FactoryContent"
            COMMENT "Copying factory content next to VST3 module"
            VERBATIM
        )
    endif()
endif()
