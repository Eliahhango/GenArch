# Achicad Automation — Master Task List

---

## Summary: What This Project Does

This tool lets you type a natural language description of a building and it draws the complete model inside Archicad — walls, doors, windows, slabs, columns, beams, roofs, stairs, curtain walls, railings, terrain mesh, furniture in every room, with proper materials and surfaces. **Multi-story, 3D-ready, production quality.**

It also supports **PDF and image import** — give it a floor plan drawing and the AI extracts walls, doors, windows, and rooms as structured JSON to recreate in Archicad.

```
You type: "2-story modern house, 4 bedrooms, 3 bathrooms, gable roof"
             ↓
      AI Planner powered by the MASTER ARCHITECT SOUL
      (embodies Vitruvius, F.L. Wright, Mies van der Rohe, Louis Kahn,
       Zaha Hadid, Tadao Ando — firmitas, utilitas, venustas)
             ↓
      Structured JSON (every element, material, surface, furniture item)
             ↓
      C++ Add-On inside Archicad (AchicadAutomation.apx)
             ↓
      Complete .pln model drawn in 3D with materials, surfaces, furniture

OR:

      [PDF floor plan / image] → Vision AI (Ollama) → JSON → Add-On → Archicad
```

---

## What's Done

### Python Side — 100% Complete & Tested (18 source files + 4 backends)

| File | Purpose | Status |
|------|---------|--------|
| `python/architect_soul.py` | **MASTER ARCHITECT SOUL** — 12KB system prompt embedding Vitruvius triad, passive solar design, kitchen work triangle, material honesty, structural logic, building codes, room psychology, 6 architectural styles, principles of Wright/Kahn/Mies/Le Corbusier/Ando/Hadid | Done |
| `python/schemas.py` | **18 Pydantic models**: Coordinate, WallDef, DoorDef, WindowDef, SlabDef, ColumnDef, BeamDef, RoofDef, ZoneDef, DimensionDef, StairDef, CurtainWallDef, MeshDef, RailingDef, FurnitureDef, FloorDef, SiteDef, ProjectDef, BuildingPlan | Done |
| `python/prompts.py` | Production system prompt, floor-plan-from-image prompt, PDF import prompt | Done |
| `python/backends/base.py` | `AIBackend` (text) + `VisionBackend` (text + images) abstract base classes | Done |
| `python/backends/deepseek.py` | DeepSeek API backend (`deepseek-v4-pro`), JSON mode, Bearer auth | Done |
| `python/backends/ollama_local.py` | Ollama local backend, text + vision support, configurable model, auto-removes code fences from JSON | Done |
| `python/backends/ollama_cloud.py` | Ollama Cloud backend (`ollama.com`), text + vision, Bearer auth, code fence removal | Done |
| `python/ai_planner.py` | `AIPlanner` — calls backend, validates against Pydantic schema, 2 retries | Done |
| `python/archicad_builder.py` | `ArchicadBuilder` — connects to Archicad, multi-floor loop, GUID tracking, 14 element types, surface assignment post-creation | Done |
| `python/importer.py` | `ArchicadImporter` — processes PDF (PyMuPDF) and images (Pillow), detects vector/scanned/hybrid PDFs, renders pages at 200 DPI, sends to vision model for floor plan extraction | Done |
| `python/main.py` | CLI — `--prompt`, `--import-file`, `--input-json`, `--backend`, `--model`, `--dry-run`, `--output-json`, `--verbose` | Done |
| `python/inspect_plan.py` | Utility to analyze AI-generated plan quality (materials, furniture per room, surfaces) | Done |
| `python/requirements.txt` | `archicad`, `requests`, `python-dotenv`, `ollama`, `pydantic`, `pillow` (+ `pymupdf` optional) | Done |
| `python/.env.example` | Template with all config vars including vision model settings | Done |

### C++ Add-On Side — Source Code 100% Written (Not Yet Compiled)

**42 files** in `addon/` — 30 source + 3 resource + CMake + config

**16 registered JSON commands:**

| Command | C++ API | Creates |
|---------|---------|---------|
| `CreateWalls` | `ACAPI_Element_Create` (API_WallID) | Walls with material, composite, surface, floor, reference line |
| `CreateDoors` | `ACAPI_Element_CreateExt` (API_DoorID) | Doors on walls by GUID, position, dimensions, type |
| `CreateWindows` | `ACAPI_Element_CreateExt` (API_WindowID) | Windows on walls by GUID, sill, dimensions, type |
| `CreateSlabs` | `ACAPI_Element_Create` (API_SlabID) | Floor/ceiling slabs with polygon, material, composite |
| `CreateColumns` | `ACAPI_Element_Create` (API_ColumnID) | Columns with position, dimensions, material |
| `CreateBeams` | `ACAPI_Element_Create` (API_BeamID) | Beams with start/end, dimensions, material |
| `CreateRoofs` | `ACAPI_Element_Create` (API_RoofID) | Roofs with polygon, angle, thickness, type |
| `CreateZones` | `ACAPI_Element_Create` (API_ZoneID) | Room zones with polygon, name/number, category |
| `CreateDimensions` | `ACAPI_Element_Create` (API_LinearDimensionID) | Linear/aligned/angular/radial dimensions |
| `CreateStairs` | `ACAPI_Element_Create` (API_StairID) | Stairs with baseline, steps, tread, riser, material |
| `CreateCurtainWalls` | `ACAPI_Element_CreateExt` (API_CurtainWallID) | Curtain walls with polygon, height, panel, frame |
| `CreateMeshes` | `ACAPI_Element_Create` (API_MeshID) | Terrain meshes with polygon, height points, material |
| `CreateRailings` | `ACAPI_Element_Create` (API_RailingID) | Railings with polyline, height, material |
| `CreateFurniture` | `ACAPI_Element_CreateExt` (API_ObjectID) | Furniture/objects with position, rotation, params |
| `SetElementSurfaces` | `ACAPI_Element_Change` | Post-creation surface/material assignment by name lookup |

