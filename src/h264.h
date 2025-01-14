//
//  h264.h
//  rtc
//
//  Created by Mr.Panda on 2023/3/15.
//

#ifndef librtc_h264_h
#define librtc_h264_h
#pragma once

#include "modules/video_coding/codecs/h264/include/h264.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

enum CodecKind
{
    kEncoder,
    kDecoder,
};

enum CodecRet
{
    Ok = WEBRTC_VIDEO_CODEC_OK,
    Err = WEBRTC_VIDEO_CODEC_ERROR,
};

static std::string Decoders[] = {
    "h264_cuvid",
    "h264_qsv",
    "h264_videotoolbox",
    "libx264"
};

static std::string Encoders[] = {
    "h264_nvenc",
    "h264_qsv",
    "h264_videotoolbox",
#ifdef WEBRTC_IOS
    "libx264",
#endif
};

typedef struct
{
    const AVCodec* codec;
    std::string name;
} CodecLayer;

constexpr webrtc::ScalabilityMode IkSupportedScalabilityModes[] = {
    webrtc::ScalabilityMode::kL1T1,
    webrtc::ScalabilityMode::kL1T2,
    webrtc::ScalabilityMode::kL1T3
};

CodecLayer find_codec(CodecKind kind);
webrtc::SdpVideoFormat create_h264_format(webrtc::H264Profile profile,
                                         webrtc::H264Level level,
                                         const std::string& packetization_mode,
                                         bool add_scalability_modes);
std::vector<webrtc::SdpVideoFormat> supported_h264_codecs(bool mode /* add_scalability_modes */);

#endif /* librtc_h264_h */
