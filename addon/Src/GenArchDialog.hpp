#pragma once

#include "DGModule.hpp"


class GenArchDialog : public DG::ModalDialog,
                      public DG::PanelObserver,
                      public DG::ButtonItemObserver,
                      public DG::CompoundItemObserver
{
private:
    enum {
        ButtonGenerateId      = 1,
        ButtonCloseId         = 2,
        LeftTextConfigTitleId = 3,
        LeftTextOllamaLabelId = 4,
        TextEditOllamaKeyId   = 5,
        LeftTextDeepseekLabelId = 6,
        TextEditDeepseekKeyId = 7,
        LeftTextLocalUrlLabelId = 8,
        TextEditLocalUrlId    = 9,
        ButtonSaveSettingsId  = 10,
        LeftTextGenTitleId    = 11,
        LeftTextDescLabelId   = 12,
        TextEditDescriptionId = 13,
        LeftTextBackendLabelId = 14,
        PopUpBackendId        = 15,
        LeftTextStatusLabelId = 16,
        TextEditStatusId      = 17,
        LeftTextTipId         = 18
    };

    DG::Button      generateButton;
    DG::Button      closeButton;
    DG::TextEdit    ollamaKeyEdit;
    DG::TextEdit    deepseekKeyEdit;
    DG::TextEdit    localUrlEdit;
    DG::Button      saveSettingsButton;
    DG::TextEdit    descriptionEdit;
    DG::PopUp       backendPopUp;
    DG::TextEdit    statusEdit;

    GS::UniString   configDir;
    GS::UniString   configFile;

    void    LoadConfig ();
    void    SaveConfig ();
    bool    HasConfig ();
    void    GenerateBuilding ();
    void    SetStatus (const GS::UniString& text);

    virtual void    ButtonClicked (const DG::ButtonClickEvent& ev) override;

public:
    GenArchDialog ();
    ~GenArchDialog ();
};
