#include "CreateMeshesCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateMeshesCommand::GetName() const { return "CreateMeshes"; }

GS::UniString CreateMeshesCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "meshesData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": -1 }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["meshesData"]
})";
}

GS::UniString CreateMeshesCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"meshGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateMeshesCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> meshesData;
    parameters.Get("meshesData", meshesData);
    GS::Array<GS::ObjectState> meshGuids;

    ACAPI_CallUndoableCommand("Create Meshes", [&]() -> GSErrCode {
        for (const auto& data : meshesData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_MeshID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            element.mesh.level = 0.0;
            element.mesh.skirt = 1;
            element.mesh.materialsChained = true;
            element.mesh.useFloorFill = false;
            element.mesh.contPen = 1;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

            element.mesh.poly.nCoords = polygonData.GetSize();
            element.mesh.poly.nSubPolys = 1;
            element.mesh.poly.nArcs = 0;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(
                (polygonData.GetSize() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            for (UIndex i = 0; i < polygonData.GetSize(); ++i) {
                (*memo.coords)[i] = Get2DCoordinateFromObjectState(polygonData[i]);
            }
            (*memo.coords)[polygonData.GetSize()] = (*memo.coords)[0];
            (*memo.pends)[0] = polygonData.GetSize();

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            meshGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("meshGuids", meshGuids);
    return response;
}

} // namespace AchicadAutomation
