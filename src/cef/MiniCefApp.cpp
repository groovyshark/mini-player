#include "MiniCefApp.hpp"

#include "app/Config.hpp"

void MiniCefApp::OnBeforeCommandLineProcessing(const CefString &processType, CefRefPtr<CefCommandLine> commandLine) {
    if (config::AppConfig::getInstance().cefDisableGpu) {
        commandLine->AppendSwitch("disable-gpu");
        commandLine->AppendSwitch("disable-gpu-compositing");
        commandLine->AppendSwitch("disable-software-rasterizer");
    }
    if (config::AppConfig::getInstance().cefNoSandbox) {
        commandLine->AppendSwitch("no-sandbox");
        commandLine->AppendSwitch("no-zygote");
    }
    if (config::AppConfig::getInstance().cefDisableNetworking) {
        commandLine->AppendSwitch("disable-background-networking");
        commandLine->AppendSwitch("disable-features=NetworkService,NetworkServiceInProcess");
    }
}