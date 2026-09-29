#include <jni.h>
#include <opus/opus.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>

namespace {
std::mutex gMutex;
OpusDecoder* gDecoder = nullptr;
std::string gSessionId;

constexpr std::uint32_t kMagic = 0x5653544Du;
constexpr std::size_t kHeaderSize = 28;
constexpr int kSampleRate = 48000;
constexpr int kChannels = 2;
constexpr int kMaxFrameSamples = 5760;

std::uint16_t readU16(const std::uint8_t* p) {
    return static_cast<std::uint16_t>((p[0] << 8) | p[1]);
}
std::uint32_t readU32(const std::uint8_t* p) {
    return (static_cast<std::uint32_t>(p[0]) << 24)
         | (static_cast<std::uint32_t>(p[1]) << 16)
         | (static_cast<std::uint32_t>(p[2]) << 8)
         | static_cast<std::uint32_t>(p[3]);
}
}

extern "C" JNIEXPORT void JNICALL
Java_com_vstream_receiver_MainActivity_nativeInit(JNIEnv* env, jclass, jstring session)
{
    std::lock_guard<std::mutex> lock(gMutex);
    if (gDecoder) {
        opus_decoder_destroy(gDecoder);
        gDecoder = nullptr;
    }

    const char* chars = env->GetStringUTFChars(session, nullptr);
    gSessionId = chars != nullptr ? chars : "";
    if (chars) env->ReleaseStringUTFChars(session, chars);

    int error = OPUS_OK;
    gDecoder = opus_decoder_create(kSampleRate, kChannels, &error);
    if (error != OPUS_OK)
        gDecoder = nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_vstream_receiver_MainActivity_nativeReset(JNIEnv*, jclass)
{
    std::lock_guard<std::mutex> lock(gMutex);
    if (gDecoder)
        opus_decoder_ctl(gDecoder, OPUS_RESET_STATE);
}

extern "C" JNIEXPORT jshortArray JNICALL
Java_com_vstream_receiver_MainActivity_nativeDecode(JNIEnv* env, jclass, jbyteArray data)
{
    std::lock_guard<std::mutex> lock(gMutex);
    if (!gDecoder || data == nullptr)
        return nullptr;

    const jsize size = env->GetArrayLength(data);
    if (size < static_cast<jsize>(kHeaderSize))
        return nullptr;

    std::vector<std::uint8_t> packet(static_cast<std::size_t>(size));
    env->GetByteArrayRegion(data, 0, size, reinterpret_cast<jbyte*>(packet.data()));

    if (readU32(packet.data()) != kMagic || readU16(packet.data() + 4) != 1)
        return nullptr;
    if (packet[6] != kChannels || readU32(packet.data() + 12) != kSampleRate)
        return nullptr;

    const auto payloadSize = readU16(packet.data() + 18);
    if (payloadSize == 0 || kHeaderSize + payloadSize > packet.size())
        return nullptr;

    std::array<char, 8> expected{};
    std::memset(expected.data(), '0', expected.size());
    const auto copyCount = std::min<std::size_t>(gSessionId.size(), expected.size());
    std::memcpy(expected.data(), gSessionId.data(), copyCount);
    if (std::memcmp(packet.data() + 20, expected.data(), expected.size()) != 0)
        return nullptr;

    std::array<std::int16_t, kMaxFrameSamples * kChannels> pcm{};
    const int decoded = opus_decode(
        gDecoder,
        reinterpret_cast<const unsigned char*>(packet.data() + kHeaderSize),
        payloadSize,
        pcm.data(),
        kMaxFrameSamples,
        0);

    if (decoded <= 0)
        return nullptr;

    const jsize samples = static_cast<jsize>(decoded * kChannels);
    auto result = env->NewShortArray(samples);
    if (!result)
        return nullptr;
    env->SetShortArrayRegion(result, 0, samples, pcm.data());
    return result;
}
