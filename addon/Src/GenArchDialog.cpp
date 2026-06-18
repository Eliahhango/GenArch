#include "GenArchDialog.hpp"
#include "GenArchResIDs.hpp"

#include "APIEnvir.h"
#include "ACAPinc.h"

#include <cstdlib>
#include <windows.h>


GenArchDialog::GenArchDialog () :
    DG::ModalDialog (ACAPI_GetOwnResModule (), GENARCH_DIALOG_RESID, ACAPI_GetOwnResModule ()),
    generateButton  (GetReference (), ButtonGenerateId),
    closeButton     (GetReference (), ButtonCloseId),
    descriptionEdit (GetReference (), EditTextDescriptionId),
    backendPopUp    (GetReference (), PopUpBackendId),
    statusEdit      (GetReference (), EditTextStatusId)
{
    Attach (*this);
    AttachToAllItems (*this);

    backendPopUp.InsertItem (1);
    backendPopUp.SetItemText (1, "Ollama Cloud");
    backendPopUp.InsertItem (2);
    backendPopUp.SetItemText (2, "DeepSeek");
    backendPopUp.InsertItem (3);
    backendPopUp.SetItemText (3, "Ollama Local");
    backendPopUp.SelectItem (1);

    SetStatus ("Ready. Describe your building and click Generate.");
}


GenArchDialog::~GenArchDialog ()
{
    Detach (*this);
    DetachFromAllItems (*this);
}


void GenArchDialog::ButtonClicked (const DG::ButtonClickEvent& ev)
{
    if (ev.GetSource () == &closeButton) {
        PostCloseRequest (Cancel);
    } else if (ev.GetSource () == &generateButton) {
        GenerateBuilding ();
    }
}


void GenArchDialog::SetStatus (const GS::UniString& text)
{
    statusEdit.SetText (text);
    statusEdit.Redraw ();
}


void GenArchDialog::GenerateBuilding ()
{
    GS::UniString description = descriptionEdit.GetText ();
    if (description.IsEmpty ()) {
        SetStatus ("Please enter a building description first.");
        return;
    }

    GS::UniString backend;
    switch (backendPopUp.GetSelectedItem ()) {
        case 1:  backend = "ollama-cloud"; break;
        case 2:  backend = "deepseek";     break;
        case 3:  backend = "ollama";       break;
        default: backend = "ollama-cloud"; break;
    }

    generateButton.Disable ();
    SetStatus ("Generating... this may take a moment.");

    wchar_t ownPath[MAX_PATH];
    DWORD len = GetModuleFileNameW (nullptr, ownPath, MAX_PATH);
    GS::UniString pythonDir = L"..\\..\\python";
    if (len > 0) {
        wchar_t* lastSlash = wcsrchr (ownPath, L'\\');
        if (lastSlash) {
            *(lastSlash + 1) = L'\0';
            pythonDir = GS::UniString (ownPath) + L"..\\python";
        }
    }

    wchar_t tempPath[MAX_PATH];
    GetTempPathW (MAX_PATH, tempPath);
    wchar_t pidStr[32];
    _itow_s (::GetCurrentProcessId (), pidStr, 32, 10);
    GS::UniString tempFile = GS::UniString (tempPath) + L"genarch_output_" + GS::UniString (pidStr) + L".json";

    GS::UniString escapedDesc = description;
    escapedDesc.ReplaceAll ("\"", "\\\"");

    GS::UniString cmd = GS::UniString::Printf (
        L"cmd /c \"cd /d \"%T\" && python main.py --backend %T --output-json \"%T\" \"%T\" 2>&1\"",
        pythonDir, backend, tempFile, escapedDesc);

    INT exitCode = (INT)::_wsystem (cmd.ToUStr ().Get ());

    if (exitCode == 0) {
        SetStatus ("Generation complete!");
    } else {
        SetStatus ("Generation failed. Check Python and API key config.");
    }

    generateButton.Enable ();
}
