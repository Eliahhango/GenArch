#pragma once

#include "APIEnvir.h"
#include "ACAPinc.h"


namespace AchicadAutomation {

class CommandBase : public API_AddOnCommand {
public:
    virtual GS::String GetNamespace() const override;
    virtual GS::Optional<GS::UniString> GetSchemaDefinitions() const override;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;
    virtual API_AddOnCommandExecutionPolicy GetExecutionPolicy() const override;
    virtual bool IsProcessWindowVisible() const override;
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override = 0;
    virtual void OnResponseValidationFailed(const GS::ObjectState& response) const override;

protected:
    virtual GS::String GetName() const = 0;
    virtual GS::UniString GetInputSchema() const = 0;
    virtual GS::UniString GetOutputSchema() const = 0;
};

} // namespace AchicadAutomation
