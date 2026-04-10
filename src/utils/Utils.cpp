#include "Utils.hpp"

#include <filesystem>
#include <fstream>

namespace utils {

std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("utils::loadShaderSource: Failed to open shader file: " + filepath);
    }

    const auto fileSize = std::filesystem::file_size(filepath);
    if (fileSize == 0) {
        throw std::runtime_error("utils::loadShaderSource: Shader file is empty: " + filepath);
    }

    std::string buffer(fileSize, '\0');
    file.read(buffer.data(), fileSize);
    
    return buffer;
}

} // namespace utils