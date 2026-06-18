#include "CreateCurtainWallsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateCurtainWallsCommand::GetName() const { return "CreateCurtainWalls"; }

GS::UniString CreateCurtainWallsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "curtainWallsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "height": { "type": "number", "default": 3000 },
          "nominalWidth": { "type": "number", "default": 300 }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["curtainWallsData"]
})";
}

GS::UniString CreateCurtainWallsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"curtainWallGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateCurtainWallsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> cwData;
    parameters.Get("curtainWallsData", cwData);
    GS::Array<GS::ObjectState> cwGuids;

    ACAPI_CallUndoableCommand("Create Curtain Walls", [&]() -> GSErrCode {
        for (const auto& data : cwData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            API_SubElement marker = {};
            GSErrCode err2 = GetElementDefaultsExt(element, memo, marker, API_CurtainWallID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            double height = 3000.0, nominalWidth = 300.0;
            data.Get("height", height);
            data.Get("nominalWidth", nominalWidth);

            element.curtainWall.height = height;
            element.curtainWall.nominalWidth = nominalWidth;
            element.curtainWall.angle = 0.0;
            element.curtainWall.flipped = false;

            BNZeroMemory(&element.curtainWall.planeMatrix, sizeof(API_Tranmat));
            element.curtainWall.planeMatrix.tmx[0] = 1.0;
            element.curtainWall.planeMatrix.tmx[5] = 1.0;
            element.curtainWall.planeMatrix.tmx[10] = 1.0;

            element.curtainWall.polygon.nCoords = polygonData.GetSize();
            element.curtainWall.polygon.nSubPolys = 1;
            element.curtainWall.polygon.nArcs = 0;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(
                (polygonData.GetSize() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            for (UIndex i = 0; i < polygonData.GetSize(); ++i) {
                (*memo.coords)[i] = Get2DCoordinateFromObjectState(polygonData[i]);
            }
            (*memo.coords)[polygonData.GetSize()] = (*memo.coords)[0];
            (*memo.pends)[0] = polygonData.GetSize();

            short floorIndex = 0;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

            err2 = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            cwGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("curtainWallGuids", cwGuids);
    return response;
}

} // namespace AchicadAutomation
