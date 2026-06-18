# Achicad Automation — User Guide

AI-powered Archicad Add-On that generates full 3D building models from natural language descriptions.

## Architecture

```
┌─────────────────────┐     JSON over TCP      ┌──────────────────────┐
│  Python CLI (CLI)   │ ──────────────────────> │  C++ Add-On (.apx)   │
│  main.py            │    Add-On Command API   │  Archicad 29         │
│                     │                         │                      │
│  AI Backend ────────┤                         │  16 Commands:        │
│  • DeepSeek API     │                         │  CreateWalls         │
│  • Ollama Local     │                         │  CreateDoors         │
│  • Ollama Cloud     │                         │  CreateWindows       │
│                     │                         │  CreateSlabs         │
│  Output: JSON       │                         │  ...and 12 more      │
│  building plan      │                         └──────────────────────┘
└─────────────────────┘
```

## Installation

### 1. Install the Add-On

Copy `AchicadAutomation.apx` into Archicad's Add-Ons folder:

```
%APPDATA%\GRAPHISOFT\ARCHICAD 29\Add-Ons\
```

Restart Archicad 29. The Add-On loads automatically.

### 2. Python Setup

Install dependencies:

```
cd python/
pip install requests ollama pydantic pyyaml
```

**Optional** — for PDF/image import:
```
pip install PyMuPDF
```

## Usage

### Quick Start

```bash
python main.py --backend ollama-cloud "2-story modern house with 4 bedrooms and a pool"
```

### Building Types

| Example | Description |
|---------|-------------|
| `"a small 1-bedroom cottage with a pitched roof"` | Simple residential |
| `"3-story office building with open floor plan, curtain walls, and a lobby"` | Commercial |
| `"modern villa with 5 bedrooms, swimming pool, and a 2-car garage"` | Luxury residential |
| `"school building with 8 classrooms, a gymnasium, and an auditorium"` | Educational |
| `"hospital wing with 12 patient rooms, nurses station, and an operating theatre"` | Healthcare |

### CLI Options

| Flag | Description |
|------|-------------|
| `--backend` | AI backend: `deepseek`, `ollama`, `ollama-cloud` (default: `deepseek`) |
| `--dry-run` | Print generated JSON to console, don't send to Archicad |
| `--output-json` | Save generated building plan to a JSON file |
| `--input-json` | Load a previously saved JSON file and send to Archicad |
| `--import-file` | Import a PDF or image file for AI analysis |
| `--verbose` | Show detailed debug output |

### Examples

#### Generate and preview without Archicad:
```bash
python main.py --backend deepseek --dry-run "modern 3-bedroom house"
```

#### Save the plan for later use:
```bash
python main.py --backend ollama-cloud --output-json my_design.json "small cabin in the woods"
```

#### Rebuild from a saved plan:
```bash
python main.py --input-json my_design.json
```

#### Import a PDF floor plan sketch:
```bash
python main.py --import-file sketch.pdf --backend deepseek
```

### AI Backends

| Backend | Flag | Requirements |
|---------|------|-------------|
| DeepSeek API | `--backend deepseek` | `DEEPSEEK_API_KEY` env var |
| Ollama Cloud | `--backend ollama-cloud` | Ollama Cloud account/token |
| Ollama Local | `--backend ollama` | Local Ollama running with a model |

Set environment variables:
```bash
set DEEPSEEK_API_KEY=sk-your-key-here        # Windows PowerShell
set OLLAMA_CLOUD_TOKEN=your-token-here
```

## What Can Be Generated

The Add-On creates 16 element types in Archicad:

| Category | Elements |
|----------|----------|
| Structure | Walls, Slabs, Columns, Beams, Roofs |
| Openings | Doors (linked to walls), Windows (linked to walls) |
| Enclosure | Curtain Walls, Meshes, Railings |
| Interior | Zones (rooms with area/name), Stairs |
| Annotation | Dimensions |
| Objects | Furniture, Library Part Objects |

### What each element requires

- **Walls**: position (x1,y1 → x2,y2), height, thickness, building material
- **Doors/Windows**: parent wall GUID, position along wall, width, height
- **Slabs**: polygon points, thickness, building material/composite
- **Columns**: position, width, depth, height, building material
- **Beams**: start/end position, width, height, building material
- **Roofs**: polygon, thickness, pitch angle, building material
- **Zones**: polygon, name, category, height
- **Dimensions**: pairs of points, type (horizontal/vertical/aligned)
- **Stairs**: position, width, number of treads, riser height
- **Curtain Walls**: position, height, width, grid divisions
- **Meshes**: polygon grid, elevation points
- **Railings**: path segments, height, railing type
- **Furniture/Obejcts**: position, library part name, scale

## Generated JSON Structure

A building plan contains floors, each with arrays of elements:

```json
{
  "project": {
    "name": "Modern House",
    "description": "2-story modern house"
  },
  "floors": [
    {
      "name": "Ground Floor",
      "elevation": 0.0,
      "height": 3.0,
      "walls": [ ... ],
      "slabs": [ ... ],
      "columns": [ ... ],
      "doors": [ ... ],
      "windows": [ ... ],
      "zones": [ ... ]
    },
    {
      "name": "Second Floor",
      "elevation": 3.0,
      "height": 3.0,
      "walls": [ ... ],
      "zones": [ ... ]
    }
  ]
}
```

Each element contains all geometry, material, and surface properties needed for Archicad to create it fully configured.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| "Failed to connect" | Archicad not running or Add-On not loaded | Verify Archicad 29 is running and the .apx is in the Add-Ons folder |
| "API key not set" | Missing environment variable | Set `DEEPSEEK_API_KEY` or `OLLAMA_CLOUD_TOKEN` |
| "Invalid JSON" | AI returned malformed output | Retry with `--verbose` to see raw AI output; try `--backend ollama-cloud` which is more reliable |
| Elements not showing | Wrong floor/story index | Ensure Archicad has stories matching the plan; the Add-On auto-maps z-coordinates to stories |
| "PyMuPDF not found" | Missing PDF library | `pip install PyMuPDF` |
| Add-On not loading in Archicad | Wrong Archicad version | This Add-On is compiled for Archicad 29 only |

## Files

```
├── addon/                    # C++ Add-On source (.apx)
│   ├── Src/                  # Source code (16 command pairs + helpers)
│   ├── Build/                # Build outputs
│   └── build_ac29.bat        # Build script for AC29
├── python/                   # Python CLI
│   ├── main.py               # Entry point
│   ├── architect_soul.py     # Master architect AI prompt
│   ├── schemas.py            # Pydantic models
│   ├── ai_planner.py         # AI interaction layer
│   └── archicad_builder.py   # Sends commands to Archicad
└── README.md
```
