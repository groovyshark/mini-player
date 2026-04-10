#pragma once

#include <functional>

#include <EGL/egl.h>
#include <X11/Xlib.h>

class EGLWindow {
public:
    using ResizeCallback = std::function<void(int width, int height)>;

    EGLWindow(int width, int height);
    ~EGLWindow();

    void init();

    void pollEvents();
    void swapBuffers();

    void setResizeCallback(ResizeCallback callback) {
        _resizeCallback = callback;
    }

    bool shouldClose() const { return _shouldClose; };

    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

private:
    void shutdown();

private:
    Display* _display{nullptr};
    Window _window;

    EGLDisplay _eglDisplay{EGL_NO_DISPLAY};
    EGLSurface _eglSurface{EGL_NO_SURFACE};
    EGLContext _eglContext{EGL_NO_CONTEXT};
    
    int _width;
    int _height;
    bool _shouldClose{false};

    ResizeCallback _resizeCallback;
};