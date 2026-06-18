#include "CreateRailingsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateRailingsCommand::GetName() const { return "CreateRailings"; }

GS::UniString CreateRailingsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "railingsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "height": { "type": "number", "default": 1000 }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["railingsData"]
})";
}

GS::UniString CreateRailingsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"railingGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateRailingsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> railingsData;
    parameters.Get("railingsData", railingsData);
    GS::Array<GS::ObjectState> railingGuids;

    ACAPI_CallUndoableCommand("Create Railings", [&]() -> GSErrCode {
        for (const auto& data : railingsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_RailingID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            element.railing.bottomOffset = 0.0;
            element.railing.referenceLinePen = 1;
            element.railing.contourPen = 1;

            short floorIndex = 0;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

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
            railingGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("railingGuids", railingGuids);
    return response;
}

} // namespace AchicadAutomation
