#pragma once

#include <include/cef_app.h>

class MiniCefApp : public CefApp, public CefBrowserProcessHandler {
public:
    MiniCefApp() = default;

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    void OnBeforeCommandLineProcessing(const CefString& processType, CefRefPtr<CefCommandLine> commandLine) override;

private:
    IMPLEMENT_REFCOUNTING(MiniCefApp);
};