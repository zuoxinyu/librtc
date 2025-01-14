//
//  video_encoder.h
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#ifndef librtc_video_encoder_h
#define librtc_video_encoder_h
#pragma once

#include "api/video_codecs/video_encoder_factory.h"

class IVideoEncoderFactory
: public webrtc::VideoEncoderFactory
{
public:
    IVideoEncoderFactory();
    static std::unique_ptr<IVideoEncoderFactory> Create();
    
    // Returns a list of supported video formats in order of preference, to use
    // for signaling etc.
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const;
    
    // Creates a VideoEncoder for the specified format.
    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& format);
private:
    std::unique_ptr<webrtc::VideoEncoderFactory> _factory;
    std::vector<webrtc::SdpVideoFormat> _formats;
};

#endif  // librtc_video_encoder_h