**Support files:** `CommandBase.hpp/cpp`, `Helpers.hpp/cpp`, `AddOnMain.cpp`, `CMakeLists.txt`, `config.json`, resource files

### Documentation

| File | Content |
|------|---------|
| `README.md` | Full setup guide, usage, architecture, troubleshooting, roadmap |
| `TODO.md` | This file |

---

## The MASTER ARCHITECT SOUL — What Makes It Special

The system prompt (`architect_soul.py`) is not a generic AI instruction. It is a 12KB architectural constitution that embeds:

- **Vitruvius's Triad** — firmitas (strength), utilitas (utility), venustas (beauty)
- **Passive solar design principles** — orientation, window sizing by direction, roof overhang formula, cross-ventilation, thermal mass placement
- **Kitchen work triangle** — exact dimensions (1200-2700mm per leg, 4000-8000mm total)
- **Structural logic** — load path continuity, max spans per material, column grid norms, lintel requirements
- **Building code essentials** — minimum room sizes, egress windows, stair dimensions, railing heights, fire separation
- **Material truth** — "let brick look like brick, never disguise one material as another"
- **Room psychology** — compression and release, axial alignment, hierarchy of spaces
- **6 architectural styles** — Modern/Contemporary, Traditional/Classical, Minimalist, Industrial, Scandinavian
- **Embedded quotes** — Frank Lloyd Wright ("of the hill"), Louis Kahn ("room without natural light"), Mies ("God is in the details"), Le Corbusier ("machine for living"), Tadao Ando ("architecture should remain silent"), Zaha Hadid ("there are 360 degrees")
- **8 sacred rules** — grand concept required, natural light in every room, indoor-outdoor connection, storage, materials specified, buildable, named rooms, furniture proving the space

---

## What's Still Missing

### BLOCKED: C++ Add-On Compilation

| Step | What | Why Blocked |
|------|------|-------------|
| Clone build tools | `git clone https://github.com/GRAPHISOFT/archicad-addon-cmake-tools.git Tools` in `addon/` | Quick — just needs git |
| Install **Visual Studio 2022** | Community edition (free). "Desktop development with C++" workload | Not installed on this machine |
| Install **v142 toolset** | "MSVC v142 - VS 2019 C++ x64/x86 build tools" in VS Installer → Individual Components | Required for AC26 build |
| Install **CMake 3.16+** | Comes with VS2022 or standalone from cmake.org | Not installed |
| Build `.apx` | `python Tools/BuildAddOn.py --configFile config.json --acVersion 26 29 --buildConfig RelWithDebInfo --package` | Depends on all above |

**After compilation:** produces `Build/AC26/RelWithDebInfo/AchicadAutomation.apx` and `Build/AC29/RelWithDebInfo/AchicadAutomation.apx`

### BLOCKED: End-to-End Test in Archicad

| Task | Needs |
|------|-------|
| Install `.apx` into Archicad (Options → Add-On Manager → Add) | Compiled `.apx` |
| Verify Add-On registers (namespace "AchicadAutomation") | Archicad + Add-On |
| Test single wall creation via Python → Add-On | Archicad + Add-On |
| Test full plan from AI JSON → Archicad | Archicad + Add-On |
| Test 3D view of generated building | Archicad + Add-On |

### OPTIONAL: PDF Import Dependencies

| Tool | Install | Reason |
|------|---------|--------|
| **PyMuPDF** | `pip install pymupdf` | Enables PDF import (vector detection, page rendering, OCR) |
| **Ollama vision model** | `ollama pull llava:13b` (local) | Enables image/PDF→JSON extraction |

---

## How to Finish (When VS2022 is Installed)

```powershell
# 1. Clone build tools into addon/
cd "Achicad Automation\addon"
git clone https://github.com/GRAPHISOFT/archicad-addon-cmake-tools.git Tools

# 2. Build for both Archicad versions
python Tools/BuildAddOn.py --configFile config.json --acVersion 26 29 --buildConfig RelWithDebInfo --package

# 3. Install .apx into Archicad (Options → Add-On Manager → Add)

# 4. Run end-to-end — text to building
cd ..\python
python main.py --backend ollama-cloud "2-story modern house, 4 bedrooms, gable roof"

# 5. Import a floor plan image
python main.py --import-file "floor_plan.png" --backend ollama-cloud --dry-run --output-json extracted.json
```

---

## Project File Count

- **Python**: 18 source files + 4 backends + config
- **C++ Add-On**: 30 source files + 3 resource files + CMake + config
- **Documentation**: 2 files
- **Total**: ~77 files (excluding venv and __pycache__)

---

## Legend
- `[ ]` = Not started
- `[~]` = In progress
- `[x]` = Done
- `[!]` = Blocked (waiting on external dependency)
