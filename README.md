# Achicad Automation

AI-driven building generator for Graphisoft Archicad. Describe a building in natural language, and it draws the complete model — walls, doors, windows, slabs, columns, beams, roofs, zones, and dimensions — directly into a running Archicad instance.

```
You type: "12x9m house with 3 bedrooms, 2 bathrooms, gable roof"
                    ↓
         AI Planner (Ollama Cloud / DeepSeek / Ollama Local)
                    ↓
         Structured JSON building plan
                    ↓
         C++ Add-On inside Archicad
                    ↓
         Complete .pln model drawn automatically
```

---

## Project Structure

```
Achicad Automation/
├── addon/                          # C++ Add-On for Archicad
│   ├── CMakeLists.txt              # Build config (AC26 + AC29)
│   ├── config.json                 # Add-On metadata
│   ├── Src/
│   │   ├── AddOnMain.cpp           # Entry point, registers all commands
│   │   ├── CommandBase.hpp/cpp     # Base class for JSON commands
│   │   ├── Helpers.hpp/cpp         # Coordinate/GUID/floor utilities
│   │   ├── CreateWallsCommand.hpp/cpp
│   │   ├── CreateDoorsCommand.hpp/cpp
│   │   ├── CreateWindowsCommand.hpp/cpp
│   │   ├── CreateSlabsCommand.hpp/cpp
│   │   ├── CreateColumnsCommand.hpp/cpp
│   │   ├── CreateBeamsCommand.hpp/cpp
│   │   ├── CreateRoofsCommand.hpp/cpp
│   │   ├── CreateZonesCommand.hpp/cpp
│   │   ├── CreateDimensionsCommand.hpp/cpp
│   │   └── CreateObjectsCommand.hpp/cpp
│   ├── RFIX/                       # Language-neutral resources
│   ├── RFIX.win/                   # Windows-specific resources
│   └── RINT/                       # English localized strings
├── python/
│   ├── main.py                     # CLI entry point
│   ├── ai_planner.py               # AI orchestration with retry logic
│   ├── archicad_builder.py         # Connects to Archicad, calls Add-On
│   ├── schemas.py                  # Pydantic models for all element types
│   ├── prompts.py                  # System prompts for AI
│   ├── backends/
│   │   ├── base.py                 # AIBackend abstract class
│   │   ├── deepseek.py             # DeepSeek API backend
│   │   ├── ollama_local.py         # Ollama local backend
│   │   └── ollama_cloud.py         # Ollama Cloud backend
│   ├── requirements.txt
│   └── .env.example
└── TODO.md                         # Master task list
```

---

## Prerequisites

