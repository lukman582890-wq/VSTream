# VSTream Protocol

Version 1 targets local-network streaming.

## Planned flow

1. VST3 creates a stream session.
2. Sender advertises a session on the local network.
3. Sender displays a link containing the session identifier.
4. Android opens the link using an Android App Link/deep link.
5. Receiver discovers or contacts the sender.
6. Audio is transported as low-latency Opus packets.

The exact packet format will be frozen before implementing the Android decoder.
