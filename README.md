# GenArch

**Gen**erative **Arch**itecture — AI-powered building generator for Graphisoft Archicad.

Describe a building in plain English, and GenArch draws the complete 3D model — walls, doors, windows, slabs, columns, beams, roofs, rooms, stairs, furniture, and more — directly into a running Archicad instance.

```
"2-story modern house with 4 bedrooms, open-plan living, and a flat roof"
                              ↓
                    AI Planner (Ollama Cloud / DeepSeek / Ollama)
                              ↓
                    Structured JSON building plan
                              ↓
                    C++ Add-On inside Archicad
                              ↓
                    Complete .pln model drawn automatically
```

## Quick Start

```powershell
cd python
pip install -r requirements.txt
python main.py --backend ollama-cloud "1-bedroom cabin with gable roof"
```

Requires: Python 3.10+, an API key, and Archicad 29 with the Add-On installed.

## Features

- **14 building element types** — Walls, Doors (wall-linked), Windows (wall-linked), Slabs, Columns, Beams, Roofs, Zones, Stairs, Curtain Walls, Meshes, Railings, Furniture, Objects
- **Multi-floor** — Any number of stories with per-floor element arrays
- **3 AI backends** — Ollama Cloud (recommended), DeepSeek API, Ollama Local
- **PDF/image import** — Sketch a floor plan and let AI turn it into a 3D model
- **Architecturally aware** — The AI follows real architectural conventions (wall thickness, room sizes, structural logic, passive solar principles)
- **Multi-style** — Modern, Victorian, Japanese, Brutalist, Mediterranean, Prairie

## Setup

### 1. Python

```powershell
cd python
pip install -r requirements.txt          # or pip install requests ollama pydantic
copy .env.example .env                   # then edit .env with your API keys
```

### 2. Add-On

Copy the compiled `.apx` into Archicad's Add-Ons folder:

```
%APPDATA%\GRAPHISOFT\ARCHICAD 29\Add-Ons\
```

Restart Archicad 29. To build from source, see [Building the Add-On](#building-the-add-on).

### 3. Generate a Building

```powershell
python main.py "3-bedroom house with pool and 2-car garage"
```

## CLI Reference

| Command | Description |
|---------|-------------|
| `python main.py "description"` | Generate and draw in Archicad |
| `--dry-run` | AI plan only, no Archicad |
| `--output-json file.json` | Save the generated plan to file |
| `--input-json file.json` | Replay a saved plan (no AI call) |
| `--backend <name>` | `ollama-cloud`, `deepseek`, or `ollama` |
| `--model <name>` | Override the AI model |
| `--import-file <path>` | Import a PDF/image for AI analysis |
| `--verbose` | Detailed logging |

### Examples

```powershell
# Preview without Archicad
python main.py --dry-run "modern 2-story office with curtain walls"

# Save and reuse a plan
python main.py --output-json plan.json "L-shaped villa with courtyard"
python main.py --input-json plan.json

# Import a floor plan sketch
python main.py --import-file sketch.pdf

# Choose backend and model
python main.py --backend deepseek "small cottage"
```

## Building the Add-On

Requires Visual Studio 2022, CMake 3.19+, and the AC29 DevKit.

```powershell
cd addon
.\build_ac29.bat
```

Output: `addon/Build/AC29_NMake/AchicadAutomation.apx`

## Architecture

```
python/main.py           → AI generates JSON building plan
python/archicad_builder.py → Sends JSON to Archicad via Add-On Command API
addon/Src/AddOnMain.cpp  → Registers 16 commands
addon/Src/Create*.cpp    → Each command creates an element type
```

Elements are created in dependency order: Walls first (GUIDs captured for door/window linking), then Slabs, Columns, Beams, Doors, Windows, Roofs, Zones, Dimensions, Stairs, Curtain Walls, Meshes, Railings, Furniture, Objects.

## Project Structure

```
├── addon/               # C++ Add-On source (.apx)
│   ├── Src/             # 16 command pairs + helpers + entry point
│   ├── build_ac29.bat   # One-command build
│   └── CMakeLists.txt
├── python/              # Python CLI
│   ├── main.py          # Entry point
│   ├── backends/        # AI backends (DeepSeek, Ollama, Ollama Cloud)
│   ├── schemas.py       # Pydantic models
│   └── architect_soul.py # Master architect AI prompt
└── USER_GUIDE.md        # Full documentation
```
