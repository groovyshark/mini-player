#include "EGLWindow.hpp"

#include <iostream>
#include <stdexcept>

// #include <glad/egl.h>
#include <X11/Xutil.h>
#include <glad/gl.h>

EGLWindow::EGLWindow(int width, int height) : 
    _width(width),
    _height(height)
{
}

EGLWindow::~EGLWindow() {
    shutdown();
}

void EGLWindow::shutdown() {
    if (_eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (_eglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(_eglDisplay, _eglContext);
        }
        if (_eglSurface != EGL_NO_SURFACE) {
            eglDestroySurface(_eglDisplay, _eglSurface);
        }

        eglTerminate(_eglDisplay);
    }

    if (_display) {
        XDestroyWindow(_display, _window);
        XCloseDisplay(_display);
    }
}

void EGLWindow::init() {
    _display = XOpenDisplay(nullptr);
    if (!_display) {
        throw std::runtime_error("EGLWindow::init: Failed to open X display");
    }

    _eglDisplay = eglGetDisplay((EGLNativeDisplayType)_display);
    if (_eglDisplay == EGL_NO_DISPLAY) {
        throw std::runtime_error("EGLWindow::init: Failed to get EGL display");
    }

    EGLint major, minor;
    if (!eglInitialize(_eglDisplay, &major, &minor)) {
        throw std::runtime_error("EGLWindow::init: Failed to initialize EGL");
    }

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE};

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(_eglDisplay, configAttribs, &config, 1, &numConfigs) || numConfigs == 0) {
        throw std::runtime_error("EGLWindow::init: Failed to choose EGL config");
    }

    EGLint visualId;
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_VISUAL_ID, &visualId);

    XVisualInfo visualTemplate;
    visualTemplate.visualid = visualId;

    int numVisuals;
    XVisualInfo *visualInfo = XGetVisualInfo(_display, VisualIDMask, &visualTemplate, &numVisuals);
    if (!visualInfo) {
        throw std::runtime_error("EGLWindow::init: Failed to find matching X11 visual");
    }

    Window root = DefaultRootWindow(_display);
    XSetWindowAttributes windowAttribs;
    windowAttribs.colormap = XCreateColormap(_display, root, visualInfo->visual, AllocNone);
    windowAttribs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    _window = XCreateWindow(
        _display, root,
        0, 0, _width, _height, 0, // x, y, width, height, border_width
        visualInfo->depth, InputOutput,
        visualInfo->visual,
        CWColormap | CWEventMask, &windowAttribs);

    XFree(visualInfo);
    XMapWindow(_display, _window);

    _eglSurface = eglCreateWindowSurface(_eglDisplay, config, _window, NULL);
    if (_eglSurface == EGL_NO_SURFACE) {
        throw std::runtime_error("EGLWindow::init: Failed to create EGL surface");
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE};

    eglBindAPI(EGL_OPENGL_API);
    _eglContext = eglCreateContext(_eglDisplay, config, EGL_NO_CONTEXT, contextAttribs);
    if (_eglContext == EGL_NO_CONTEXT) {
        throw std::runtime_error("EGLWindow::init: Failed to create EGL context");
    }

    if (!eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
        throw std::runtime_error("EGLWindow::init: Failed to make EGL context current");
    }

    if (!gladLoadGL((GLADloadfunc)eglGetProcAddress)) {
        throw std::runtime_error("EGLWindow::init: Failed to initialize GLAD");
    }
}

void EGLWindow::swapBuffers() {
    eglSwapBuffers(_eglDisplay, _eglSurface);
}

void EGLWindow::pollEvents() {
    while (XPending(_display) > 0) {
        XEvent event;
        XNextEvent(_display, &event);

        if (event.type == ConfigureNotify) {
            int newWidth = event.xconfigure.width;
            int newHeight = event.xconfigure.height;

            if (_resizeCallback) {
                _resizeCallback(newWidth, newHeight);
            }
        }
        
        if (event.type == DestroyNotify) {
            _shouldClose = true;
        }
    }
}