param(
    [Parameter(Mandatory = $true)]
    [string]$AzaharPath,

    [string]$AppPath = "build/browser-3ds/links-3ds-browser.3dsx",

    [switch]$RuntimeSmoke,
    [switch]$NetworkSmoke
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

if ($RuntimeSmoke -and $NetworkSmoke) {
    throw "Choose only one diagnostic mode: -RuntimeSmoke or -NetworkSmoke."
}
if ($RuntimeSmoke) {
    $AppPath = "build/runtime-smoke-3ds/links-3ds-runtime-smoke.3dsx"
}
if ($NetworkSmoke) {
    $AppPath = "build/network-smoke-3ds/links-3ds-network-smoke.3dsx"
}

if (-not [System.IO.Path]::IsPathRooted($AzaharPath)) {
    $AzaharPath = Join-Path $root $AzaharPath
}
if (-not [System.IO.Path]::IsPathRooted($AppPath)) {
    $AppPath = Join-Path $root $AppPath
}

if (-not (Test-Path -LiteralPath $AzaharPath -PathType Leaf)) {
    throw "Azahar executable not found: $AzaharPath"
}
if (-not (Test-Path -LiteralPath $AppPath -PathType Leaf)) {
    throw "3DS application not found: $AppPath"
}

Write-Host "Launching Azahar"
Write-Host "Executable: $AzaharPath"
Write-Host "Application: $AppPath"

Start-Process -FilePath $AzaharPath -ArgumentList @($AppPath) -WorkingDirectory $root -Wait
