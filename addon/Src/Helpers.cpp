#include "Helpers.hpp"


namespace AchicadAutomation {

API_Coord Get2DCoordinateFromObjectState(const GS::ObjectState& os)
{
    API_Coord coord = {};
    os.Get("x", coord.x);
    os.Get("y", coord.y);
    return coord;
}

GS::UniString GetGuidStringFromObjectState(const GS::ObjectState& os)
{
    GS::UniString guidStr;
    os.Get("guid", guidStr);
    return guidStr;
}

API_Guid GetGuidFromObjectState(const GS::ObjectState& os)
{
    GS::UniString guidStr = GetGuidStringFromObjectState(os);
    if (!guidStr.IsEmpty()) {
        return APIGuidFromString(guidStr.ToCStr().Get());
    }
    return APINULLGuid;
}

Stories GetStories(void)
{
    Stories stories;
    API_StoryInfo storyInfo = {};
    GSErrCode err = ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
    if (err == NoError) {
        for (short i = storyInfo.firstStory; i <= storyInfo.lastStory; ++i) {
            API_StoryType* story = storyInfo.data[i];
            stories.Push(StoryInfo(i, story->level));
        }
    }
    return stories;
}

GS::Pair<short, double> GetFloorIndexAndOffset(double zPos, const Stories& stories)
{
    short floorIndex = 0;
    double bottomOffset = zPos;
    for (UIndex i = 0; i < stories.GetSize(); ++i) {
        if (zPos >= stories[i].second) {
            floorIndex = stories[i].first;
            bottomOffset = zPos - stories[i].second;
        }
    }
    return GS::Pair<short, double>(floorIndex, bottomOffset);
}

bool DoesWallExist(const API_Guid& wallGuid)
{
    if (wallGuid == APINULLGuid) return false;
    API_Elem_Head head = {};
    head.guid = wallGuid;
    GSErrCode err = ACAPI_Element_GetHeader(&head);
    return (err == NoError && head.type.typeID == API_WallID);
}

GSErrCode GetElementDefaults(API_Element& element, API_ElementMemo& memo, API_ElemTypeID typeID)
{
    BNZeroMemory(&element, sizeof(API_Element));
    BNZeroMemory(&memo, sizeof(API_ElementMemo));
    element.header.type.typeID = typeID;
    return ACAPI_Element_GetDefaults(&element, &memo);
}

GSErrCode GetElementDefaultsExt(API_Element& element, API_ElementMemo& memo, API_SubElement& marker, API_ElemTypeID typeID)
{
    BNZeroMemory(&element, sizeof(API_Element));
    BNZeroMemory(&memo, sizeof(API_ElementMemo));
    BNZeroMemory(&marker, sizeof(API_SubElement));
    element.header.type.typeID = typeID;
    marker.subType = APISubElement_MainMarker;
    return ACAPI_Element_GetDefaultsExt(&element, &memo, 1UL, &marker);
}

API_AttributeIndex FindAttributeIndex(API_AttrTypeID typeID, const GS::UniString& name)
{
    API_AttributeIndex result = {};
    if (name.IsEmpty()) return result;

    API_Attribute attr = {};
    attr.header.typeID = typeID;
    strncpy(attr.header.name, name.ToCStr().Get(), API_AttrNameLen - 1);
    attr.header.name[API_AttrNameLen - 1] = '\0';
    GSErrCode err = ACAPI_Attribute_Get(&attr);
    if (err == NoError) return attr.header.index;
    return result;
}

} // namespace AchicadAutomation
