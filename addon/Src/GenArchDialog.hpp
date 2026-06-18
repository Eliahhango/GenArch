#pragma once

#include "DGModule.hpp"


class GenArchDialog : public DG::ModalDialog,
                      public DG::PanelObserver,
                      public DG::ButtonItemObserver,
                      public DG::CompoundItemObserver
{
private:
    enum {
        ButtonGenerateId    = 1,
        ButtonCloseId       = 2,
        EditTextDescriptionId = 4,
        PopUpBackendId      = 6,
        EditTextStatusId    = 8
    };

    DG::Button      generateButton;
    DG::Button      closeButton;
    DG::TextEdit    descriptionEdit;
    DG::PopUp       backendPopUp;
    DG::TextEdit    statusEdit;

    void    GenerateBuilding (void);
    void    SetStatus (const GS::UniString& text);
    bool    RunPythonAndGetJSON (const GS::UniString& description, const GS::UniString& backend, GS::UniString& outJSON);
    bool    CreateElementsFromJSON (const GS::UniString& jsonStr);

    virtual void    ButtonClicked (const DG::ButtonClickEvent& ev) override;

public:
    GenArchDialog ();
    ~GenArchDialog ();
};
