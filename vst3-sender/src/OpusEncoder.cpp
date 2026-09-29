#include "vstream/OpusEncoder.h"
#include <opus.h>
namespace vstream {
struct OpusEncoder::Impl { ::OpusEncoder* encoder=nullptr; int channels=2; };
OpusEncoder::OpusEncoder():impl_(std::make_unique<Impl>()){}
OpusEncoder::~OpusEncoder(){ if(impl_->encoder) opus_encoder_destroy(impl_->encoder); }
bool OpusEncoder::open(int sampleRate,int channels,int bitrate){
    if(impl_->encoder) opus_encoder_destroy(impl_->encoder);
    int error=OPUS_OK;
    impl_->channels=channels;
    impl_->encoder=opus_encoder_create(sampleRate,channels,OPUS_APPLICATION_AUDIO,&error);
    if(error!=OPUS_OK||!impl_->encoder)return false;
    opus_encoder_ctl(impl_->encoder,OPUS_SET_BITRATE(bitrate));
    opus_encoder_ctl(impl_->encoder,OPUS_SET_COMPLEXITY(5));
    return true;
}
bool OpusEncoder::encode(const float* interleaved,int frames,std::vector<std::uint8_t>& packet){
    if(!isOpen()||frames<=0)return false;
    packet.resize(4000);
    const auto n=opus_encode_float(impl_->encoder,interleaved,frames,packet.data(),static_cast<opus_int32>(packet.size()));
    if(n<0)return false;
    packet.resize(static_cast<std::size_t>(n));
    return true;
}
bool OpusEncoder::isOpen()const noexcept{return impl_->encoder!=nullptr;}
}
