#pragma once

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoDecoder {
public:
    VideoDecoder() = default;
    ~VideoDecoder();

    void init(const std::string& videoFilePath);

    bool nextFrame();

    const uint8_t* getFrameData() const { return _buffer; }
    int getWidth() const { return _codecContext->width; }
    int getHeight() const { return _codecContext->height; }

    double getFps() const { return _fps; }
private:
    void initFps();

    void cleanup();

private:
    AVFormatContext* _formatContext{nullptr};
    AVCodecContext* _codecContext{nullptr};
    AVFrame* _frame{nullptr};
    AVFrame* _rgbFrame{nullptr};

    uint8_t* _buffer{nullptr};

    SwsContext* _swsContext{nullptr};

    int _videoStreamIndex{-1};

    double _fps{30.0};
};