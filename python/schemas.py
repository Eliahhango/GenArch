from __future__ import annotations

from pydantic import BaseModel, Field


class Coordinate(BaseModel):
    x: float
    y: float


class SurfaceAssignment(BaseModel):
    face: str = Field(description="Face name: top, bottom, left, right, front, back, side, exterior, interior")
    material_name: str = Field(description="Building material or surface name in Archicad")


class BuildingMaterialDef(BaseModel):
    element_type: str = Field(description="wall | slab | roof | beam | column | mesh | stair")
    material_name: str = Field(description="Building material name (e.g. 'Concrete - Structural', 'Brick - Common')")


class CompositeDef(BaseModel):
    composite_name: str = Field(description="Archicad composite name (e.g. 'Cavity Wall - Brick/Block')")


class WallDef(BaseModel):
    start: Coordinate
    end: Coordinate
    height_mm: float = Field(default=3000, description="Wall height in millimeters")
    thickness_mm: float = Field(default=300, description="Wall thickness in millimeters")
    z_mm: float = Field(default=0, description="Absolute Z coordinate in millimeters")
    floor_index: int = Field(default=0, description="Floor index (0 = ground floor)")
    reference_line: str = Field(default="center", description="center | outside | inside")
    building_material: str = Field(default="", description="Building material name")
    composite: str = Field(default="", description="Composite structure name")
    exterior_surface: str = Field(default="", description="Exterior surface material name")
    interior_surface: str = Field(default="", description="Interior surface material name")


class DoorDef(BaseModel):
    wall_index: int = Field(description="Index into the wall arrays")
    wall_type: str = Field(default="outer", description="outer | inner — which wall array to index")
    position_mm: float = Field(description="Distance along wall from start coordinate in mm")
    width_mm: float = Field(default=900)
    height_mm: float = Field(default=2100)
    sill_height_mm: float = Field(default=0)
    door_type: str = Field(default="Hinged Single", description="Hinged Single | Hinged Double | Sliding | Garage | Glass Door")
    material_name: str = Field(default="", description="Door surface/frame material")


class WindowDef(BaseModel):
    wall_index: int = Field(description="Index into the wall arrays")
    wall_type: str = Field(default="outer", description="outer | inner — which wall array to index")
    position_mm: float = Field(description="Distance along wall from start coordinate in mm")
    width_mm: float = Field(default=1200)
    height_mm: float = Field(default=1500)
    sill_height_mm: float = Field(default=900)
    window_type: str = Field(default="Casement", description="Casement | Fixed | Sliding | Awning | Bay Window")
    material_name: str = Field(default="", description="Window frame material")


class SlabDef(BaseModel):
    floor_index: int = Field(default=0)
    polygon: list[Coordinate]
    thickness_mm: float = Field(default=150)
    z_mm: float = Field(default=0)
    building_material: str = Field(default="Concrete - Structural")
    top_surface: str = Field(default="", description="Top surface material")
    composite: str = Field(default="", description="Composite structure name")


class ColumnDef(BaseModel):
    position: Coordinate
    floor_index: int = Field(default=0)
    height_mm: float = Field(default=3000)
    width_mm: float = Field(default=300)
    depth_mm: float = Field(default=300)
    z_mm: float = Field(default=0)
    building_material: str = Field(default="Concrete - Structural")
    surface: str = Field(default="", description="Column surface material")


class BeamDef(BaseModel):
    start: Coordinate
    end: Coordinate
    floor_index: int = Field(default=0)
    width_mm: float = Field(default=200)
    height_mm: float = Field(default=300)
    z_mm: float = Field(default=3000)
    building_material: str = Field(default="Concrete - Structural")
    surface: str = Field(default="")


class RoofDef(BaseModel):
    floor_index: int = Field(default=1)
    polygon: list[Coordinate]
    angle_deg: float = Field(default=30)
    thickness_mm: float = Field(default=300)
    roof_type: str = Field(default="gable", description="gable | hip | flat | shed | mansard")
    z_mm: float = Field(default=3000)
    building_material: str = Field(default="")
    top_surface: str = Field(default="", description="Roof covering surface")
    composite: str = Field(default="", description="Composite structure name")


class ZoneDef(BaseModel):
    floor_index: int = Field(default=0)
    polygon: list[Coordinate]
    room_name: str = Field(default="Room")
    room_number: str = Field(default="")
    category: str = Field(default="", description="Living | Bedroom | Kitchen | Bathroom | Corridor | Office | Garage | Storage")
    floor_surface: str = Field(default="", description="Floor finish material name")
    wall_surface: str = Field(default="", description="Interior wall surface material")
    ceiling_surface: str = Field(default="")


