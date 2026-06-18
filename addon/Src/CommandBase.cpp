#include "CommandBase.hpp"


namespace AchicadAutomation {

GS::String CommandBase::GetNamespace() const
{
    return "AchicadAutomation";
}

GS::Optional<GS::UniString> CommandBase::GetSchemaDefinitions() const
{
    return GS::NoValue;
}

GS::Optional<GS::UniString> CommandBase::GetInputParametersSchema() const
{
    return GetInputSchema();
}

GS::Optional<GS::UniString> CommandBase::GetResponseSchema() const
{
    return GetOutputSchema();
}

API_AddOnCommandExecutionPolicy CommandBase::GetExecutionPolicy() const
{
    return API_AddOnCommandExecutionPolicy::ScheduleForExecutionOnMainThread;
}

bool CommandBase::IsProcessWindowVisible() const
{
    return true;
}

void CommandBase::OnResponseValidationFailed(const GS::ObjectState&) const
{
}

} // namespace AchicadAutomation
