//
//  h264_encoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/10.
//

#ifndef h264_encoder_h
#define h264_encoder_h
#pragma once

#include "modules/video_coding/codecs/h264/include/h264.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "api/create_peerconnection_factory.h"
#include "common_video/h264/h264_common.h"
#include <optional>
extern "C" {
#include "libavcodec/avcodec.h"
}

class H264EncoderLayer
{
public:
    void Release()
    {
        avcodec_send_frame(ctx, NULL);
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&ctx);
    }
    
    std::string name;
    webrtc::H264PacketizationMode pkt_mode;
    webrtc::EncodedImage image;
    uint8_t simulcast_idx;
    const AVCodec* codec;
    AVCodecContext* ctx;
    AVPacket* packet;
    AVFrame* frame;
};

class H264Encoder
: public webrtc::VideoEncoder
{
public:
    H264Encoder(const webrtc::SdpVideoFormat& format): _format(format) {}
    static std::vector<webrtc::SdpVideoFormat> GetSupportedFormats();
    static std::unique_ptr<H264Encoder> Create(const webrtc::SdpVideoFormat& format);
    int InitEncode(const webrtc::VideoCodec* codec_settings, const Settings& settings);
    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* callback);
    int32_t Encode(const webrtc::VideoFrame& frame,
                   const std::vector<webrtc::VideoFrameType>* frame_types);
    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters);
    int32_t Release();
private:
    int _OnFrame(int index,
                 webrtc::VideoFrameType frame_type,
                 const uint8_t* frame_buf,
                 int width,
                 int height,
                 size_t len);
    int _ReadPacket(int index,
                    webrtc::VideoFrameType frame_type,
                    const webrtc::VideoFrame& frame);
    std::optional<H264EncoderLayer> _OpenEncoder(const webrtc::SimulcastStream* stream,
                                                 int stream_idx);
    
    const webrtc::VideoCodec* _codec_settings = nullptr;
    webrtc::EncodedImageCallback* _callback = nullptr;
    webrtc::H264BitstreamParser _h264_bitstream_parser;
    std::vector<H264EncoderLayer> _encoders;
    const webrtc::SdpVideoFormat& _format;
};

#endif /* h264_encoder_h */
