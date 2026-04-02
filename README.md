# Math SDK for JeanClaude StakeEngine

**Author:** ambroiselebs
**Studio:** JeanClaude StakeEngine

## Description

Math SDK is a high-performance mathematical file generation tool for the JeanClaude StakeEngine. It features a modern web-based graphical interface (Vue.js) powered by a native C++ backend (WebKit2GTK). It allows you to create, simulate, and analyze game modes with complex multiplier distributions and free spin mechanics.

## Why C++ & WebViewer?

- **Performance**: 100,000 simulations executed in milliseconds thanks to the C++17 engine.
- **Modern UI**: A reactive and beautiful interface built with **Vue.js 3**, providing a much better UX than traditional desktop toolkits.
- **Real-time**: Instant RTP (Return to Player) calculation and statistical feedback as you edit weights and values.

## Key Features

- ✅ **Dynamic Mode Editor**: Create and manage multiple game modes with custom costs.
- ✅ **Multiplier Management**: Fine-tune multiplier values and their respective weights with real-time probability calculation.
- ✅ **Free Spins Engine**: Configure trigger weights, spin counts, multiplier boosts, and retrigger logic.
- ✅ **Real-time Simulations**: Run millions of iterations to validate your mathematical model.
- ✅ **RTP Analytics**: Automatic calculation of expected RTP with visual health indicators.
- ✅ **Persistence**: Import and Export your configurations in standard JSON format.
- ✅ **Stake Engine Export**: One-click generation of CSV, compressed JSONL (zstd), and index files ready for engine integration.

## Prerequisites

- C++ compiler supporting C++17 (g++, clang++)
- Make
- System libraries:
  - `libzstd` (compression)
  - `gtk+-3.0`
  - `webkit2gtk-4.1`

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential pkg-config libzstd-dev libgtk-3-dev libwebkit2gtk-4.1-dev
```

## Compilation & Execution

Build the application:
```bash
make
```

Run the application:
```bash
./math-engine
```

Clean compiled files:
```bash
make clean      # Remove object files
make fclean     # Remove binaries and temporary output
```

## Usage

### 1. Game Modes
Click **+ New Mode** to add a new game configuration. You can edit the name and cost directly.

### 2. Multipliers
For each mode, add multipliers. Each row consists of:
- **Value**: The multiplier value (e.g., 2.5 for 2.5x).
- **Weight**: The relative weight in the distribution.
The tool automatically calculates the **Prob %** for each entry.

### 3. Free Spins
Enable the Free Spins section to simulate bonus rounds. You can configure:
- **Trigger Weight**: Probability of entering the bonus round.
- **Spins Count**: Number of spins awarded.
- **Multiplier Boost**: Extra multiplier applied during free spins.
- **Retrigger**: Toggle if bonus rounds can award more spins.

### 4. Simulations
Set the **Simulations Count** (default 100,000) and click **▶ Run Simulations**. The tool will execute the math engine and update the **Expected RTP** for every mode.

### 5. Import/Export
- **💾 Save**: Save your current workspace to the specified JSON config path.
- **📂 Import**: Load an existing JSON configuration file.
- **🚀 Export for Stake Engine**: Generate all necessary assets in the **Output Directory**.

## Project Structure

```
.
├── Makefile              # Build system
├── includes/             # C++ Headers
│   ├── webview.h         # Native WebViewer wrapper
│   ├── AppBridge.hpp     # JS <-> C++ Bridge logic
│   ├── ModeManager.hpp   # Business logic manager
│   └── Distribution.hpp  # Math engine core
├── srcs/                 # C++ Source files
│   ├── main.cpp          # Entry point
│   ├── AppBridge.cpp     # Bridge implementation
│   ├── ModeManager.cpp   
│   └── Distribution.cpp
├── frontend/             # Web UI
│   ├── index.html        # Main structure
│   ├── app.js            # Vue.js logic & IPC bridge
│   └── style.css         # Modern dark theme
└── output/               # Generated engine files
```

## License

Private project - All rights reserved

---

**Made with ⚡ by ambroiselebs @ JeanClaude**
