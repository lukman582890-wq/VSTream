# VSTream VST3 Sender

## MVP UI

- STREAM button
- Session ID
- Stream URL
- Copy Link button
- Connection/status indicator
- Audio input meter

## Host

Cubase via VST3.

## Build

Windows + Visual Studio + CMake + JUCE.

The first milestone is a clean VST3 plugin that builds and loads in Cubase. Network transport is intentionally isolated from the DSP/audio callback so networking never blocks real-time audio processing.

## Real-time rule

Never perform socket I/O, memory allocation, filesystem I/O, or UI operations directly in the VST audio callback. Audio blocks are copied into a lock-free/ring buffer and consumed by the streaming worker.
