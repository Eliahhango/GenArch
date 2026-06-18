import json, sys
from pathlib import Path

path = sys.argv[1] if len(sys.argv) > 1 else "plan_full.json"

with open(path, encoding="utf-8") as f:
    data = json.load(f)

p = data["project"]
print(f"=== {p['name']} === Style: {p['style']} ===\n")

for floor in p["floors"]:
    print(f"Floor {floor['index']}: {floor['name']} ({floor['height_mm']}mm)")
    print(f"  Outer walls: {len(floor['outer_walls'])} | Inner: {len(floor['inner_walls'])}")

    for i, w in enumerate(floor["outer_walls"][:2]):
        print(f"    Wall {i}: mat='{w.get('building_material','')}' ext='{w.get('exterior_surface','')}' int='{w.get('interior_surface','')}'")
    for i, w in enumerate(floor["inner_walls"][:2]):
        print(f"    Inner {i}: mat='{w.get('building_material','')}'")

    for i, z in enumerate(floor["zones"][:4]):
        print(f"    Zone '{z['room_name']}': floor='{z.get('floor_surface','')}' wall='{z.get('wall_surface','')}'")

    # Count furniture per room
    furn_by_room = {}
    for f_item in floor["furniture"]:
        pos = f_item.get("position", {})
        fx, fy = pos.get("x", 0), pos.get("y", 0)
        for z in floor["zones"]:
            poly = z.get("polygon", [])
            if poly:
                minx = min(pt["x"] for pt in poly)
                maxx = max(pt["x"] for pt in poly)
                miny = min(pt["y"] for pt in poly)
                maxy = max(pt["y"] for pt in poly)
                if minx - 500 <= fx <= maxx + 500 and miny - 500 <= fy <= maxy + 500:
                    name = z["room_name"]
                    furn_by_room[name] = furn_by_room.get(name, 0) + 1
                    break
    for name, count in sorted(furn_by_room.items()):
        print(f"    Room '{name}': {count} furniture items")

    for s in floor.get("stairs", []):
        print(f"    Stairs: {s.get('width_mm')}mm wide, {s.get('step_count')} steps, material='{s.get('material_name','')}'")

    print()

# Roofs
for r in p.get("roofs", []):
    print(f"Roof: type={r.get('roof_type','')} angle={r.get('angle_deg','')} deg, material='{r.get('building_material','')}' surface='{r.get('top_surface','')}'")

# Site
site = p.get("site", {})
boundary = site.get("property_boundary", [])
print(f"\nSite: boundary={len(boundary)} points, has_terrain={bool(site.get('terrain_mesh'))}")
print(f"Setbacks: front={site.get('setback_front_mm')} side={site.get('setback_side_mm')} rear={site.get('setback_rear_mm')}")
print(f"Outdoor furniture: {len(p.get('outdoor_furniture', []))} items")
for f in p.get("outdoor_furniture", []):
    print(f"  {f.get('library_part_name','')} {f.get('generic_type','')} at ({f.get('position',{}).get('x',0)}, {f.get('position',{}).get('y',0)})")

# Totals
total_furn = sum(len(fl["furniture"]) for fl in p["floors"])
total_zones = sum(len(fl["zones"]) for fl in p["floors"])
total_stairs = sum(len(fl["stairs"]) for fl in p["floors"])
print(f"\nTotals: {total_zones} zones, {total_stairs} stairs, {total_furn} furniture items")
