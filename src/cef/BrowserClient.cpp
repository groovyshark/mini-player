#include "BrowserClient.hpp"

BrowserClient::BrowserClient(CefRefPtr<OSRRenderHandler> renderHandler) : 
    _renderHandler(renderHandler) 
{
}

bool BrowserClient::getFrameData(std::vector<uint8_t>& outBuffer) {
    if (_renderHandler->getSharedBuffer()->isDirty.exchange(false)) {
        std::lock_guard<std::mutex> lock(_renderHandler->getSharedBuffer()->mutex);

        if (outBuffer.size() != _renderHandler->getSharedBuffer()->data.size()) {
            outBuffer.resize(_renderHandler->getSharedBuffer()->data.size());
        }
        
        std::memcpy(outBuffer.data(), _renderHandler->getSharedBuffer()->data.data(), _renderHandler->getSharedBuffer()->data.size());
        return true;
    }

    return false;
}