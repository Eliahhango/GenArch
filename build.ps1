param(
    [string]$ArchiCADVersion = "29"
)

$RootDir = $PSScriptRoot
$DevKitDir = "$RootDir\addon\Build\DevKit\APIDevKit-$ArchiCADVersion"
$BuildDir = "$RootDir\addon\Build\AC${ArchiCADVersion}_NMake"
$DistDir = "$RootDir\dist"
$ToolsDir = "$RootDir\addon\Tools"

# Ensure build tools exist
if (!(Test-Path "$ToolsDir\CMakeCommon.cmake")) {
    Write-Host "Cloning build tools..." -ForegroundColor Cyan
    git clone --depth 1 https://github.com/GRAPHISOFT/archicad-addon-cmake-tools.git $ToolsDir
}

# Ensure DevKit exists
if (!(Test-Path "$DevKitDir\Support\Inc\APIdefs_Elements.h")) {
    Write-Host "Downloading AC29 DevKit..." -ForegroundColor Cyan
    $zip = "$env:TEMP\APIDevKit-29.zip"
    Invoke-WebRequest -Uri "https://github.com/GRAPHISOFT/archicad-api-devkit/releases/download/29.3100/API.Development.Kit.WIN.29.3100.zip" -OutFile $zip -UseBasicParsing
    New-Item -ItemType Directory -Path "$DevKitDir" -Force | Out-Null
    Expand-Archive -LiteralPath $zip -DestinationPath "$DevKitDir" -Force
    Remove-Item $zip -Force
}

# Build
$vcvars = Get-ChildItem "$env:ProgramFiles\Microsoft Visual Studio\2022\*\VC\Auxiliary\Build\vcvars64.bat" | Select-Object -First 1 -ExpandProperty FullName
Write-Host "Building..." -ForegroundColor Cyan
& cmd /c """$vcvars""" '&&' "set PATH=C:\Program Files\CMake\bin;%PATH%" '&&' "cd /d $RootDir\addon" '&&' "cmake -B ""$BuildDir"" -G ""NMake Makefiles"" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DAC_API_DEVKIT_DIR=""$DevKitDir\Support"" -DAC_VERSION=$ArchiCADVersion -S ""$RootDir\addon""" '&&' "cmake --build ""$BuildDir"" --config RelWithDebInfo"

if ($LASTEXITCODE -ne 0) { Write-Host "Build failed!" -ForegroundColor Red; exit 1 }

# Copy to dist
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null
Copy-Item -LiteralPath "$BuildDir\AchicadAutomation.apx" -Destination "$DistDir\AchicadAutomation.apx" -Force
Write-Host "`nUpdated dist/AchicadAutomation.apx ($(Get-Item "$DistDir\AchicadAutomation.apx" | Select-Object -ExpandProperty Length) bytes)" -ForegroundColor Green
Write-Host "Commit and push to distribute: git add dist/ && git commit -m ""Update .apx"" && git push" -ForegroundColor Yellow
