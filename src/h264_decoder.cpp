//
//  h264_decoder.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/3/16.
//

#include "h264_decoder.h"
#include "libyuv.h"
#include "frame.h"

std::vector<webrtc::SdpVideoFormat> H264Decoder::GetSupportedFormats()
{
    return supported_h264_codecs(true);
}

std::unique_ptr<H264Decoder> H264Decoder::Create()
{
    return std::make_unique<H264Decoder>();
}

bool H264Decoder::Configure(const Settings& settings)
{
    _layer = find_codec(CodecKind::kDecoder);
    if (!_layer.codec)
    {
        return false;
    }
    
    _ctx = avcodec_alloc_context3(_layer.codec);
    if (_ctx == NULL)
    {
        return false;
    }
    
    if (avcodec_open2(_ctx, _layer.codec, NULL) != 0)
    {
        return false;
    }
    
    _packet = av_packet_alloc();
    if (_packet == NULL)
    {
        return false;
    }
    
    _frame = av_frame_alloc();
    return _frame != NULL;
}

int32_t H264Decoder::Decode(const webrtc::EncodedImage& input_image,
                            bool _missing_frames,
                            int64_t render_time_ms)
{
    if (!_layer.codec)
    {
        return CodecRet::Err;
    }
    
    _packet->data = (uint8_t*)input_image.data();
    _packet->size = (int)input_image.size();
    
    if (avcodec_send_packet(_ctx, _packet) != 0)
    {
        return CodecRet::Err;
    }
    
    while (true)
    {
        if (_ReadFrame() != 0)
        {
            break;
        }
    }
    
    return CodecRet::Ok;
}

int32_t H264Decoder::RegisterDecodeCompleteCallback(webrtc::DecodedImageCallback* callback)
{
    _callback = callback;
    return CodecRet::Ok;
}

int32_t H264Decoder::Release()
{
    avcodec_send_frame(_ctx, NULL);
    avcodec_free_context(&_ctx);
    av_packet_free(&_packet);
    av_frame_free(&_frame);
    return CodecRet::Ok;
}

int H264Decoder::_ReadFrame()
{
    if (avcodec_receive_frame(_ctx, _frame) != 0)
    {
        return -1;
    }
    
    if (!_i420_buffer)
    {
        int stride_u = _frame->format == AV_PIX_FMT_NV12 ? _frame->linesize[1] / 2 : _frame->linesize[1];
        int stride_v = _frame->format == AV_PIX_FMT_NV12 ? stride_u : _frame->linesize[2];
        _i420_buffer = webrtc::I420Buffer::Create(_frame->width,
                                                  _frame->height,
                                                  _frame->linesize[0],
                                                  stride_u,
                                                  stride_v);
    }
    
    if (_frame->format == AV_PIX_FMT_NV12)
    {
        libyuv::NV12ToI420(_frame->data[0],
                           _frame->linesize[0],
                           _frame->data[1],
                           _frame->linesize[1],
                           (uint8_t*)_i420_buffer->DataY(),
                           _i420_buffer->StrideY(),
                           (uint8_t*)_i420_buffer->DataU(),
                           _i420_buffer->StrideU(),
                           (uint8_t*)_i420_buffer->DataV(),
                           _i420_buffer->StrideV(),
                           _frame->width,
                           _frame->height);
    }
    else
    {
        _i420_buffer->Copy(_frame->width,
                           _frame->height,
                           _frame->data[0],
                           _frame->linesize[0],
                           _frame->data[1],
                           _frame->linesize[1],
                           _frame->data[2],
                           _frame->linesize[2]);
    }
    
    int64_t time_ms = _frame->pts * (_frame->time_base.num * 1000 / _frame->time_base.den);
    webrtc::VideoFrame frame(_i420_buffer,
                             webrtc::kVideoRotation_0,
                             time_ms * rtc::kNumMicrosecsPerMillisec);
    _callback->Decoded(frame);
    return 0;
}
