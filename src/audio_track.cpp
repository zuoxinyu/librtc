//
//  audio_track.cpp
//  rtc
//
//  Created by Mr.Panda on 2023/2/21.
//

#include "audio_track.h"

/* IAudioTrackSink */

IAudioTrackSink::IAudioTrackSink(webrtc::AudioTrackInterface* track)
{
    _track = track;
    _track->AddRef();
}

IAudioTrackSink* IAudioTrackSink::Create(webrtc::AudioTrackInterface* track)
{
    auto self = new rtc::RefCountedObject<IAudioTrackSink>(track);
    self->AddRef();
    return self;
}

void IAudioTrackSink::OnData(const void* audio_data,
                             int bits_per_sample,
                             int sample_rate,
                             size_t number_of_channels,
                             size_t number_of_frames,
                             absl::optional<int64_t> capture_timestamp_ms)
{
    if (!_handler || !audio_data)
    {
        return;
    }
    
    auto frames = into_c((const int16_t*)audio_data,
                         sample_rate,
                         number_of_channels,
                         number_of_frames,
                         capture_timestamp_ms.value_or(0));
    _handler(_ctx, frames);
}

void IAudioTrackSink::SetOnFrame(void* ctx,
                                 void(*handler)(void* ctx, IAudioFrame* frame))
{
    _track->AddSink(this);
    _handler = handler;
    _ctx = ctx;
}

void IAudioTrackSink::RemoveOnFrame()
{
    _track->RemoveSink(this);
    _handler = NULL;
    _ctx = NULL;
}

/* IAudioTrackSource */

IAudioTrackSource* IAudioTrackSource::Create()
{
    auto self = new rtc::RefCountedObject<IAudioTrackSource>();
    self->AddRef();
    return self;
}

void IAudioTrackSource::RegisterObserver(webrtc::ObserverInterface* observer)
{
    observer->OnChanged();
}

void IAudioTrackSource::AddSink(webrtc::AudioTrackSinkInterface* sink)
{
    _sinks.insert(sink);
}

void IAudioTrackSource::RemoveSink(webrtc::AudioTrackSinkInterface* sink)
{
    _sinks.erase(sink);
}

const cricket::AudioOptions IAudioTrackSource::options() const
{
    cricket::AudioOptions options;
#if defined(WEBRTC_IOS)
    options.ios_force_software_aec_HACK = true;
#endif
    options.echo_cancellation = true;
    options.auto_gain_control = true;
    options.noise_suppression = true;
    return options;
}

webrtc::AudioSourceInterface::SourceState IAudioTrackSource::state() const
{
    return SourceState::kLive;
}

bool IAudioTrackSource::remote() const
{
    return false;
}

void IAudioTrackSource::OnData(IAudioFrame* frame)
{
    for (auto &sink: _sinks)
    {
        sink->OnData(frame->data,
                     16,
                     frame->sample_rate,
                     frame->channels,
                     frame->frames,
                     frame->timestamp);
    }
}
