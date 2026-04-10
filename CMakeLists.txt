cmake_minimum_required(VERSION 3.26.0)
project(mini-player VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

add_subdirectory(${CEF_ROOT}/libcef_dll)

file(GLOB_RECURSE SOURCES
    "src/*.cc" "src/*.h" 
    "src/*.cpp" "src/*.hpp"
)
set(GLAD_SOURCE 
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/gl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/egl.c"
)
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${GLAD_SOURCE}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libswscale libavutil)

find_package(X11 REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

set(GLAD_INCLUDE external/glad/include)
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CEF_ROOT}
    ${X11_INCLUDE_DIR}
    ${FFMPEG_INCLUDE_DIRS}
    ${GLAD_INCLUDE}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    libcef_dll_wrapper
    "${CEF_ROOT}/Release/libcef.so"
    ${CEF_STANDARD_LIBS}
    ${X11_LIBRARIES}
    EGL
    OpenGL::GL
    ${FFMPEG_LIBRARIES}
    pthread
)

set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "$ORIGIN")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CEF_ROOT}/Resources"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Syncing CEF Resources (fast)..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CEF_ROOT}/Release"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "Syncing CEF Binaries (fast)..."
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
    COMMENT "Syncing project resources (fast)..."
)