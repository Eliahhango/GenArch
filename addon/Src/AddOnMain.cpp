#include "APIEnvir.h"
#include "ACAPinc.h"

#include "CommandBase.hpp"
#include "CreateWallsCommand.hpp"
#include "CreateDoorsCommand.hpp"
#include "CreateWindowsCommand.hpp"
#include "CreateSlabsCommand.hpp"
#include "CreateColumnsCommand.hpp"
#include "CreateBeamsCommand.hpp"
#include "CreateRoofsCommand.hpp"
#include "CreateZonesCommand.hpp"
#include "CreateDimensionsCommand.hpp"
#include "CreateStairsCommand.hpp"
#include "CreateCurtainWallsCommand.hpp"
#include "CreateMeshesCommand.hpp"
#include "CreateRailingsCommand.hpp"
#include "CreateFurnitureCommand.hpp"
#include "SetElementSurfacesCommand.hpp"


API_AddonType CheckEnvironment(API_EnvirParams* envir)
{
    RSGetIndString(&envir->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule());
    RSGetIndString(&envir->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule());
    return APIAddon_Preload;
}


GSErrCode RegisterInterface(void)
{
    return NoError;
}


GSErrCode Initialize(void)
{
    GSErrCode err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateWallsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateDoorsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateWindowsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateSlabsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateColumnsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateBeamsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateRoofsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateZonesCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateDimensionsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateStairsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateCurtainWallsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateMeshesCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateRailingsCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::CreateFurnitureCommand>());
    if (err != NoError) return err;

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler(
        GS::NewOwned<AchicadAutomation::SetElementSurfacesCommand>());
    if (err != NoError) return err;

    return NoError;
}


GSErrCode FreeData(void)
{
    return NoError;
}