class DimensionDef(BaseModel):
    dim_type: str = Field(default="linear", description="linear | aligned | angular | radial | elevation")
    points: list[Coordinate]
    offset_mm: float = Field(default=500)
    floor_index: int = Field(default=0)


class StairDef(BaseModel):
    floor_index: int = Field(default=0)
    start: Coordinate
    end: Coordinate
    width_mm: float = Field(default=1000)
    total_height_mm: float = Field(default=3000)
    step_count: int = Field(default=16)
    tread_depth_mm: float = Field(default=280)
    direction: str = Field(default="up", description="up | down")
    material_name: str = Field(default="Concrete - Structural")
    tread_surface: str = Field(default="")


class CurtainWallDef(BaseModel):
    floor_index: int = Field(default=0)
    polygon: list[Coordinate]
    height_mm: float = Field(default=3000)
    nominal_width_mm: float = Field(default=300)
    z_mm: float = Field(default=0)
    panel_type: str = Field(default="Glass", description="Glass | Solid | Louver")
    frame_material: str = Field(default="Metal - Aluminium")
    glass_surface: str = Field(default="Glass - Clear")


class MeshDef(BaseModel):
    floor_index: int = Field(default=-1, description="Negative for terrain (below ground floor)")
    polygon: list[Coordinate]
    z_mm: float = Field(default=0)
    building_material: str = Field(default="Earth")
    top_surface: str = Field(default="", description="Terrain/grass surface")
    height_points: list[tuple[float, float, float]] = Field(
        default_factory=list,
        description="List of (x, y, z_mm) elevation points for terrain shaping"
    )


class RailingDef(BaseModel):
    floor_index: int = Field(default=0)
    polygon: list[Coordinate]
    height_mm: float = Field(default=1000)
    z_mm: float = Field(default=0)
    material_name: str = Field(default="Metal - Aluminium")


class FurnitureDef(BaseModel):
    library_part_name: str = Field(
        default="",
        description="Archicad library part name (e.g. 'Dining Table 001', 'Sofa 001', 'Bed 001')"
    )
    generic_type: str = Field(
        default="",
        description="Furniture category if library part unknown: table | chair | sofa | bed | cabinet | sink | toilet | bathtub | shower | stove | refrigerator | desk | bookshelf | plant"
    )
    position: Coordinate
    floor_index: int = Field(default=0)
    z_mm: float = Field(default=0)
    rotation_deg: float = Field(default=0)
    params: dict[str, float | str] = Field(default_factory=dict)


class FloorDef(BaseModel):
    index: int = Field(default=0)
    name: str = Field(default="Ground Floor")
    height_mm: float = Field(default=3000, description="Floor-to-floor height")
    outer_walls: list[WallDef] = Field(default_factory=list)
    inner_walls: list[WallDef] = Field(default_factory=list)
    doors: list[DoorDef] = Field(default_factory=list)
    windows: list[WindowDef] = Field(default_factory=list)
    slabs: list[SlabDef] = Field(default_factory=list)
    columns: list[ColumnDef] = Field(default_factory=list)
    beams: list[BeamDef] = Field(default_factory=list)
    zones: list[ZoneDef] = Field(default_factory=list)
    stairs: list[StairDef] = Field(default_factory=list)
    curtain_walls: list[CurtainWallDef] = Field(default_factory=list)
    railings: list[RailingDef] = Field(default_factory=list)
    dimensions: list[DimensionDef] = Field(default_factory=list)
    furniture: list[FurnitureDef] = Field(default_factory=list)


class SiteDef(BaseModel):
    property_boundary: list[Coordinate] = Field(default_factory=list)
    terrain_mesh: MeshDef | None = Field(default=None)
    setback_front_mm: float = Field(default=5000)
    setback_side_mm: float = Field(default=3000)
    setback_rear_mm: float = Field(default=5000)


class ProjectDef(BaseModel):
    name: str = Field(default="AI Generated Building")
    description: str = Field(default="")
    style: str = Field(default="modern", description="modern | traditional | contemporary | minimalist | industrial")
    floors: list[FloorDef] = Field(default_factory=list)
    roofs: list[RoofDef] = Field(default_factory=list)
    site: SiteDef = Field(default_factory=SiteDef)
    outdoor_furniture: list[FurnitureDef] = Field(default_factory=list)


class BuildingPlan(BaseModel):
    project: ProjectDef = Field(default_factory=ProjectDef)
