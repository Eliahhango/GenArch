#include "CreateSlabsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateSlabsCommand::GetName() const { return "CreateSlabs"; }

GS::UniString CreateSlabsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "slabsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "thickness": { "type": "number", "default": 150 }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["slabsData"]
})";
}

GS::UniString CreateSlabsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"slabGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateSlabsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> slabsData;
    parameters.Get("slabsData", slabsData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> slabGuids;

    ACAPI_CallUndoableCommand("Create Slabs", [&]() -> GSErrCode {
        for (const auto& data : slabsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_SlabID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            element.slab.poly.nCoords = polygonData.GetSize();
            element.slab.poly.nSubPolys = 1;
            element.slab.poly.nArcs = 0;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(
                (polygonData.GetSize() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            for (UIndex i = 0; i < polygonData.GetSize(); ++i) {
                (*memo.coords)[i] = Get2DCoordinateFromObjectState(polygonData[i]);
            }
            (*memo.coords)[polygonData.GetSize()] = (*memo.coords)[0];
            (*memo.pends)[0] = polygonData.GetSize();

            double zCoordinate = 0.0, thickness = 150.0;
            data.Get("zCoordinate", zCoordinate);
            data.Get("thickness", thickness);
            element.slab.thickness = thickness;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; }

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            slabGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("slabGuids", slabGuids);
    return response;
}

} // namespace AchicadAutomation
