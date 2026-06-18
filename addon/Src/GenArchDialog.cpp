#include "GenArchDialog.hpp"
#include "GenArchResIDs.hpp"

#include "APIEnvir.h"
#include "ACAPinc.h"

#include <cstdlib>
#include <cstdio>
#include <windows.h>


GenArchDialog::GenArchDialog () :
    DG::ModalDialog (ACAPI_GetOwnResModule (), GENARCH_DIALOG_RESID, ACAPI_GetOwnResModule ()),
    generateButton    (GetReference (), ButtonGenerateId),
    closeButton       (GetReference (), ButtonCloseId),
    ollamaKeyEdit     (GetReference (), TextEditOllamaKeyId),
    deepseekKeyEdit   (GetReference (), TextEditDeepseekKeyId),
    localUrlEdit      (GetReference (), TextEditLocalUrlId),
    saveSettingsButton (GetReference (), ButtonSaveSettingsId),
    descriptionEdit   (GetReference (), TextEditDescriptionId),
    backendPopUp      (GetReference (), PopUpBackendId),
    statusEdit        (GetReference (), TextEditStatusId)
{
    Attach (*this);
    AttachToAllItems (*this);

    wchar_t appData[MAX_PATH];
    GetEnvironmentVariableW (L"APPDATA", appData, MAX_PATH);
    configDir = GS::UniString (appData) + L"\\GenArch";
    configFile = configDir + L"\\config.json";

    CreateDirectoryW (configDir.ToUStr ().Get (), nullptr);

    backendPopUp.InsertItem (1);
    backendPopUp.SetItemText (1, "Ollama Cloud");
    backendPopUp.InsertItem (2);
    backendPopUp.SetItemText (2, "DeepSeek");
    backendPopUp.InsertItem (3);
    backendPopUp.SetItemText (3, "Ollama Local");
    backendPopUp.SelectItem (1);

    LoadConfig ();
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
    } else if (ev.GetSource () == &saveSettingsButton) {
        SaveConfig ();
    }
}


void GenArchDialog::SetStatus (const GS::UniString& text)
{
    statusEdit.SetText (text);
    statusEdit.Redraw ();
}


void GenArchDialog::LoadConfig ()
{
    FILE* f = _wfopen (configFile.ToUStr ().Get (), L"rb");
    if (!f) return;

    wchar_t key[1024], val[1024];
    while (fwscanf (f, L" \"%1023[^\"]\" : \"%1023[^\"]\" ", key, val) == 2) {
        if (wcscmp (key, L"ollama_cloud_api_key") == 0) ollamaKeyEdit.SetText (GS::UniString (val));
        if (wcscmp (key, L"deepseek_api_key") == 0)     deepseekKeyEdit.SetText (GS::UniString (val));
        if (wcscmp (key, L"ollama_local_base_url") == 0) localUrlEdit.SetText (GS::UniString (val));
    }
    fclose (f);
}


void GenArchDialog::SaveConfig ()
{
    GS::UniString ollama  = ollamaKeyEdit.GetText ();
    GS::UniString deep    = deepseekKeyEdit.GetText ();
    GS::UniString local   = localUrlEdit.GetText ();

    FILE* f = _wfopen (configFile.ToUStr (). Get (), L"wb");
    if (!f) {
        SetStatus ("Failed to save config — check permissions.");
        return;
    }

    fwprintf (f, L"{\n");
    fwprintf (f, L"  \"ollama_cloud_api_key\": \"%s\",\n", ollama.ToUStr ().Get ());
    fwprintf (f, L"  \"deepseek_api_key\": \"%s\",\n",     deep.ToUStr ().Get ());
    fwprintf (f, L"  \"ollama_local_base_url\": \"%s\"\n", local.ToUStr ().Get ());
    fwprintf (f, L"}\n");
    fclose (f);

    SetStatus ("Settings saved to %APPDATA%\\GenArch\\config.json");
}


bool GenArchDialog::HasConfig ()
{
    FILE* f = _wfopen (configFile.ToUStr ().Get (), L"rb");
    if (!f) return false;
    fclose (f);
    return true;
}


void GenArchDialog::GenerateBuilding ()
{
    if (!HasConfig ()) {
        SetStatus ("Please save your API configuration first (top section).");
        return;
    }

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
