cmake_minimum_required(VERSION 3.22)

project(VSTream VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE will be supplied through JUCE_DIR or FetchContent in the next build step.
# This initial skeleton intentionally keeps the transport independent from JUCE.

add_library(VSTreamCore INTERFACE)
target_include_directories(VSTreamCore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# VST3 target will be enabled after JUCE is configured.
