#include "SetElementSurfacesCommand.hpp"
#include "Helpers.hpp"

#include "ACAPinc.h"
#include "APIdefs_Elements.h"
#include "APIdefs_Attributes.h"


namespace AchicadAutomation {

GS::String SetElementSurfacesCommand::GetName() const { return "SetElementSurfaces"; }

GS::UniString SetElementSurfacesCommand::GetInputSchema() const {
    return R"({
  "type": "object",
  "properties": {
    "surfaceAssignments": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "elementGuid": { "type": "object", "properties": { "guid": { "type": "string" } } },
          "surfaceName": { "type": "string" },
          "buildingMaterialName": { "type": "string", "default": "" },
          "compositeName": { "type": "string", "default": "" }
        },
        "required": ["surfaceName"]
      }
    }
  },
  "required": ["surfaceAssignments"]
})";
}

GS::UniString SetElementSurfacesCommand::GetOutputSchema() const {
    return R"({"type":"object","properties":{"count":{"type":"integer"}}})";
}

GS::ObjectState SetElementSurfacesCommand::Execute(const GS::ObjectState& parameters, GS::ProcessControl&) const
{
    GS::Array<GS::ObjectState> assignments;
    parameters.Get("surfaceAssignments", assignments);
    int count = 0;

    ACAPI_CallUndoableCommand("Set Element Surfaces", [&]() -> GSErrCode {
        for (const auto& data : assignments) {
            GS::UniString surfaceName, buildingMaterialName, compositeName;
            data.Get("surfaceName", surfaceName);
            data.Get("buildingMaterialName", buildingMaterialName);
            data.Get("compositeName", compositeName);

            API_AttributeIndex materialIdx = FindAttributeIndex(API_MaterialID, surfaceName);
            API_AttributeIndex buildingMatIdx = FindAttributeIndex(API_BuildingMaterialID, buildingMaterialName);
            API_AttributeIndex compositeIdx = FindAttributeIndex(API_CompWallID, compositeName);

            API_Guid elemGuid = APINULLGuid;
            if (data.Contains("elementGuid")) {
                elemGuid = GetGuidFromObjectState(*data.Get("elementGuid"));
            }

            if (elemGuid != APINULLGuid) {
                API_Element element = {};
                element.header.guid = elemGuid;
                GSErrCode err2 = ACAPI_Element_Get(&element);
                if (err2 != NoError) continue;

                API_Element mask = {};
                ACAPI_ELEMENT_MASK_CLEAR(mask);

                if (element.header.type.typeID == API_WallID) {
                    if (buildingMatIdx.IsPositive()) {
                        element.wall.buildingMaterial = buildingMatIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_WallType, buildingMaterial);
                    }
                    if (compositeIdx.IsPositive()) {
                        element.wall.modelElemStructureType = API_CompositeStructure;
                        element.wall.composite = compositeIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_WallType, modelElemStructureType);
                        ACAPI_ELEMENT_MASK_SET(mask, API_WallType, composite);
                    }
                }
                else if (element.header.type.typeID == API_SlabID) {
                    if (materialIdx.IsPositive()) {
                        element.slab.topMat = materialIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_SlabType, topMat);
                    }
                    if (buildingMatIdx.IsPositive()) {
                        element.slab.buildingMaterial = buildingMatIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_SlabType, buildingMaterial);
                    }
                }
                else if (element.header.type.typeID == API_MeshID) {
                    if (materialIdx.IsPositive()) {
                        element.mesh.topMat = materialIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_MeshType, topMat);
                    }
                    if (buildingMatIdx.IsPositive()) {
                        element.mesh.buildingMaterial = buildingMatIdx;
                        ACAPI_ELEMENT_MASK_SET(mask, API_MeshType, buildingMaterial);
                    }
                }

                API_ElementMemo memo = {};
                err2 = ACAPI_Element_Change(&element, &mask, &memo, 0, true);
                if (err2 == NoError) ++count;
            }
        }
        return NoError;
    });

    GS::ObjectState response;
    response.Add("count", count);
    return response;
}

} // namespace AchicadAutomation
