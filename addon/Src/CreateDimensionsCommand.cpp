#include "CreateDimensionsCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"


namespace AchicadAutomation {

GS::String CreateDimensionsCommand::GetName() const { return "CreateDimensions"; }

GS::UniString CreateDimensionsCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "dimensionsData": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "points": { "type": "array", "items": { "type": "object", "properties": { "x": {"type":"number"}, "y": {"type":"number"} }, "required": ["x","y"] } },
          "offset": { "type": "number", "default": 500 },
          "floorIndex": { "type": "integer", "default": 0 }
        },
        "required": ["points"]
      }
    }
  },
  "required": ["dimensionsData"]
})";
}

GS::UniString CreateDimensionsCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"dimensionGuids":{"type":"array","items":{"type":"object","properties":{"guid":{"type":"string"}}}}}})";
}

GS::ObjectState CreateDimensionsCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> dimsData;
    parameters.Get("dimensionsData", dimsData);
    GS::Array<GS::ObjectState> dimGuids;

    ACAPI_CallUndoableCommand("Create Dimensions", [&]() -> GSErrCode {
        for (const auto& data : dimsData) {
            API_Element element = {};
            API_ElementMemo memo = {};
            GSErrCode err2 = GetElementDefaults(element, memo, API_DimensionID);
            if (err2 != NoError) return err2;

            GS::Array<GS::ObjectState> pointsData;
            data.Get("points", pointsData);

            element.dimension.dimAppear = APIApp_Normal;
            element.dimension.textPos = APIPos_Above;
            element.dimension.textWay = APIDir_Parallel;
            element.dimension.defStaticDim = true;
            element.dimension.usedIn3D = false;
            element.dimension.horizontalText = false;

            if (pointsData.GetSize() >= 2) {
                element.dimension.refC = Get2DCoordinateFromObjectState(pointsData[0]);
                API_Coord endC = Get2DCoordinateFromObjectState(pointsData[pointsData.GetSize() - 1]);
                element.dimension.direction.x = endC.x - element.dimension.refC.x;
                element.dimension.direction.y = endC.y - element.dimension.refC.y;
            }

            element.dimension.nDimElem = pointsData.GetSize();

            memo.dimElems = reinterpret_cast<API_DimElem**>(BMAllocateHandle(
                pointsData.GetSize() * sizeof(API_DimElem), ALLOCATE_CLEAR, 0));
            for (UIndex i = 0; i < pointsData.GetSize(); ++i) {
                API_DimElem& dimElem = (*memo.dimElems)[i];
                API_Coord pt = Get2DCoordinateFromObjectState(pointsData[i]);
                dimElem.base.loc = pt;
                dimElem.note = element.dimension.defNote;
                dimElem.witnessVal = element.dimension.defWitnessVal;
                dimElem.witnessForm = element.dimension.defWitnessForm;
                dimElem.fixedPos = true;
                dimElem.pos = pt;
            }

            short floorIndex = 0;
            if (data.Contains("floorIndex")) { data.Get("floorIndex", floorIndex); }
            element.header.floorInd = floorIndex;

            err2 = ACAPI_Element_Create(&element, &memo);
            ACAPI_DisposeElemMemoHdls(&memo);
            if (err2 != NoError) return err2;

            GS::ObjectState guidOs;
            guidOs.Add("guid", APIGuidToString(element.header.guid));
            dimGuids.Push(guidOs);
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("dimensionGuids", dimGuids);
    return response;
}

} // namespace AchicadAutomation
