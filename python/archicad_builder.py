from __future__ import annotations

import logging
from dataclasses import dataclass, field
from typing import Any

from archicad import ACConnection

from schemas import BuildingPlan, FloorDef

logger = logging.getLogger(__name__)

ADDON_NAMESPACE = "AchicadAutomation"


@dataclass
class BuildResult:
    walls: int = 0
    doors: int = 0
    windows: int = 0
    slabs: int = 0
    columns: int = 0
    beams: int = 0
    roofs: int = 0
    zones: int = 0
    dimensions: int = 0
    stairs: int = 0
    curtain_walls: int = 0
    meshes: int = 0
    railings: int = 0
    furniture: int = 0
    surfaces_assigned: int = 0
    errors: list[str] = field(default_factory=list)

    @property
    def total_elements(self) -> int:
        return (
            self.walls + self.doors + self.windows + self.slabs
            + self.columns + self.beams + self.roofs + self.zones
            + self.dimensions + self.stairs + self.curtain_walls
            + self.meshes + self.railings + self.furniture
        )

    @property
    def success(self) -> bool:
        return len(self.errors) == 0


class ArchicadBuilder:
    def __init__(self) -> None:
        self._conn: ACConnection | None = None
        self._commands: Any = None
        self._types: Any = None

    def connect(self) -> None:
        try:
            self._conn = ACConnection.connect()
            self._commands = self._conn.commands
            self._types = self._conn.types
            logger.info("Connected to Archicad (port %s, version %s)", self._conn.port, self._conn.version)
        except Exception as e:
            raise ConnectionError(f"Failed to connect to Archicad: {e}") from e

    def disconnect(self) -> None:
        self._conn = None
        self._commands = None
        self._types = None

    @property
    def is_connected(self) -> bool:
        if self._conn is None:
            return False
        try:
            return self._commands.IsAlive()
        except Exception:
            return False

    def _execute_addon_command(self, command_name: str, params: dict[str, Any]) -> Any:
        cmd_id = self._types.AddOnCommandId(ADDON_NAMESPACE, command_name)
        return self._commands.ExecuteAddOnCommand(cmd_id, params)

    def _coord(self, coord) -> dict[str, float]:
        return {"x": coord.x, "y": coord.y}

    def build(self, plan: BuildingPlan) -> BuildResult:
        result = BuildResult()

        if not self.is_connected:
            result.errors.append("Not connected to Archicad")
            return result

        try:
            proj = plan.project

            for floor in proj.floors:
                self._build_floor(floor, result)

            self._create_roofs(proj, result)
            self._create_site_mesh(proj, result)
            self._create_outdoor_furniture(proj, result)
            self._assign_materials(plan, result)

        except Exception as e:
            logger.exception("Build failed")
            result.errors.append(str(e))

        logger.info(
            "Build complete: %d elements (%dW %dD %dWin %dS %dC %dB %dR %dZ %dDim %dSt %dCW %dM %dRl %dF)",
            result.total_elements,
            result.walls, result.doors, result.windows, result.slabs,
            result.columns, result.beams, result.roofs, result.zones,
            result.dimensions, result.stairs, result.curtain_walls,
            result.meshes, result.railings, result.furniture,
        )
        return result

    def _build_floor(self, floor: FloorDef, result: BuildResult) -> None:
        logger.info("Building floor %d: %s", floor.index, floor.name)
        wall_guids = self._create_walls(floor, result)
        self._create_slabs(floor, result)
        self._create_columns(floor, result)
        self._create_beams(floor, result)
        self._create_curtain_walls(floor, result)
        self._create_doors(floor, result, wall_guids)
        self._create_windows(floor, result, wall_guids)
        self._create_stairs(floor, result)
        self._create_zones(floor, result)
        self._create_railings(floor, result)
        self._create_dimensions(floor, result)
        self._create_furniture(floor, result)

    def _create_walls(self, floor: FloorDef, result: BuildResult) -> list[str]:
        all_walls = list(floor.outer_walls) + list(floor.inner_walls)
        if not all_walls:
            return []

        walls_data = []
        for w in all_walls:
            walls_data.append({
                "begCoordinate": self._coord(w.start),
                "endCoordinate": self._coord(w.end),
                "zCoordinate": w.z_mm,
                "height": w.height_mm,
                "thickness": w.thickness_mm,
                "floorIndex": w.floor_index,
                "referenceLineLocation": w.reference_line,
                "buildingMaterialName": w.building_material or "",
                "compositeName": w.composite or "",
                "exteriorSurfaceName": w.exterior_surface or "",
                "interiorSurfaceName": w.interior_surface or "",
            })

        response = self._execute_addon_command("CreateWalls", {"wallsData": walls_data})
        result.walls += len(all_walls)
        logger.info("  [Floor %d] Created %d walls", floor.index, len(all_walls))

        wall_guids = []
        guid_list = response.get("wallGuids", [])
        for item in guid_list:
            wall_guids.append(item.get("guid", ""))
        return wall_guids

    def _create_doors(self, floor: FloorDef, result: BuildResult, wall_guids: list[str]) -> None:
        if not floor.doors:
            return

        doors_data = []
        for d in floor.doors:
            guid = wall_guids[d.wall_index] if d.wall_index < len(wall_guids) else ""
            doors_data.append({
                "ownerWallId": {"guid": guid},
                "centerOffset": d.position_mm,
                "width": d.width_mm,
                "height": d.height_mm,
                "sillHeight": d.sill_height_mm,
                "doorType": d.door_type,
                "materialName": d.material_name,
            })

        self._execute_addon_command("CreateDoors", {"doorsData": doors_data})
        result.doors += len(floor.doors)
        logger.info("  [Floor %d] Created %d doors", floor.index, len(floor.doors))

    def _create_windows(self, floor: FloorDef, result: BuildResult, wall_guids: list[str]) -> None:
        if not floor.windows:
            return

        windows_data = []
        for w in floor.windows:
            guid = wall_guids[w.wall_index] if w.wall_index < len(wall_guids) else ""
            windows_data.append({
                "ownerWallId": {"guid": guid},
                "centerOffset": w.position_mm,
                "width": w.width_mm,
                "height": w.height_mm,
                "sillHeight": w.sill_height_mm,
                "windowType": w.window_type,
                "materialName": w.material_name,
            })

        self._execute_addon_command("CreateWindows", {"windowsData": windows_data})
        result.windows += len(floor.windows)
        logger.info("  [Floor %d] Created %d windows", floor.index, len(floor.windows))

    def _create_slabs(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.slabs:
            return

        slabs_data = []
        for s in floor.slabs:
            slabs_data.append({
                "polygon": [self._coord(c) for c in s.polygon],
                "zCoordinate": s.z_mm,
                "floorIndex": s.floor_index,
                "thickness": s.thickness_mm,
                "buildingMaterialName": s.building_material,
                "compositeName": s.composite,
                "topSurfaceName": s.top_surface,
            })

        self._execute_addon_command("CreateSlabs", {"slabsData": slabs_data})
        result.slabs += len(floor.slabs)
        logger.info("  [Floor %d] Created %d slabs", floor.index, len(floor.slabs))

    def _create_columns(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.columns:
            return

        columns_data = []
        for c in floor.columns:
            columns_data.append({
                "position": self._coord(c.position),
                "zCoordinate": c.z_mm,
                "floorIndex": c.floor_index,
                "height": c.height_mm,
                "width": c.width_mm,
                "depth": c.depth_mm,
                "buildingMaterialName": c.building_material,
                "surfaceName": c.surface,
            })

        self._execute_addon_command("CreateColumns", {"columnsData": columns_data})
        result.columns += len(floor.columns)
        logger.info("  [Floor %d] Created %d columns", floor.index, len(floor.columns))

    def _create_beams(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.beams:
            return

        beams_data = []
        for b in floor.beams:
            beams_data.append({
                "begCoordinate": self._coord(b.start),
                "endCoordinate": self._coord(b.end),
                "zCoordinate": b.z_mm,
                "floorIndex": b.floor_index,
                "width": b.width_mm,
                "height": b.height_mm,
                "buildingMaterialName": b.building_material,
                "surfaceName": b.surface,
            })

        self._execute_addon_command("CreateBeams", {"beamsData": beams_data})
        result.beams += len(floor.beams)
        logger.info("  [Floor %d] Created %d beams", floor.index, len(floor.beams))

    def _create_stairs(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.stairs:
            return

        stairs_data = []
        for st in floor.stairs:
            stairs_data.append({
                "startCoordinate": self._coord(st.start),
                "endCoordinate": self._coord(st.end),
                "zCoordinate": 0,
                "floorIndex": st.floor_index,
                "width": st.width_mm,
                "totalHeight": st.total_height_mm,
                "stepCount": st.step_count,
                "treadDepth": st.tread_depth_mm,
                "direction": st.direction,
                "buildingMaterialName": st.material_name,
                "treadSurfaceName": st.tread_surface,
            })

        self._execute_addon_command("CreateStairs", {"stairsData": stairs_data})
        result.stairs += len(floor.stairs)
        logger.info("  [Floor %d] Created %d stairs", floor.index, len(floor.stairs))

    def _create_curtain_walls(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.curtain_walls:
            return

        cw_data = []
        for cw in floor.curtain_walls:
            cw_data.append({
                "polygon": [self._coord(c) for c in cw.polygon],
                "zCoordinate": cw.z_mm,
                "floorIndex": cw.floor_index,
                "height": cw.height_mm,
                "nominalWidth": cw.nominal_width_mm,
                "panelType": cw.panel_type,
                "frameMaterialName": cw.frame_material,
                "glassSurfaceName": cw.glass_surface,
            })

        self._execute_addon_command("CreateCurtainWalls", {"curtainWallsData": cw_data})
        result.curtain_walls += len(floor.curtain_walls)
        logger.info("  [Floor %d] Created %d curtain walls", floor.index, len(floor.curtain_walls))

    def _create_railings(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.railings:
            return

        railings_data = []
        for r in floor.railings:
            railings_data.append({
                "polygon": [self._coord(c) for c in r.polygon],
                "zCoordinate": r.z_mm,
                "floorIndex": r.floor_index,
                "height": r.height_mm,
                "materialName": r.material_name,
            })

        self._execute_addon_command("CreateRailings", {"railingsData": railings_data})
        result.railings += len(floor.railings)
        logger.info("  [Floor %d] Created %d railings", floor.index, len(floor.railings))

    def _create_zones(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.zones:
            return

        zones_data = []
        for z in floor.zones:
            zones_data.append({
                "polygon": [self._coord(c) for c in z.polygon],
                "zCoordinate": 0,
                "floorIndex": z.floor_index,
                "roomName": z.room_name,
                "roomNumber": z.room_number,
                "category": z.category,
                "floorSurfaceName": z.floor_surface,
                "wallSurfaceName": z.wall_surface,
                "ceilingSurfaceName": z.ceiling_surface,
            })

        self._execute_addon_command("CreateZones", {"zonesData": zones_data})
        result.zones += len(floor.zones)
        logger.info("  [Floor %d] Created %d zones", floor.index, len(floor.zones))

    def _create_dimensions(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.dimensions:
            return

        dims_data = []
        for d in floor.dimensions:
            dims_data.append({
                "type": d.dim_type,
                "points": [self._coord(p) for p in d.points],
                "offset": d.offset_mm,
                "floorIndex": d.floor_index,
            })

        self._execute_addon_command("CreateDimensions", {"dimensionsData": dims_data})
        result.dimensions += len(floor.dimensions)
        logger.info("  [Floor %d] Created %d dimensions", floor.index, len(floor.dimensions))

    def _create_furniture(self, floor: FloorDef, result: BuildResult) -> None:
        if not floor.furniture:
            return

        furniture_data = []
        for f in floor.furniture:
            furniture_data.append({
                "libraryPartName": f.library_part_name,
                "genericType": f.generic_type,
                "position": self._coord(f.position),
                "zCoordinate": f.z_mm,
                "floorIndex": f.floor_index,
                "rotation": f.rotation_deg,
                "params": f.params,
            })

        self._execute_addon_command("CreateFurniture", {"furnitureData": furniture_data})
        result.furniture += len(floor.furniture)
        logger.info("  [Floor %d] Created %d furniture items", floor.index, len(floor.furniture))

    def _create_roofs(self, proj, result: BuildResult) -> None:
        if not proj.roofs:
            return

        roofs_data = []
        for r in proj.roofs:
            roofs_data.append({
                "polygon": [self._coord(c) for c in r.polygon],
                "zCoordinate": r.z_mm,
                "floorIndex": r.floor_index,
                "angle": r.angle_deg,
                "thickness": r.thickness_mm,
                "roofType": r.roof_type,
                "buildingMaterialName": r.building_material,
                "compositeName": r.composite,
                "topSurfaceName": r.top_surface,
            })

        self._execute_addon_command("CreateRoofs", {"roofsData": roofs_data})
        result.roofs += len(proj.roofs)
        logger.info("Created %d roofs", len(proj.roofs))

    def _create_site_mesh(self, proj, result: BuildResult) -> None:
        site = proj.site
        if site.terrain_mesh and site.property_boundary:
            m = site.terrain_mesh
            height_pts = []
            for h in m.height_points:
                height_pts.append({"x": h[0], "y": h[1], "z": h[2]})

            mesh_data = [{
                "polygon": [self._coord(c) for c in (site.property_boundary or m.polygon)],
                "zCoordinate": m.z_mm,
                "floorIndex": m.floor_index,
                "buildingMaterialName": m.building_material,
                "topSurfaceName": m.top_surface,
                "heightPoints": height_pts,
            }]

            self._execute_addon_command("CreateMeshes", {"meshesData": mesh_data})
            result.meshes += 1
            logger.info("Created site mesh")

    def _create_outdoor_furniture(self, proj, result: BuildResult) -> None:
        if not proj.outdoor_furniture:
            return

        furniture_data = []
        for f in proj.outdoor_furniture:
            furniture_data.append({
                "libraryPartName": f.library_part_name,
                "genericType": f.generic_type,
                "position": self._coord(f.position),
                "zCoordinate": f.z_mm,
                "floorIndex": f.floor_index,
                "rotation": f.rotation_deg,
                "params": f.params,
            })

        self._execute_addon_command("CreateFurniture", {"furnitureData": furniture_data})
        result.furniture += len(proj.outdoor_furniture)
        logger.info("Created %d outdoor furniture items", len(proj.outdoor_furniture))

    def _assign_materials(self, plan: BuildingPlan, result: BuildResult) -> None:
        assignments = []
        for floor in plan.project.floors:
            for z in floor.zones:
                if z.wall_surface:
                    assignments.append({
                        "elementType": "zone",
                        "face": "wall",
                        "surfaceName": z.wall_surface,
                    })
                if z.floor_surface:
                    assignments.append({
                        "elementType": "zone",
                        "face": "floor",
                        "surfaceName": z.floor_surface,
                    })
                if z.ceiling_surface:
                    assignments.append({
                        "elementType": "zone",
                        "face": "ceiling",
                        "surfaceName": z.ceiling_surface,
                    })

        if assignments:
            try:
                self._execute_addon_command("SetElementSurfaces", {"surfaceAssignments": assignments})
                result.surfaces_assigned = len(assignments)
                logger.info("Applied %d surface assignments", len(assignments))
            except Exception as e:
                logger.warning("Surface assignment failed: %s", e)
