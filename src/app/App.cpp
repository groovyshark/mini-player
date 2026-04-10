#include "App.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>

#include "include/cef_app.h"
#include <glad/gl.h>

#include "app/Config.hpp"
#include "utils/Constants.hpp"

App::App() : _window(utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT),
             _cefWidth(_window.getWidth()),
             _cefHeight(_window.getHeight() / 2) {
}

App::~App() {
    shutdown();
}

void App::shutdown() {
    shutdownVideoThread();
    shutdownCEFContext();
}

void App::shutdownVideoThread() {
    _isVideoRunning = false;
    if (_videoThread.joinable()) {
        _videoThread.join();
    }
}

void App::shutdownCEFContext() {
    if (_browser.get()) {
        _browser->GetHost()->CloseBrowser(true);
        _browser = nullptr;
    }

    _browserClient = nullptr;

    for (int i = 0; i < 10; ++i) {
        CefDoMessageLoopWork();
    }
}

void App::init() {
    _window.init();
    _window.setResizeCallback([this](int w, int h) {
        this->onWindowResized(w, h);
    });

    _currentWindowWidth = _window.getWidth();
    _currentWindowHeight = _window.getHeight();

    _renderer.init();

    initVideoContext();
    initCEFContext();
}

void App::initVideoContext() {
    std::string videoPath = config::AppConfig::getInstance().videoPath;
    if (videoPath.empty()) {
        videoPath = "res/video/test.mp4";
    }
    _decoder.init(videoPath);

    _videoThread = std::thread(&App::videoThreadFunc, this);

    _videoBuffer = std::make_shared<SharedBuffer>();
    _videoFrameSize = _decoder.getWidth() * _decoder.getHeight() * 3;
    _videoBuffer->data.resize(_videoFrameSize);
    _videoBuffer->width = _decoder.getWidth();
    _videoBuffer->height = _decoder.getHeight();

    calculateLetterbox(_currentWindowWidth, _currentWindowHeight);
}

void App::initCEFContext() {
    _cefBuffer = std::make_shared<SharedBuffer>();
    _cefBuffer->data.resize(_cefWidth * _cefHeight * 4);
    _cefBuffer->width = _cefWidth;
    _cefBuffer->height = _cefHeight;

    _renderHandler = new OSRRenderHandler(_cefBuffer, _cefWidth, _cefHeight);
    _browserClient = new BrowserClient(_renderHandler);

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(0);

    CefBrowserSettings browserSettings;
    browserSettings.windowless_frame_rate = 60;

    std::string urlToLoad = config::AppConfig::getInstance().cefUrl;
    std::string finalUrl;

    std::string url = config::getConstructedUrl(config::AppConfig::getInstance().cefUrl);
    _browser = CefBrowserHost::CreateBrowserSync(windowInfo, _browserClient,
                                                 url, browserSettings,
                                                 nullptr, nullptr);
}

void App::run() {
    glViewport(0, 0, utils::WINDOW_WIDTH, utils::WINDOW_HEIGHT);

    while (!_window.shouldClose()) {
        _window.pollEvents();

        update();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _renderer.draw(_videoScaleX, _videoScaleY);

        _window.swapBuffers();
    }
}

void App::update() {
    CefDoMessageLoopWork();

    if (_cefBuffer->isDirty.exchange(false)) {
        std::lock_guard<std::mutex> lock(_cefBuffer->mutex);

        _renderer.updateCEFTexture(
            _cefBuffer->data.data(),
            _cefBuffer->width,
            _cefBuffer->height);
    }

    if (_videoBuffer->isDirty.exchange(false)) {
        std::lock_guard<std::mutex> lock(_videoBuffer->mutex);

        _renderer.updateVideoTexture(
            _videoBuffer->data.data(),
            _videoBuffer->width,
            _videoBuffer->height);
    }
}

void App::videoThreadFunc() {
    double fps = _decoder.getFps();
    double targetFrameDurationMs = 1000.0 / fps;

    while (_isVideoRunning) {
        auto startTime = std::chrono::high_resolution_clock::now();

        if (_decoder.nextFrame()) {
            std::lock_guard<std::mutex> lock(_videoBuffer->mutex);

            std::memcpy(_videoBuffer->data.data(), _decoder.getFrameData(), _videoFrameSize);
            _videoBuffer->isDirty.store(true);
        } else {
            std::cout << "App::videoThreadFunc: Video ended." << std::endl;
            break;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> decodeDuration = endTime - startTime;

        double sleepTimeMs = targetFrameDurationMs - decodeDuration.count();
        if (sleepTimeMs > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(sleepTimeMs));
        } else {
            // std::cerr << "Warning: Decoder is lagging behind!" << std::endl;
        }
    }
}

void App::calculateLetterbox(int windowWidth, int windowHeight) {
    if (!config::AppConfig::getInstance().useLetterbox || _decoder.getWidth() == 0)
        return;

    float zoneWidth = static_cast<float>(windowWidth);
    float zoneHeight = static_cast<float>(windowHeight) / 2.0f;

    float targetAspect = zoneWidth / zoneHeight;
    float videoAspect = static_cast<float>(_decoder.getWidth()) / static_cast<float>(_decoder.getHeight());

    if (videoAspect > targetAspect) {
        _videoScaleY = targetAspect / videoAspect;
    } else {
        _videoScaleX = videoAspect / targetAspect;
    }
}

void App::onWindowResized(int width, int height) {
    if (width == 0 || height == 0)
        return;

    glViewport(0, 0, width, height);

    calculateLetterbox(width, height);

    if (_browser) {
        _browser->GetHost()->WasResized();
    }
}