#pragma once

#include <memory>

#include "include/cef_render_handler.h"

#include "core/SharedBuffer.hpp"

class OSRRenderHandler : public CefRenderHandler {
public:
    OSRRenderHandler(SharedBufferPtr buffer, int width, int height);

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
    
    void OnPaint(
        CefRefPtr<CefBrowser> browser, 
        PaintElementType type,
        const RectList& dirtyRects, 
        const void* buffer,
        int width, int height) override;
    
    SharedBufferPtr getSharedBuffer() const { return _sharedBuffer; }
    
private:
    SharedBufferPtr _sharedBuffer;

    int _width; 
    int _height;

    IMPLEMENT_REFCOUNTING(OSRRenderHandler);
};