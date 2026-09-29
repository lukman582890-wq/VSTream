# VSTream

Real-time audio streaming from Cubase to Android.

## Architecture

Cubase VST3 Sender -> Opus/UDP transport -> Android Receiver

## Goals

- One-click START STREAM in Cubase
- Copy a shareable stream link
- Open the link on Android to launch the receiver
- Real-time stereo audio without exporting
- Local Wi-Fi low-latency mode first
- Remote streaming can be added later

## Project layout

- `vst3-sender/` — JUCE VST3 plugin
- `android-receiver/` — Android receiver application
- `protocol/` — transport and session specification
- `docs/` — architecture and build notes

## Status

VST3 Windows build pipeline verified; artifact packaging is being finalized.
