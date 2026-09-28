# VSTream Architecture

## MVP

Cubase
  |
  | VST3 audio callback
  v
VSTream Sender
  |
  | Opus / UDP
  v
Android Receiver
  |
  v
AudioTrack / Bluetooth / Headphones

## Link model

The sender generates a session URL containing a short-lived session ID. Android handles the URL with an App Link/deep link and uses the session ID to join the stream.

## Design priorities

1. Reliable build on Windows + Visual Studio
2. Low latency
3. No audio export
4. Simple pairing
5. Clear connection state
