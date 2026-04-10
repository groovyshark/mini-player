#include "OSRRenderHandler.hpp"

OSRRenderHandler::OSRRenderHandler(SharedBufferPtr buffer, int width, int height)
    : _sharedBuffer(buffer), _width(width), _height(height)
{
}

void OSRRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) {
    rect = CefRect(0, 0, _width, _height);
}

void OSRRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                 const RectList& dirtyRects, const void* buffer,
                 int width, int height) 
{
    std::lock_guard<std::mutex> lock(_sharedBuffer->mutex);

    int size = width * height * 4;
    if (_sharedBuffer->data.size() != size) {
        _sharedBuffer->data.resize(size);
    }

    std::memcpy(_sharedBuffer->data.data(), buffer, size);
    _sharedBuffer->width = width;
    _sharedBuffer->height = height;
    _sharedBuffer->isDirty.store(true);
}