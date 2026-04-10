#pragma once

#include <string>
#include <filesystem>
#include <format>

namespace config {

struct AppConfig {
    std::string videoPath{"res/video/test.mp4"};
    std::string cefUrl{"res/web/index.html"};

    bool useLetterbox{true};

    bool cefDisableGpu{true};
    bool cefNoSandbox{true};
    bool cefDisableNetworking{true};

    static AppConfig &getInstance() {
        static AppConfig instance;
        return instance;
    }
};

inline void parseArgs(int argc, char* argv[]) {
    AppConfig& config = AppConfig::getInstance();
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if      (arg == "--video" && i + 1 < argc)  config.videoPath = argv[++i];
        else if (arg == "--url" && i + 1 < argc)    config.cefUrl = argv[++i];
        else if (arg == "--no-letterbox")           config.useLetterbox = false;
        else if (arg == "--enable-gpu")             config.cefDisableGpu = false;
        else if (arg == "--enable-sandbox")         config.cefNoSandbox = false;
        else if (arg == "--enable-networking")      config.cefDisableNetworking = false;
    }
}

inline std::string getConstructedUrl(std::string urlToLoad) {
    if (urlToLoad.empty()) { // default URL
        return std::format("file://{}", std::filesystem::absolute("res/web/index.html").string());
    } else if (urlToLoad.starts_with("http://") || // redy to use URL
               urlToLoad.starts_with("https://") ||
               urlToLoad.starts_with("file://")) {
        return urlToLoad;
    } else { // custom path, like --url "test/my_ui.html"
        return std::format("file://{}", std::filesystem::absolute(urlToLoad).string());
    }

}

}