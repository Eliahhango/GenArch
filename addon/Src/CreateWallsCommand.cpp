#include "CreateWallsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateWallsCommand::GetName() const { return "CreateWalls"; }

GS::UniString CreateWallsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "wallsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "begCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "endCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "zCoordinate": { "type": "number", "default": 0 },
          "height": { "type": "number", "default": 3000 },
          "thickness": { "type": "number", "default": 300 },
          "floorIndex": { "type": "integer", "default": 0 }
        },
        "required": ["begCoordinate", "endCoordinate"]
      }
    }
  },
  "required": ["wallsData"]
})";
}

GS::UniString CreateWallsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"wallGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateWallsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> wallsData;
    parameters.Get("wallsData", wallsData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> wallGuids;

    ACAPI_CallUndoableCommand("Create Walls", [&]() -> GSErrCode {
        for (const auto& data : wallsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_WallID);
            if (err2 != NoError) return err2;

            element.wall.type = APIWtyp_Normal;
            element.wall.begC = Get2DCoordinateFromObjectState(*data.Get("begCoordinate"));
            element.wall.endC = Get2DCoordinateFromObjectState(*data.Get("endCoordinate"));
            element.wall.modelElemStructureType = API_BasicStructure;
            element.wall.offset = 0.0;

            double zCoordinate = 0.0, height = 3000.0, thickness = 300.0;
            data.Get("zCoordinate", zCoordinate);
            data.Get("height", height);
            data.Get("thickness", thickness);
            element.wall.height = height;
            element.wall.thickness = thickness;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; element.wall.bottomOffset = fi.second; }

            err2 = ACAPI_Element_Create(&element, &memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            wallGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("wallGuids", wallGuids);
    return response;
}

} // namespace AchicadAutomation
