#include "CreateColumnsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateColumnsCommand::GetName() const { return "CreateColumns"; }

GS::UniString CreateColumnsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "columnsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "position": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "height": { "type": "number", "default": 3000 }
        },
        "required": ["position"]
      }
    }
  },
  "required": ["columnsData"]
})";
}

GS::UniString CreateColumnsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"columnGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateColumnsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> columnsData;
    parameters.Get("columnsData", columnsData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> columnGuids;

    ACAPI_CallUndoableCommand("Create Columns", [&]() -> GSErrCode {
        for (const auto& data : columnsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_ColumnID);
            if (err2 != NoError) return err2;

            element.column.origoPos = Get2DCoordinateFromObjectState(*data.Get("position"));

            double zCoordinate = 0.0, height = 3000.0;
            data.Get("zCoordinate", zCoordinate);
            data.Get("height", height);
            element.column.height = height;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; element.column.bottomOffset = fi.second; }

            err2 = ACAPI_Element_Create(&element, &memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            columnGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("columnGuids", columnGuids);
    return response;
}

} // namespace AchicadAutomation
