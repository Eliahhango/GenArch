#include "CreateWindowsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateWindowsCommand::GetName() const
{
    return "CreateWindows";
}

GS::UniString CreateWindowsCommand::GetInputSchema() const
{
    return R"({
  "type": "object",
  "properties": {
    "windowsData": {
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
          "width": { "type": "number", "default": 1200 },
          "height": { "type": "number", "default": 1500 },
          "sillHeight": { "type": "number", "default": 900 }
        },
        "required": ["ownerWallId", "centerOffset"]
      }
    }
  },
  "required": ["windowsData"]
})";
}

GS::UniString CreateWindowsCommand::GetOutputSchema() const
{
    return R"({
  "type": "object",
  "properties": {
    "windowGuids": {
      "type": "array",
      "items": { "type": "object", "properties": { "guid": { "type": "string" } } }
    }
  }
})";
}

GS::ObjectState CreateWindowsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> windowsData;
    parameters.Get("windowsData", windowsData);

    GS::Array<GS::ObjectState> windowGuids;

    ACAPI_CallUndoableCommand("Create Windows", [&]() -> GSErrCode {
        for (const auto& data : windowsData) {
            API_Guid wallGuid = GetGuidFromObjectState(*data.Get("ownerWallId"));
            if (!DoesWallExist(wallGuid)) continue;

            API_Element element = {};
            API_ElementMemo memo = {};
            API_SubElement marker = {};
            GSErrCode err2 = GetElementDefaultsExt(element, memo, marker, API_WindowID);
            if (err2 != NoError) return err2;

            double centerOffset = 0.0;
            data.Get("centerOffset", centerOffset);

            element.window.owner = wallGuid;
            element.window.objLoc = centerOffset;

            double sillHeight = 900.0;
            data.Get("sillHeight", sillHeight);
            element.window.lower = sillHeight;

            err2 = ACAPI_Element_CreateExt(&element, &memo, 1UL, &marker);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            windowGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("windowGuids", windowGuids);
    return response;
}

} // namespace AchicadAutomation
