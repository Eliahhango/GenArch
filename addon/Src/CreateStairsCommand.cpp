#include "CreateStairsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateStairsCommand::GetName() const { return "CreateStairs"; }

GS::UniString CreateStairsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "stairsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "startCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "endCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "width": { "type": "number", "default": 1000 },
          "totalHeight": { "type": "number", "default": 3000 },
          "stepCount": { "type": "integer", "default": 16 },
          "treadDepth": { "type": "number", "default": 280 }
        },
        "required": ["startCoordinate", "endCoordinate"]
      }
    }
  },
  "required": ["stairsData"]
})";
}

GS::UniString CreateStairsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"stairGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateStairsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> stairsData;
    parameters.Get("stairsData", stairsData);
    GS::Array<GS::ObjectState> stairGuids;

    ACAPI_CallUndoableCommand("Create Stairs", [&]() -> GSErrCode {
        for (const auto& data : stairsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_StairID);
            if (err2 != NoError) return err2;

            double width = 1000.0, treadDepth = 280.0;
            int stepCount = 16;
            data.Get("width", width);
            data.Get("treadDepth", treadDepth);
            data.Get("stepCount", stepCount);

            API_Coord startC = Get2DCoordinateFromObjectState(*data.Get("startCoordinate"));
            API_Coord endC = Get2DCoordinateFromObjectState(*data.Get("endCoordinate"));

            double totalHeight = 3000.0;
            data.Get("totalHeight", totalHeight);

            element.stair.flightWidth = width;
            element.stair.stepNum = stepCount;
            element.stair.treadNum = stepCount - 1;
            element.stair.totalHeight = totalHeight;
            element.stair.riserHeight = totalHeight / stepCount;
            element.stair.treadDepth = treadDepth;

            short floorIndex = 0;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(3 * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            (*memo.coords)[0] = startC;
            (*memo.coords)[1] = endC;
            (*memo.coords)[2] = startC;
            (*memo.pends)[0] = 2;

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            stairGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("stairGuids", stairGuids);
    return response;
}

} // namespace AchicadAutomation
