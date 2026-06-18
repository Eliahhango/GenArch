from __future__ import annotations

import argparse
import json
import logging
import os
import sys
from pathlib import Path

from dotenv import load_dotenv

from ai_planner import AIPlanner
from archicad_builder import ArchicadBuilder, BuildResult
from backends import DeepSeekBackend, OllamaCloudBackend, OllamaLocalBackend
from backends.base import VisionBackend
from importer import ArchicadImporter
from prompts import PRODUCTION_BUILDING_SYSTEM_PROMPT, FLOOR_PLAN_FROM_IMAGE_PROMPT
from schemas import BuildingPlan

logger = logging.getLogger(__name__)


def setup_logging(verbose: bool) -> None:
    level = logging.DEBUG if verbose else logging.INFO
    logging.basicConfig(
        level=level,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        datefmt="%H:%M:%S",
    )


def load_env() -> None:
    env_path = Path(__file__).parent / ".env"
    if env_path.exists():
        load_dotenv(env_path)
    else:
        load_dotenv()


def create_backend(backend_name: str, model: str | None) -> object:
    backends = {
        "deepseek": DeepSeekBackend,
        "ollama-local": OllamaLocalBackend,
        "ollama-cloud": OllamaCloudBackend,
    }
    cls = backends.get(backend_name)
    if cls is None:
        valid = ", ".join(backends.keys())
        raise ValueError(f"Unknown backend '{backend_name}'. Valid options: {valid}")
    return cls(model=model) if model else cls()


def print_plan_summary(plan: BuildingPlan) -> None:
    proj = plan.project
    print(f"\n=== {proj.name} ===")
    print(f"  Style: {proj.style}")
    print(f"  Floors: {len(proj.floors)}")

    total_walls = 0
    total_inner = 0
    total_doors = 0
    total_windows = 0
    total_slabs = 0
    total_cols = 0
    total_beams = 0
    total_zones = 0
    total_stairs = 0
    total_cw = 0
    total_rail = 0
    total_dim = 0
    total_furn = 0

    for i, floor in enumerate(proj.floors):
        outer = len(floor.outer_walls)
        inner = len(floor.inner_walls)
        doors = len(floor.doors)
        wins = len(floor.windows)
        slabs = len(floor.slabs)
        cols = len(floor.columns)
        beams = len(floor.beams)
        zones = len(floor.zones)
        stairs = len(floor.stairs)
        cw = len(floor.curtain_walls)
        rail = len(floor.railings)
        dims = len(floor.dimensions)
        furn = len(floor.furniture)

        total_walls += outer + inner
        total_inner += inner
        total_doors += doors
        total_windows += wins
        total_slabs += slabs
        total_cols += cols
        total_beams += beams
        total_zones += zones
        total_stairs += stairs
        total_cw += cw
        total_rail += rail
        total_dim += dims
        total_furn += furn

        print(f"\n  Floor {i}: {floor.name} ({floor.height_mm}mm)")
        print(f"    Walls: {outer} outer + {inner} inner | Doors: {doors} | Windows: {wins}")
        print(f"    Slabs: {slabs} | Columns: {cols} | Beams: {beams}")
        print(f"    Zones: {zones} | Stairs: {stairs} | CurtainWalls: {cw} | Railings: {rail}")
        print(f"    Furniture: {furn} | Dimensions: {dims}")

    print(f"\n  Roofs: {len(proj.roofs)}")
    print(f"  Outdoor furniture: {len(proj.outdoor_furniture)}")

    print(f"\n  TOTALS: {total_walls} walls | {total_doors} doors | {total_windows} windows | {total_slabs} slabs")
    print(f"          {total_cols} columns | {total_beams} beams | {total_zones} zones | {total_stairs} stairs")
    print(f"          {total_furn} furniture | {total_dim} dimensions")


