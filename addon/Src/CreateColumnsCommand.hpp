#pragma once

#include "CommandBase.hpp"


namespace AchicadAutomation {

class CreateColumnsCommand : public CommandBase {
protected:
    virtual GS::String GetName() const override;
    virtual GS::UniString GetInputSchema() const override;
    virtual GS::UniString GetOutputSchema() const override;

public:
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters, GS::ProcessControl& processControl) const override;
};

} // namespace AchicadAutomation
