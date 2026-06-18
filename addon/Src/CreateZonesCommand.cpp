#include "CreateZonesCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateZonesCommand::GetName() const { return "CreateZones"; }

GS::UniString CreateZonesCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "zonesData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "polygon": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "zCoordinate": { "type": "number", "default": 0 },
          "floorIndex": { "type": "integer", "default": 0 },
          "roomName": { "type": "string", "default": "Room" },
          "roomNumber": { "type": "string", "default": "" }
        },
        "required": ["polygon"]
      }
    }
  },
  "required": ["zonesData"]
})";
}

GS::UniString CreateZonesCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"zoneGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateZonesCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> zonesData;
    parameters.Get("zonesData", zonesData);
    const Stories stories = GetStories();
    GS::Array<GS::ObjectState> zoneGuids;

    ACAPI_CallUndoableCommand("Create Zones", [&]() -> GSErrCode {
        for (const auto& data : zonesData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_ZoneID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> polygonData;
            data.Get("polygon", polygonData);

            element.zone.poly.nCoords = polygonData.GetSize();
            element.zone.poly.nSubPolys = 1;
            element.zone.poly.nArcs = 0;

            memo.coords = reinterpret_cast<API_Coord**>(BMAllocateHandle(
                (polygonData.GetSize() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0));
            memo.pends = reinterpret_cast<Int32**>(BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0));
            memo.parcs = reinterpret_cast<API_PolyArc**>(BMAllocateHandle(1 * sizeof(API_PolyArc), ALLOCATE_CLEAR, 0));

            for (UIndex i = 0; i < polygonData.GetSize(); ++i) {
                (*memo.coords)[i] = Get2DCoordinateFromObjectState(polygonData[i]);
            }
            (*memo.coords)[polygonData.GetSize()] = (*memo.coords)[0];
            (*memo.pends)[0] = polygonData.GetSize();

            double zCoordinate = 0.0;
            short floorIndex = -1;
            data.Get("zCoordinate", zCoordinate);
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); element.header.floorInd = floorIndex; }
            else { const auto fi = GetFloorIndexAndOffset(zCoordinate, stories); element.header.floorInd = fi.first; }

            GS::UniString roomName = "Room", roomNumber = "";
            data.Get("roomName", roomName);
            data.Get("roomNumber", roomNumber);

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            zoneGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("zoneGuids", zoneGuids);
    return response;
}

} // namespace AchicadAutomation
