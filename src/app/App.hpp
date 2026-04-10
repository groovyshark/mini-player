#pragma once

#include <thread>
#include <memory>

#include "window/EGLWindow.hpp"
#include "renderer/Renderer.hpp"
#include "core/SharedBuffer.hpp"

#include "cef/BrowserClient.hpp"
#include "cef/OSRRenderHandler.hpp"

#include "video/VideoDecoder.hpp"

#include "utils/Constants.hpp"

class App {
public:
    App();
    ~App();

    void init();
    void run();

private:
    void initVideoContext();
    void initCEFContext();

    void update();

    void videoThreadFunc();

    void shutdown();
    void shutdownVideoThread();
    void shutdownCEFContext();

    void calculateLetterbox(int windowWidth, int windowHeight);

private:
    void onWindowResized(int width, int height);

private:
    EGLWindow _window;
    int _currentWindowWidth{0};
    int _currentWindowHeight{0};

    Renderer _renderer;

    CefRefPtr<OSRRenderHandler> _renderHandler;
    CefRefPtr<BrowserClient> _browserClient;
    CefRefPtr<CefBrowser> _browser;

    VideoDecoder _decoder;
    
    SharedBufferPtr _cefBuffer;
    SharedBufferPtr _videoBuffer;

    std::thread _videoThread;
    std::atomic<bool> _isVideoRunning{true};
    int _videoFrameSize{0};
    float _videoScaleX{1.0f};
    float _videoScaleY{1.0f};

    int _cefWidth;
    int _cefHeight;
};