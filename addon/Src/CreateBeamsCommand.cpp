#include "CreateBeamsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateBeamsCommand::GetName() const { return "CreateBeams"; }

GS::UniString CreateBeamsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "beamsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "begCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "endCoordinate": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "zCoordinate": { "type": "number", "default": 3000 },
          "floorIndex": { "type": "integer", "default": 0 }
        },
        "required": ["begCoordinate", "endCoordinate"]
      }
    }
  },
  "required": ["beamsData"]
})";
}

GS::UniString CreateBeamsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"beamGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateBeamsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> beamsData;
    parameters.Get("beamsData", beamsData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> beamGuids;

    ACAPI_CallUndoableCommand("Create Beams", [&]() -> GSErrCode {
        for (const auto& data : beamsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_BeamID);
            if (err2 != NoError) return err2;

            element.beam.begC = Get2DCoordinateFromObjectState(*data.Get("begCoordinate"));
            element.beam.endC = Get2DCoordinateFromObjectState(*data.Get("endCoordinate"));

            double zCoordinate = 3000.0;
            data.Get("zCoordinate", zCoordinate);
            element.beam.level = zCoordinate;

            short floorIndex = -1;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; }

            err2 = ACAPI_Element_Create(&element, &memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            beamGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("beamGuids", beamGuids);
    return response;
}

} // namespace AchicadAutomation
