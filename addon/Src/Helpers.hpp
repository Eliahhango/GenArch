#pragma once

#include "APIEnvir.h"
#include "ACAPinc.h"

#include "ObjectState.hpp"

#include "APIdefs_Elements.h"
#include "APIdefs_Goodies.h"
#include "APIdefs_Attributes.h"


namespace AchicadAutomation {

API_Coord Get2DCoordinateFromObjectState(const GS::ObjectState& os);
GS::UniString GetGuidStringFromObjectState(const GS::ObjectState& os);
API_Guid GetGuidFromObjectState(const GS::ObjectState& os);

using StoryInfo = GS::Pair<short, double>;
using Stories = GS::Array<StoryInfo>;
Stories GetStories(void);
GS::Pair<short, double> GetFloorIndexAndOffset(double zPos, const Stories& stories);

bool DoesWallExist(const API_Guid& wallGuid);

GSErrCode GetElementDefaults(API_Element& element, API_ElementMemo& memo, API_ElemTypeID typeID);
GSErrCode GetElementDefaultsExt(API_Element& element, API_ElementMemo& memo, API_SubElement& marker, API_ElemTypeID typeID);

API_AttributeIndex FindAttributeIndex(API_AttrTypeID typeID, const GS::UniString& name);

} // namespace AchicadAutomation
