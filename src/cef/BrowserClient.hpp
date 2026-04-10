#pragma once

#include <mutex>
#include <vector>
#include <atomic>

#include "include/cef_client.h"
#include "include/cef_render_handler.h"

#include "cef/OSRRenderHandler.hpp"
#include "core/SharedBuffer.hpp"

class BrowserClient : public CefClient {
public:
    BrowserClient(CefRefPtr<OSRRenderHandler> renderHandler);

    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return _renderHandler; }
    
    bool getFrameData(std::vector<uint8_t>& outBuffer);

private:
    CefRefPtr<OSRRenderHandler> _renderHandler;

    IMPLEMENT_REFCOUNTING(BrowserClient);
};