# VSTream Protocol v1

VSTream v1 is a local-LAN realtime audio transport.

## Flow

1. VST3 creates a session and HTTP session URL.
2. VST3 captures stereo audio from Cubase.
3. Audio is converted to 48 kHz stereo 20 ms frames (960 samples/channel).
4. Frames are encoded with libopus at 128 kbps.
5. Encoded packets are sent over UDP multicast.
6. Android joins the multicast group, filters by session ID, decodes Opus, and writes PCM to the audio output.
7. A browser receiver can use the same session URL as the control plane; browser transport is separate because normal browsers do not expose raw UDP.

## Network

- HTTP session/control: TCP `45821`
- Audio: UDP `239.255.77.77:45822`
- Audio transport: IPv4 multicast
- Codec: Opus
- Codec input/output: 48 kHz, stereo
- Frame duration: 20 ms
- Frames per packet: 960 samples/channel
- Target bitrate: 128 kbps

## Audio packet

All multi-byte fields are big-endian.

| offset | size | field |
|---:|---:|---|
| 0 | 4 | magic = ASCII `VSTM` |
| 4 | 2 | protocol version = 1 |
| 6 | 1 | channels = 2 |
| 7 | 1 | flags |
| 8 | 4 | sequence number |
| 12 | 4 | sample rate = 48000 |
| 16 | 2 | frame samples = 960 |
| 18 | 2 | Opus payload length |
| 20 | 8 | session ID (ASCII, padded) |
| 28 | N | Opus payload |

Receivers must discard packets with invalid magic/version, wrong session ID, invalid payload length, or unsupported channel/sample-rate values.

Sequence numbers are used for loss/reordering detection. A receiver should use Opus packet-loss concealment when a sequence gap occurs.

## Receiver requirements

Android target is Android 10+ and must:

- open the session URL;
- join `239.255.77.77:45822`;
- filter packets by the session ID;
- decode Opus at 48 kHz stereo;
- maintain a small jitter buffer;
- output PCM through a low-latency audio API.
