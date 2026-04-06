# Math SDK for JeanClaude StakeEngine

**Author:** ambroiselebs
**Studio:** JeanClaude StakeEngine

## Description

Math SDK is a high-performance mathematical file generation tool for the JeanClaude StakeEngine. It features a modern web-based graphical interface (Vue.js 3) powered by a native C++ backend. 

The application is cross-platform, using **WebKit2GTK** on Linux and **Microsoft WebView2 (Edge)** on Windows to provide a seamless desktop experience with a reactive web UI.

## Why C++ & WebViewer?

- **Performance**: 100,000 simulations executed in milliseconds thanks to the C++17 engine.
- **Modern UI**: A reactive and beautiful interface built with **Vue.js 3**, providing a much better UX than traditional desktop toolkits.
- **Real-time**: Instant RTP (Return to Player) calculation and statistical feedback as you edit weights and values.
- **Cross-Platform**: Native performance and look-and-feel on both Linux and Windows.

## Key Features

- ✅ **Dynamic Mode Editor**: Create and manage multiple game modes with custom costs.
- ✅ **Multiplier Management**: Fine-tune multiplier values and their respective weights with real-time probability calculation.
- ✅ **Free Spins Engine**: Configure trigger weights, spin counts, multiplier boosts, and retrigger logic.
- ✅ **Real-time Simulations**: Run millions of iterations to validate your mathematical model.
- ✅ **RTP & Volatility Analytics**: Automatic calculation of expected RTP, volatility index, hit frequency, variance, and payout distribution.
- ✅ **Persistence**: Import and Export your configurations in standard JSON format.
- ✅ **Stake Engine Export**: One-click generation of CSV, compressed JSONL (zstd), and index files ready for engine integration.

## Prerequisites

### Linux (Ubuntu/Debian)

- C++ compiler supporting C++17 (`g++` or `clang++`)
- `make`
- System libraries:
  - `libzstd` (compression)
  - `gtk+-3.0`
  - `webkit2gtk-4.1` (or 4.0)

```bash
sudo apt update
sudo apt install build-essential pkg-config libzstd-dev libgtk-3-dev libwebkit2gtk-4.1-dev
```

### Windows

- **Visual Studio 2019+** or **Build Tools for Visual Studio** (with C++ Desktop development workload).
- **LLVM/Clang** (recommended for best compatibility with the Makefile).
- **Make for Windows** (available via Chocolatey, Scoop, or MSYS2).
- **Important**: You **must** run the PowerShell setup script once to install the required libraries (`WebView2` and `zstd`) before building.

## Setup & Compilation

### 1. Windows Dependency Setup (Mandatory)
Before the first build on Windows, you must download the required SDKs by running:

```powershell
powershell -ExecutionPolicy Bypass -File setup-windows.ps1
```
*(This will download and extract WebView2 and zstd into the `libs/` folder).*

### 2. Build the Application

**On Linux:**
```bash
make
```

**On Windows:**
Do **not** use `make` directly. Use the provided batch script which correctly initializes the MSVC environment:
```cmd
build.bat
```

### 3. Running

```bash
# Linux
./math-engine

# Windows
math-engine.exe
```

## Project Structure

```
.
├── Makefile              # Cross-platform build system
├── build.bat             # Windows build wrapper (vcvarsall + make)
├── setup-windows.ps1     # Windows dependency downloader
├── libs/                 # [Windows only] Compiled libraries & headers
│   ├── webview2/         # Microsoft Edge WebView2 SDK
│   └── zstd/             # Zstd compression library
├── includes/             # C++ Headers
│   ├── webview.h         # Cross-platform WebViewer wrapper
│   ├── AppBridge.hpp     # JS <-> C++ Bridge logic
│   ├── ModeManager.hpp   # Business logic manager
│   └── Distribution.hpp  # Math engine core
├── srcs/                 # C++ Source files
│   ├── main.cpp          # Entry point
│   ├── AppBridge.cpp     # Bridge implementation
│   ├── ModeManager.cpp   
│   ├── Distribution.cpp
│   └── win_stubs.cpp     # Windows-specific entry point logic
├── frontend/             # Web UI (Vue.js 3)
│   ├── index.html        # Main structure
│   ├── app.js            # Vue.js logic & IPC bridge
│   └── style.css         # Modern dark theme
└── output/               # Generated engine files
```

## License

Private project - All rights reserved

---

**Made with ⚡ by ambroiselebs @ JeanClaude**
