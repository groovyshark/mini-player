#include "Renderer.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <format>

#include <glad/gl.h>

#include "utils/Utils.hpp"

namespace {
float vertices[] = {
    // --- (CEF) ---
    -1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f,

    // --- (FFmpeg) ---
    -1.0f, -1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
    -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f};
} // namespace

void Renderer::init() {
    compileShaders();

    setupGeometry();

    _cefTexture = createDummyTexture(200, 50, 50);
    _videoTexture = createDummyTexture(50, 200, 50);
}

uint32_t Renderer::createDummyTexture(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    constexpr int bufferSize = 2 * 2 * 3;
    std::vector<uint8_t> buffer(bufferSize);
    for (int i = 0; i < bufferSize; i += 3) {
        buffer[i] = r;
        buffer[i + 1] = g;
        buffer[i + 2] = b;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

void Renderer::setupGeometry() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position x, y
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture coordinates u, v
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Renderer::compileShaders() {
    std::string vertCodeStr = utils::loadShaderSource("res/shaders/basic.vert");
    std::string fragCodeStr = utils::loadShaderSource("res/shaders/basic.frag");

    const char *vShaderCode = vertCodeStr.c_str();
    const char *fShaderCode = fragCodeStr.c_str();

    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    _shaderProgram = glCreateProgram();

    glAttachShader(_shaderProgram, vertexShader);
    glAttachShader(_shaderProgram, fragmentShader);
    glLinkProgram(_shaderProgram);
    checkCompileErrors(_shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::draw(float videoScaleX, float videoScaleY) {
    glUseProgram(_shaderProgram);
    glBindVertexArray(_vao);

    GLint scaleLoc = glGetUniformLocation(_shaderProgram, "uScale");
    GLint centerLoc = glGetUniformLocation(_shaderProgram, "uCenter");

    // draw CEF texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _cefTexture);
    glUniform2f(scaleLoc, 1.0f, 1.0f);
    glUniform2f(centerLoc, 0.0f, 0.5f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // draw ffmpeg texture
    glBindTexture(GL_TEXTURE_2D, _videoTexture);
    glUniform2f(scaleLoc, videoScaleX, videoScaleY);
    glUniform2f(centerLoc, 0.0f, -0.5f);
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
}

void Renderer::updateVideoTexture(const uint8_t* buffer, int width, int height) {
    if (!buffer || width == 0 || height == 0) return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _videoTexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (_videoWidth != width || _videoHeight != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

        _videoWidth = width;
        _videoHeight = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    }
}

void Renderer::updateCEFTexture(const uint8_t* buffer, int width, int height) {
    if (!buffer || width == 0 || height == 0) return;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _cefTexture);

    if (_cefWidth != width || _cefHeight != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);

        _cefWidth = width;
        _cefHeight = height;
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
    }
}

void Renderer::checkCompileErrors(uint32_t shader, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            throw std::runtime_error(std::format("Renderer::checkCompileErrors: SHADER_COMPILATION_ERROR of type: {}\n{}", type, infoLog));
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            throw std::runtime_error(std::format("Renderer::checkCompileErrors: PROGRAM_LINKING_ERROR\n{}", infoLog));
        }
    }
}