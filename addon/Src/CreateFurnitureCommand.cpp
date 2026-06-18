#include "CreateFurnitureCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateFurnitureCommand::GetName() const { return "CreateFurniture"; }

GS::UniString CreateFurnitureCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "furnitureData": {
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
  "required": ["furnitureData"]
})";
}

GS::UniString CreateFurnitureCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"furnitureGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateFurnitureCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> furnitureData;
    parameters.Get("furnitureData", furnitureData);
    GS::Array<GS::ObjectState> furnitureGuids;

    ACAPI_CallUndoableCommand("Create Furniture", [&]() -> GSErrCode {
        for (const auto& data : furnitureData) {
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
            furnitureGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("furnitureGuids", furnitureGuids);
    return response;
}

} // namespace AchicadAutomation
