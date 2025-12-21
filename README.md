# Math SDK for JeanClaude StakeEngine

**Author:** ambroiselebs
**Studio:** JeanClaude StakeEngine

## Description

Math SDK is a mathematical file generation tool for the JeanClaude StakeEngine. It allows you to create and simulate game modes with different multipliers and export the results for analysis.

## Why C++ instead of Python?

The main advantage of this C++ implementation is **exceptional performance**:
- **100,000 simulations** executed in milliseconds
- Typical execution time: ~10-50ms depending on configuration
- Performance 100x to 1000x superior to equivalent Python implementation

This speed allows rapid iteration on configurations and testing multiple scenarios without waiting time.

## Current Features

Currently, the tool supports:
- ✅ Create game modes with different costs
- ✅ Define multipliers with their respective weights
- ✅ Generate random simulations (weighted distribution)
- ✅ Automatically calculate RTP (Return to Player)
- ✅ Export results in CSV and compressed JSONL (zstd)
- ✅ Command Line Interface (CLI)
- ✅ Graphical User Interface (GUI) with ImGui

## Prerequisites

- C++ compiler supporting C++17 (g++, clang++)
- Make
- System libraries:
  - `libzstd` (compression)
  - `libglfw3` (for GUI version)
  - `OpenGL` (for GUI version)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential libzstd-dev libglfw3-dev libgl1-mesa-dev
```

**Arch Linux:**
```bash
sudo pacman -S base-devel zstd glfw-x11
```

## Compilation

### CLI Version (command line)
```bash
make
```

### GUI Version (graphical interface)
```bash
make gui
```

### Clean compiled files
```bash
make clean      # Remove object files
make fclean     # Remove everything (binaries + output)
make re         # Recompile everything from scratch
```

## Usage

### CLI Version

#### Run with default configuration
```bash
make run
# or directly:
./math-engine
```

#### Modify configuration

Edit the `main.cpp` file to customize:

```cpp
// Number of simulations
numSimulations = 100000;

// Create a game mode
dist.addMode("base", 1.0);  // name, cost

// Add multipliers (mode, multiplier, weight)
dist.addMultiplier("base", 0.0, 350);   // 0x  - weight 350
dist.addMultiplier("base", 0.5, 250);   // 0.5x - weight 250
dist.addMultiplier("base", 1.0, 200);   // 1x  - weight 200
dist.addMultiplier("base", 1.5, 120);   // 1.5x - weight 120
dist.addMultiplier("base", 2.0, 80);    // 2x  - weight 80

// Run simulations
dist.runSimulations("base", numSimulations, 42);  // mode, count, seed

// Export results
dist.exportAll("output");
```

### GUI Version

```bash
make run-gui
# or directly:
./math-engine-gui
```

The graphical interface allows you to:
- Create and edit modes visually
- Add/remove multipliers
- Run simulations in real-time
- Visualize statistics (RTP, simulation count)

## Generated Files

Results are exported to the `output/` folder:

### `index.json`
Index file containing the list of all modes and their metadata:
```json
{
  "modes": [
    {
      "name": "base",
      "cost": 1.0,
      "simulations": 100000,
      "rtp": 0.95,
      "files": {
        "csv": "output/base.csv",
        "jsonl": "output/base.jsonl.zst"
      }
    }
  ]
}
```

### `<mode>.csv`
Uncompressed CSV file for quick analysis:
```csv
id,weight,payoutMultiplier
0,350,0
1,350,0
2,250,500
...
```

### `<mode>.jsonl.zst`
JSONL file compressed with zstd (optimized for storage):
```jsonl
{"id":0,"weight":350,"payoutMultiplier":0}
{"id":1,"weight":350,"payoutMultiplier":0}
{"id":2,"weight":250,"payoutMultiplier":500}
...
```

## Project Structure

```
.
├── main.cpp              # CLI entry point
├── gui_main.cpp          # GUI entry point
├── Makefile              # Build file
├── includes/             # Headers (.hpp)
│   ├── Distribution.hpp  # Main class
│   ├── ModeManager.hpp   # Mode manager
│   ├── GuiWindow.hpp     # GUI window
│   └── ModeEditor.hpp    # Mode editor
├── srcs/                 # Implementations (.cpp)
│   ├── Distribution.cpp
│   ├── ModeManager.cpp
│   ├── GuiWindow.cpp
│   └── ModeEditor.cpp
├── libs/                 # External libraries
│   └── imgui/            # Dear ImGui (graphical interface)
└── output/               # Generated results (created automatically)
```

## Roadmap

Upcoming features:
- [ ] Import/export JSON configurations
- [ ] Advanced statistical analysis (variance, standard deviation)
- [ ] Automatic RTP validation
- [ ] Batch mode to test multiple configurations
- [ ] Tools for creating slot games
- [ ] Tools for creating board games

## License

Private project - All rights reserved

---

**Made with ⚡ by ambroiselebs @ JeanClaude**
