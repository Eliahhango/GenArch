param(
    [string]$ArchiCADVersion = "29"
)

$AddOnsDir = "$env:APPDATA\GRAPHISOFT\ARCHICAD $ArchiCADVersion\Add-Ons"
$SourceApx = "$PSScriptRoot\dist\AchicadAutomation.apx"

if (!(Test-Path $SourceApx)) {
    Write-Host "Error: $SourceApx not found." -ForegroundColor Red
    Write-Host "Make sure you cloned the full repository." -ForegroundColor Yellow
    exit 1
}

if (!(Test-Path $AddOnsDir)) {
    Write-Host "Archicad $ArchiCADVersion not found at $AddOnsDir" -ForegroundColor Red
    Write-Host "Make sure Archicad $ArchiCADVersion is installed first." -ForegroundColor Yellow
    exit 1
}

Copy-Item -LiteralPath $SourceApx -Destination "$AddOnsDir\AchicadAutomation.apx" -Force
Write-Host "Installed GenArch Add-On for Archicad $ArchiCADVersion" -ForegroundColor Green
Write-Host "Path: $AddOnsDir\AchicadAutomation.apx" -ForegroundColor Gray
Write-Host "Restart Archicad to load the Add-On." -ForegroundColor Yellow
