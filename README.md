# C++ Mini Media Player (CEF + FFmpeg + OpenGL)

A high-performance, multithreaded media engine built with C++20. This project demonstrates simultaneous hardware-accelerated rendering of a Chromium Embedded Framework (CEF) web interface and an FFmpeg-decoded video stream within a single raw OpenGL context.

## 🚀 Features

* **Split-Screen Rendering:** The upper half displays a fully interactive web UI (CEF Off-Screen Rendering), while the lower half plays a decoded video.
* **Multithreading & Synchronization:** A dedicated background thread handles FFmpeg video decoding with accurate PTS/timer synchronization, safely pushing frames to the main OpenGL thread via atomic `SharedBuffer` structures.
* **Dynamic Letterboxing:** Hardware-accelerated aspect ratio correction via GLSL shaders. The video scales correctly without distortion and reacts dynamically to X11/EGL window resize events.
* **Custom Configuration:** Fully controllable via command-line arguments (bypass CEF sandbox, toggle GPU acceleration, specify media files or URLs).

## 🛠 Tech Stack

* **Language:** C++20
* **Build System:** CMake (3.26+)
* **Graphics Pipeline:** OpenGL 3.3 Core, GLAD, EGL / X11
* **Video Decoding:** FFmpeg (libavformat, libavcodec, libswscale, libavutil)
* **Web Engine:** CEF (Chromium Embedded Framework)

## 🏗 Architecture Overview

1. **`App` (Orchestrator):** Manages the main render loop, polls X11 window events, and orchestrates resource sharing via a centralized Meyer's Singleton `AppConfig`.
2. **`VideoDecoder`:** Runs in a separate `std::thread`. Decodes `.mp4` frames into RGB24 format using `sws_scale` and respects video framerate timings to prevent out-of-sync playback.
3. **`OSRRenderHandler`:** Intercepts BGRA pixel buffers from CEF's invisible browser instance.
4. **`SharedBuffer`:** A thread-safe data structure using `std::mutex` and `std::atomic<bool>` to transfer dirty frames from suppliers (CEF/FFmpeg) to the Renderer.
5. **`Renderer`:** Uploads pixels to VRAM via `glTexSubImage2D` and draws textured quads using custom vertex/fragment shaders.

## ⚙️ Build Instructions

### Prerequisites
Ensure you have the following dependencies installed on your Linux environment (e.g., Ubuntu/WSL2):

**CEF Setup**

This project requires the Chromium Embedded Framework (CEF) Standard Distribution for Linux 64-bit.
1. Download the Standard Distribution (e.g., version 114.0.0 or similar depending on your build) from [Spotify's Open Source CEF Builds](https://cef-builds.spotifycdn.com/index.html).

2. Extract the archive.

3. Rename the extracted folder to cef and place it inside the `external/` directory of this project (`external/cef/`).

4. Ensure **`external/cef/CMakeLists.txt`** exists at that path.

**Libraries**


```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libx11-dev libegl1-mesa-dev
sudo apt install libavformat-dev libavcodec-dev libswscale-dev libavutil-dev
```

### Build Steps

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

*The CMake script is configured to automatically sync CEF resources, CEF binaries, and project assets (`res/` folder) to the output directory using the `copy_directory_if_different` command.*

## 🎮 Usage & Command-Line Arguments

Run the executable directly from the build directory. By default, it will load `res/video/test.mp4` and `res/web/index.html`.

```bash
./mini-player [OPTIONS]
```

### Available Keys:

| Argument | Description |
| :--- | :--- |
| `--video <path>` | Path to the video file to decode (e.g., `--video res/movie.mp4`). |
| `--url <url>` | URL or absolute local file path for CEF to load. |
| `--no-letterbox` | Disables aspect-ratio correction; forces the video to stretch to the window bounds. |
| `--enable-gpu` | Re-enables CEF GPU compositing (disabled by default for Linux/WSL stability). |
| `--enable-sandbox` | Re-enables Chromium sandbox (requires specific OS-level setup). |
| `--enable-networking` | Enables Chromium background networking and SSL checks. |

**Example usage with arguments:**

```bash
./mini-player --video "/home/user/videos/sample.mp4" --url "[https://webglsamples.org/aquarium/aquarium.html](https://webglsamples.org/aquarium/aquarium.html)" --no-letterbox
```

## 🐛 Troubleshooting (Linux / WSL2)

If the application crashes on startup with `Failed global descriptor lookup: 7` or SSL handshake errors, ensure you are running without the `--enable-sandbox` and `--enable-networking` flags. The default configuration uses a `MiniCefApp` wrapper to inject stability flags (like `no-zygote`) specifically tailored for headless Linux environments.