def print_build_result(result: BuildResult) -> None:
    print(f"\n=== Build Result ===")
    print(f"  Walls:       {result.walls}")
    print(f"  Doors:       {result.doors}")
    print(f"  Windows:     {result.windows}")
    print(f"  Slabs:       {result.slabs}")
    print(f"  Columns:     {result.columns}")
    print(f"  Beams:       {result.beams}")
    print(f"  Roofs:       {result.roofs}")
    print(f"  Zones:       {result.zones}")
    print(f"  Stairs:      {result.stairs}")
    print(f"  CurtainWalls:{result.curtain_walls}")
    print(f"  Meshes:      {result.meshes}")
    print(f"  Railings:    {result.railings}")
    print(f"  Furniture:   {result.furniture}")
    print(f"  Dimensions:  {result.dimensions}")
    print(f"  Surfaces:    {result.surfaces_assigned} assignments")
    print(f"  -----------------------------------")
    print(f"  Total:       {result.total_elements} elements")
    if result.errors:
        print(f"\n  Errors ({len(result.errors)}):")
        for err in result.errors:
            print(f"    - {err}")
    else:
        print(f"  Status: SUCCESS")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="AI-driven Archicad building generator — production-ready models",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python main.py "2-story modern house, 4 bedrooms, 3 bathrooms, open kitchen, gable roof"
  python main.py --backend ollama-cloud "3-bedroom bungalow with garage and patio"
  python main.py --dry-run --output-json plan.json "luxury villa with pool"
  python main.py --input-json plan.json
        """,
    )
    parser.add_argument(
        "prompt",
        nargs="?",
        help="Natural language description of the building to generate",
    )
    parser.add_argument(
        "--backend",
        choices=["deepseek", "ollama-local", "ollama-cloud"],
        default=None,
        help="AI backend (default: from DEFAULT_BACKEND env var or 'deepseek')",
    )
    parser.add_argument(
        "--model",
        default=None,
        help="Override the default model for the selected backend",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Generate the plan JSON only, do not connect to Archicad",
    )
    parser.add_argument(
        "--output-json", metavar="FILE",
        help="Save the generated plan JSON to a file",
    )
    parser.add_argument(
        "--output-pln", metavar="FILE",
        help="Save the Archicad project to a .pln file after building",
    )
    parser.add_argument(
        "--input-json", metavar="FILE",
        help="Load a previously saved plan JSON instead of calling the AI",
    )
    parser.add_argument(
        "--import-file", metavar="FILE",
        help="Import a PDF or image file containing a floor plan (uses vision model)",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable debug logging",
    )

    args = parser.parse_args()
    setup_logging(args.verbose)
    load_env()

    if args.import_file:
        if not (args.input_json or args.prompt):
            pass
        elif args.input_json or args.prompt:
            parser.error("--import-file cannot be combined with a prompt or --input-json")

    if args.input_json and args.prompt:
        parser.error("Cannot specify both --input-json and a prompt")
    if not args.input_json and not args.prompt and not args.import_file:
        parser.error("Must specify a prompt, --input-json, or --import-file")

    backend_name = args.backend or os.environ.get("DEFAULT_BACKEND", "deepseek")

    plan: BuildingPlan

    if args.import_file:
        backend = create_backend(backend_name, args.model)
        if not isinstance(backend, VisionBackend):
            print(f"ERROR: Backend '{backend.name}' does not support vision/image import.")
            print("Use --backend ollama-local or --backend ollama-cloud for image/PDF import.")
            sys.exit(1)

        logger.info("Importing %s via %s", args.import_file, backend.name)
        importer = ArchicadImporter(backend)
        result = importer.import_file(args.import_file, import_prompt=FLOOR_PLAN_FROM_IMAGE_PROMPT)

        if not result.success:
            print(f"Import failed: {result.errors}")
            sys.exit(1)

        plan = result.plans[0]
        print_plan_summary(plan)
    elif args.input_json:
        logger.info("Loading plan from %s", args.input_json)
        with open(args.input_json, encoding="utf-8") as f:
            plan = BuildingPlan.model_validate(json.load(f))
        print_plan_summary(plan)
    else:
        backend = create_backend(backend_name, args.model)
        logger.info("Using backend: %s", backend.name)
        planner = AIPlanner(backend, system_prompt=PRODUCTION_BUILDING_SYSTEM_PROMPT)

        print(f"\nGenerating plan via {backend.name}...")
        plan = planner.plan(args.prompt)
        print_plan_summary(plan)

    if args.output_json:
        output_path = Path(args.output_json)
        output_path.write_text(plan.model_dump_json(indent=2), encoding="utf-8")
        logger.info("Plan saved to %s", output_path)

    output_pln = None
    if args.output_pln:
        output_pln = Path(args.output_pln)

    if args.dry_run:
        print("\nDry run complete. No elements created in Archicad.")
        return

    print("\nConnecting to Archicad...")
    builder = ArchicadBuilder()
    try:
        builder.connect()
    except ConnectionError as e:
        print(f"ERROR: {e}")
        print("Make sure Archicad is running with the AchicadAutomation Add-On installed.")
        sys.exit(1)

    print("Building...")
    result = builder.build(plan)
    print_build_result(result)

    if output_pln:
        try:
            import json
            from urllib.request import Request, urlopen
            save_req = Request(f"http://127.0.0.1:{builder._conn.port}")
            save_req.add_header("Content-Type", "application/json")
            save_payload = json.dumps({
                "command": "API.SaveProjectAs",
                "parameters": {"path": str(output_pln)}
            })
            save_resp = urlopen(save_req, save_payload.encode("UTF-8"))
            save_result = json.loads(save_resp.read())
            if save_result.get("succeeded"):
                print(f"\nProject saved to: {output_pln}")
            else:
                print(f"\nWarning: Could not save project via API. Save manually (Ctrl+S).")
        except Exception as e:
            print(f"\nWarning: Save failed ({e}). Save manually from Archicad (Ctrl+S).")

    builder.disconnect()


if __name__ == "__main__":
    main()