### For the Python AI Pipeline (ready now)
- **Python 3.10+**
- An API key for at least one AI backend:
  - [Ollama Cloud](https://ollama.com/settings/keys) (recommended)
  - [DeepSeek](https://platform.deepseek.com/api_keys)
  - Or [Ollama](https://ollama.com/download) running locally

### For the C++ Add-On (to actually draw in Archicad)
- **Visual Studio 2022** with:
  - "Desktop development with C++" workload
  - "MSVC v142 - VS 2019 C++ x64/x86 build tools" (Individual components)
- **CMake 3.16+** (included with VS2022 or [separate install](https://cmake.org/download/))
- **Git** with submodule support
- **Archicad 26 and/or 29** (demo mode works for testing)
- **Archicad API Development Kit** (auto-downloaded by build script)

---

## Setup

### Step 1: Python Environment

```powershell
cd "Achicad Automation\python"
python -m venv venv
.\venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

### Step 2: Configure API Keys

Copy `.env.example` to `.env` and add your keys:

```powershell
copy .env.example .env
```

Edit `.env`:
```
OLLAMA_API_KEY=your-ollama-cloud-key-here
DEEPSEEK_API_KEY=sk-your-deepseek-key-here     # optional
DEFAULT_BACKEND=ollama-cloud                    # or deepseek, ollama-local
```

### Step 3: Test the AI Planner (no Archicad needed)

```powershell
python main.py --dry-run --output-json plan.json "10x8m house with 2 bedrooms, 1 bathroom, gable roof"
```

This calls the AI, generates a plan, and saves it to `plan.json`. No Archicad required.

### Step 4: Build the C++ Add-On

```powershell
cd ..\addon

# Clone the build tools (required)
git clone https://github.com/GRAPHISOFT/archicad-addon-cmake-tools.git Tools

# Build for both AC26 and AC29
python Tools/BuildAddOn.py --configFile config.json --acVersion 26 29 --buildConfig RelWithDebInfo --package
```

This produces:
- `Build/AC26/RelWithDebInfo/AchicadAutomation.apx`
- `Build/AC29/RelWithDebInfo/AchicadAutomation.apx`

### Step 5: Install the Add-On in Archicad

1. Open Archicad
2. **Options → Add-On Manager**
3. Click **Add**, browse to the `.apx` file
4. The Add-On appears as "AchicadAutomation"

### Step 6: Generate a Building

```powershell
cd ..\python
python main.py "12x9m house with 3 bedrooms, 2 bathrooms, open kitchen, gable roof"
```

---

## Usage

### CLI Commands

```powershell
# Basic: generate and draw in Archicad
python main.py "10x8m rectangular house with 2 bedrooms"

# Dry run: AI plan only, no Archicad
python main.py --dry-run "L-shaped house with patio"

# Save plan to file for inspection
python main.py --dry-run --output-json my_plan.json "3-bedroom house"

# Replay a saved plan (no AI call)
python main.py --input-json my_plan.json

# Choose backend
python main.py --backend deepseek "building description"
python main.py --backend ollama-local "building description"
python main.py --backend ollama-cloud "building description"

# Override model
python main.py --backend ollama-cloud --model "qwen3-coder:480b-cloud" "building description"

# Floor plan only (no roof/columns/beams)
python main.py --floor-plan-only "simple rectangular room"

# Verbose logging
python main.py -v "building description"
```

### Supported AI Backends

| Backend | Flag | Default Model | Requires |
|---------|------|---------------|----------|
| Ollama Cloud | `--backend ollama-cloud` | `deepseek-v3.1:671b-cloud` | `OLLAMA_API_KEY` |
| DeepSeek | `--backend deepseek` | `deepseek-v4-pro` | `DEEPSEEK_API_KEY` |
| Ollama Local | `--backend ollama-local` | `qwen3-coder` | Ollama running locally |

### What the AI Generates

The system prompt teaches the AI architectural conventions:
- Coordinate system: millimeters, origin at (0,0)
- Exterior walls: 300mm thick, 3000mm high
- Interior walls: 120mm thick
- Doors: 900x2100mm
- Windows: 1200x1500mm, sill at 900mm
- Slabs: 150mm thick
- Columns: 300x300mm at corners
- Roof: 30° gable by default
- Room minimums: living 4000x5000, bedroom 3000x4000, bathroom 2000x2500

---

## Architecture

### C++ Add-On Commands

All commands are registered under the `AchicadAutomation` namespace and called via `ExecuteAddOnCommand`:

| Command | Input | Output |
|---------|-------|--------|
| `CreateWalls` | `wallsData[]` with beg/end coordinates, height, thickness | `wallGuids[]` |
| `CreateDoors` | `doorsData[]` with ownerWallId GUID, centerOffset, dimensions | `doorGuids[]` |
| `CreateWindows` | `windowsData[]` with ownerWallId GUID, centerOffset, dimensions | `windowGuids[]` |
| `CreateSlabs` | `slabsData[]` with polygon, thickness | `slabGuids[]` |
| `CreateColumns` | `columnsData[]` with position, dimensions | `columnGuids[]` |
| `CreateBeams` | `beamsData[]` with beg/end coordinates, dimensions | `beamGuids[]` |
| `CreateRoofs` | `roofsData[]` with polygon, angle, thickness | `roofGuids[]` |
| `CreateZones` | `zonesData[]` with polygon, roomName, roomNumber | `zoneGuids[]` |
| `CreateDimensions` | `dimensionsData[]` with points, offset | `dimensionGuids[]` |
| `CreateObjects` | `objectsData[]` with libraryPartName, position | `objectGuids[]` |

### Build Order

The builder creates elements in this order to handle dependencies:
1. **Walls** first (GUIDs captured for door/window placement)
2. Slabs
3. Columns
4. Beams
5. Doors (referencing wall GUIDs)
6. Windows (referencing wall GUIDs)
7. Roofs
8. Zones
9. Dimensions
10. Objects

---

## Troubleshooting

### "Failed to connect to Archicad"
- Make sure Archicad is running
- Verify the AchicadAutomation Add-On is installed (Options → Add-On Manager)
- Try launching Archicad in demo mode: `Archicad.exe -DEMO`

### "DEEPSEEK_API_KEY not set" / "OLLAMA_API_KEY not set"
- Check `.env` file exists in the `python/` directory
- Verify the key is correct and has no extra spaces

### AI generates invalid JSON
- The planner has built-in retry logic (2 retries)
- Try a different model: `--model qwen3-coder:480b-cloud`
- Use `--verbose` to see what the AI returned

### CMake build fails
- Verify `AC_API_DEVKIT_DIR` points to the `Support/` folder inside the DevKit
- Make sure v142 toolset is installed in VS2022 for AC26 builds
- Python 3.10+ is required for the resource compilation scripts

---

## Roadmap

- [ ] Site plan generation (property boundaries, setbacks, terrain)
- [ ] Elevation generation (roof profiles, floor heights)
- [ ] Material/texture assignment
- [ ] Add-On UI dialog inside Archicad (type prompt → hit Generate)
- [ ] Additional AI backends (Claude, GPT, Gemini)
- [ ] IFC export from generated model
- [ ] Multi-floor buildings
- [ ] Curved/angled walls
- [ ] Stair generation
- [ ] Automatic room tagging and area calculation
#   G e n A r c h  
 