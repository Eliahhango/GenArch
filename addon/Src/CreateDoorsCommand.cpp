#include "CreateDoorsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateDoorsCommand::GetName() const
{
    return "CreateDoors";
}

GS::UniString CreateDoorsCommand::GetInputSchema() const
{
    return R"({
  "type": "object",
  "properties": {
    "doorsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "ownerWallId": {
            "type": "object",
            "properties": { "guid": { "type": "string" } },
            "required": ["guid"]
          },
          "centerOffset": { "type": "number" },
          "width": { "type": "number", "default": 900 },
          "height": { "type": "number", "default": 2100 },
          "sillHeight": { "type": "number", "default": 0 }
        },
        "required": ["ownerWallId", "centerOffset"]
      }
    }
  },
  "required": ["doorsData"]
})";
}

GS::UniString CreateDoorsCommand::GetOutputSchema() const
{
    return R"({
  "type": "object",
  "properties": {
    "doorGuids": {
      "type": "array",
      "items": { "type": "object", "properties": { "guid": { "type": "string" } } }
    }
  }
})";
}

GS::ObjectState CreateDoorsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> doorsData;
    parameters.Get("doorsData", doorsData);

    GS::Array<GS::ObjectState> doorGuids;

    ACAPI_CallUndoableCommand("Create Doors", [&]() -> GSErrCode {
        for (const auto& data : doorsData) {
            API_Guid wallGuid = GetGuidFromObjectState(*data.Get("ownerWallId"));
            if (!DoesWallExist(wallGuid)) continue;

            API_Element element = {};
            API_ElementMemo memo = {};
            API_SubElement marker = {};
            GSErrCode err2 = GetElementDefaultsExt(element, memo, marker, API_DoorID);
            if (err2 != NoError) return err2;

            double centerOffset = 0.0;
            data.Get("centerOffset", centerOffset);

            element.window.owner = wallGuid;
            element.window.objLoc = centerOffset;

            double sillHeight = 0.0;
            data.Get("sillHeight", sillHeight);
            element.window.lower = sillHeight;

            err2 = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            doorGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("doorGuids", doorGuids);
    return response;
}

} // namespace AchicadAutomation
