param(
    [switch]$Silent,
    [string]$ArchiCADVersion = "29"
)

$ErrorActionPreference = "Stop"
$RootDir = $PSScriptRoot
$DevKitDir = "$RootDir\addon\Build\DevKit\APIDevKit-$ArchiCADVersion"
$BuildDir = "$RootDir\addon\Build\AC${ArchiCADVersion}_NMake"
$AddOnsDir = "$env:APPDATA\GRAPHISOFT\ARCHICAD $ArchiCADVersion\Add-Ons"
$DevKitUrl = "https://github.com/GRAPHISOFT/archicad-api-devkit/releases/download/29.3100/API.Development.Kit.WIN.29.3100.zip"
$DevKitZip = "$env:TEMP\APIDevKit-29.zip"
$ToolsUrl = "https://github.com/GRAPHISOFT/archicad-addon-cmake-tools.git"
$ToolsDir = "$RootDir\addon\Tools"

function Step($msg) { Write-Host "`n==> $msg" -ForegroundColor Cyan }

# ----- Prerequisites -----
Step "Checking prerequisites"
$missing = @()
if (!(Get-Command cmake -ErrorAction SilentlyContinue))  { $missing += "CMake (install from https://cmake.org/download/)" }
if (!(Get-Command python -ErrorAction SilentlyContinue))  { $missing += "Python 3.10+ (install from https://python.org)" }
if (!(Get-Command git -ErrorAction SilentlyContinue))    { $missing += "Git (install from https://git-scm.com)" }
if (!(Test-Path "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat")) {
    # Check other common VS paths
    $vsPaths = @(
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    )
    if (!($vsPaths | Where-Object { Test-Path $_ } | Select-Object -First 1)) {
        $missing += "Visual Studio 2022 with 'Desktop development with C++' workload"
    }
}
if ($missing.Count -gt 0) {
    Write-Host "Missing prerequisites:" -ForegroundColor Yellow
    $missing | ForEach-Object { Write-Host "  - $_" }
    if (!$Silent) { Read-Host "`nInstall them first, then press Enter to continue" }
}

# ----- Build Tools -----
if (!(Test-Path "$ToolsDir\CMakeCommon.cmake")) {
    Step "Cloning build tools (archicad-addon-cmake-tools)"
    if (Test-Path $ToolsDir) { Remove-Item -Recurse -Force $ToolsDir }
    git clone --depth 1 $ToolsUrl $ToolsDir
} else {
    Write-Host "  Build tools already present" -ForegroundColor Green
}

# ----- DevKit -----
if (!(Test-Path "$DevKitDir\Support\Inc\APIdefs_Elements.h")) {
    Step "Downloading AC29 API DevKit (~15 MB)"
    Invoke-WebRequest -Uri $DevKitUrl -OutFile $DevKitZip -UseBasicParsing
    Step "Extracting DevKit"
    New-Item -ItemType Directory -Path "$DevKitDir" -Force | Out-Null
    Expand-Archive -LiteralPath $DevKitZip -DestinationPath "$DevKitDir" -Force
    Remove-Item $DevKitZip -Force
} else {
    Write-Host "  DevKit already present" -ForegroundColor Green
}

# ----- Build -----
Step "Building Add-On (cmake + nmake)"
$vcvars = Get-ChildItem "$env:ProgramFiles\Microsoft Visual Studio\2022\*\VC\Auxiliary\Build\vcvars64.bat" | Select-Object -First 1 -ExpandProperty FullName
& cmd /c """$vcvars""" '&&' "set PATH=C:\Program Files\CMake\bin;%PATH%" '&&' "cd /d $RootDir\addon" '&&' "cmake -B ""$BuildDir"" -G ""NMake Makefiles"" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DAC_API_DEVKIT_DIR=""$DevKitDir\Support"" -DAC_VERSION=$ArchiCADVersion -S ""$RootDir\addon""" '&&' "cmake --build ""$BuildDir"" --config RelWithDebInfo"
if ($LASTEXITCODE -ne 0) { Write-Host "Build failed!" -ForegroundColor Red; exit 1 }

# ----- Install -----
Step "Installing Add-On to Archicad $ArchiCADVersion"
if (!(Test-Path $AddOnsDir)) { New-Item -ItemType Directory -Path $AddOnsDir -Force | Out-Null }
Copy-Item -LiteralPath "$BuildDir\AchicadAutomation.apx" -Destination "$AddOnsDir\" -Force
Write-Host "  Copied to $AddOnsDir\AchicadAutomation.apx" -ForegroundColor Green

# ----- Python deps -----
Step "Installing Python dependencies"
pip install -r "$RootDir\python\requirements.txt" 2>&1 | Out-Null

Write-Host "`n===== Done =====" -ForegroundColor Green
Write-Host "1. Restart Archicad $ArchiCADVersion" -ForegroundColor Yellow
Write-Host "2. cd python && python main.py ""1-bedroom cabin""" -ForegroundColor Yellow
