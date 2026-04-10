#pragma once

#include <cstdint>
#include <string>

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;

    void init();
    
    void updateCEFTexture(const uint8_t* buffer, int width, int height);
    void updateVideoTexture(const uint8_t* buffer, int width, int height);

    void draw(float videoScaleX, float videoScaleY);

private:
    void setupGeometry();
    void compileShaders();

    uint32_t createDummyTexture(uint8_t r, uint8_t g, uint8_t b);

    void checkCompileErrors(uint32_t shader, const std::string& type);

private:
    uint32_t _cefTexture{0};
    uint32_t _videoTexture{0};

    uint32_t _shaderProgram{0};
    
    uint32_t _vao{0};
    uint32_t _vbo{0};

    int _videoWidth{0};
    int _videoHeight{0};

    int _cefWidth{0};
    int _cefHeight{0};
};