#include "CreateObjectsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateObjectsCommand::GetName() const { return "CreateObjects"; }

GS::UniString CreateObjectsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "objectsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "position": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "rotation": { "type": "number", "default": 0 }
        },
        "required": ["position"]
      }
    }
  },
  "required": ["objectsData"]
})";
}

GS::UniString CreateObjectsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"objectGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateObjectsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> objectsData;
    parameters.Get("objectsData", objectsData);
    GS::Array<GS::ObjectState> objectGuids;

    ACAPI_CallUndoableCommand("Create Objects", [&]() -> GSErrCode {
        for (const auto& data : objectsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            API_SubElement marker = {};
            GSErrCode err2 = GetElementDefaultsExt(element, memo, marker, API_ObjectID);
            if (err2 != NoError) return err2;

            element.object.pos = Get2DCoordinateFromObjectState(*data.Get("position"));

            double rotation = 0.0;
            data.Get("rotation", rotation);
            element.object.angle = rotation;

            short floorIndex = 0;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

            err2 = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            objectGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("objectGuids", objectGuids);
    return response;
}

} // namespace AchicadAutomation
