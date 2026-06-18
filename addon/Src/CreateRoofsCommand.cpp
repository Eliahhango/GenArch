#include "CreateRoofsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateRoofsCommand::GetName() const { return "CreateRoofs"; }

GS::UniString CreateRoofsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "roofsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 3000 },
          "floorIndex": { "type": "integer", "default": 1 },
          "angle": { "type": "number", "default": 30 },
          "thickness": { "type": "number", "default": 300 },
          "buildingMaterialName": { "type": "string", "default": "" }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["roofsData"]
})";
}

GS::UniString CreateRoofsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"roofGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateRoofsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> roofsData;
    parameters.Get("roofsData", roofsData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> roofGuids;

    ACAPI_CallUndoableCommand("Create Roofs", [&]() -> GSErrCode {
        for (const auto& data : roofsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_RoofID);
            if (err2 != NoError) return err2;

            element.roof.roofClass = API_PlaneRoofID;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            element.roof.u.planeRoof.poly.nCoords = polygonData.GetSize();
            element.roof.u.planeRoof.poly.nSubPolys = 1;
            element.roof.u.planeRoof.poly.nArcs = 0;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(
                (polygonData.GetSize() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            for (UIndex i = 0; i < polygonData.GetSize(); ++i) {
                (*memo.coords)[i] = Get2DCoordinateFromObjectState(polygonData[i]);
            }
            (*memo.coords)[polygonData.GetSize()] = (*memo.coords)[0];
            (*memo.pends)[0] = polygonData.GetSize();

            double zCoordinate = 3000.0, angle = 30.0, thickness = 300.0;
            data.Get("zCoordinate", zCoordinate);
            data.Get("angle", angle);
            data.Get("thickness", thickness);

            element.roof.shellBase.thickness = thickness;
            element.roof.u.planeRoof.angle = angle;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; }

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            roofGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("roofGuids", roofGuids);
    return response;
}

} // namespace AchicadAutomation
