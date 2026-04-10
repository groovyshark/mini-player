#include "VideoDecoder.hpp"

#include <stdexcept>

VideoDecoder::~VideoDecoder() {
    cleanup();
}

void VideoDecoder::init(const std::string& videoFilePath) {
    if (avformat_open_input(&_formatContext, videoFilePath.c_str(), nullptr, nullptr) < 0) {
        throw std::runtime_error("VideoDecoder::init: Could not open file");
    }

    if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        throw std::runtime_error("VideoDecoder::init: Could not find stream info");
    }

    for (unsigned int i = 0; i < _formatContext->nb_streams; ++i) {
        if (_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            _videoStreamIndex = i;
            break;
        }
    }

    if (_videoStreamIndex == -1) {
        throw std::runtime_error("VideoDecoder::init: Could not find video stream");
    }

    initFps();

    AVCodecParameters* codecPar = _formatContext->streams[_videoStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecPar->codec_id);
    _codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(_codecContext, codecPar);
    
    if (avcodec_open2(_codecContext, codec, nullptr) < 0) {
        throw std::runtime_error("VideoDecoder::init: Could not open codec");
    }

    _frame = av_frame_alloc();
    _rgbFrame = av_frame_alloc();

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, _codecContext->width, _codecContext->height, 1);
    _buffer = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
    
    av_image_fill_arrays(
        _rgbFrame->data, 
        _rgbFrame->linesize, 
        _buffer, 
        AV_PIX_FMT_RGB24, 
        _codecContext->width, 
        _codecContext->height, 
        1
    );

    _swsContext = sws_getContext(
        _codecContext->width,
        _codecContext->height,
        _codecContext->pix_fmt,
        _codecContext->width,
        _codecContext->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );
}

bool VideoDecoder::nextFrame() {
    AVPacket* packet = av_packet_alloc();
    
    while (av_read_frame(_formatContext, packet) >= 0) {
        if (packet->stream_index == _videoStreamIndex) {
            avcodec_send_packet(_codecContext, packet);

            int ret = avcodec_receive_frame(_codecContext, _frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_packet_unref(packet);
                continue;
            } else if (ret < 0) {
                av_packet_unref(packet);
                throw std::runtime_error("VideoDecoder::nextFrame: Error during decoding");
            }

            sws_scale(
                _swsContext,
                _frame->data,
                _frame->linesize,
                0,
                _codecContext->height,
                _rgbFrame->data,
                _rgbFrame->linesize
            );

            av_packet_unref(packet);
            return true;
        }
        av_packet_unref(packet);
    }

    av_packet_free(&packet);

    return false;
}

void VideoDecoder::cleanup() {
    if (_buffer) av_free(_buffer);
    if (_rgbFrame) av_frame_free(&_rgbFrame);
    if (_frame) av_frame_free(&_frame);
    if (_swsContext) sws_freeContext(_swsContext);
    if (_codecContext) avcodec_free_context(&_codecContext);
    if (_formatContext) avformat_close_input(&_formatContext);
}

void VideoDecoder::initFps() {
    AVRational frameRate = _formatContext->streams[_videoStreamIndex]->avg_frame_rate;
    if (frameRate.den != 0 && frameRate.num != 0) {
        _fps = static_cast<double>(frameRate.num) / static_cast<double>(frameRate.den);
    } else {
        frameRate = _formatContext->streams[_videoStreamIndex]->r_frame_rate;
        if (frameRate.den != 0 && frameRate.num != 0) {
            _fps = static_cast<double>(frameRate.num) / static_cast<double>(frameRate.den);
        }
    }
}