#include <iostream>

#include "include/cef_app.h"

#include "cef/MiniCefApp.hpp"
#include "app/App.hpp"
#include "app/Config.hpp"

int main(int argc, char* argv[]) {
    config::parseArgs(argc, argv);

    CefMainArgs mainArgs(argc, argv);
    CefRefPtr<MiniCefApp> app(new MiniCefApp);
    
    int exitCode = CefExecuteProcess(mainArgs, app, nullptr);
    if (exitCode >= 0) {
        return exitCode;
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = true;
    if (config::AppConfig::getInstance().cefNoSandbox) {
        settings.no_sandbox = true;
    }
    CefString(&settings.root_cache_path) = CefString("/tmp/cef_mini_player_cache");

    CefInitialize(mainArgs, settings, app, nullptr);

    {
        App app;
        app.init();
        app.run();
    }

    CefShutdown();
    return 0;
}
