param(
    [string]$SdkPath = "",
    [string]$ArmGccBin = "",
    [string]$NinjaPath = "",
    [string]$BuildDir = "build\windows"
)

$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")

function Resolve-FirstPath {
    param([string[]]$Candidates)

    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    return ""
}

function Find-ArmGccBin {
    param([string]$Requested)

    $fromCommand = Get-Command arm-none-eabi-gcc.exe -ErrorAction SilentlyContinue
    $candidates = @($Requested, $env:ARM_GCC_TOOLCHAIN_BIN)
    if ($fromCommand) {
        $candidates += Split-Path -Parent $fromCommand.Source
    }

    $bundleRoot = Join-Path $env:LOCALAPPDATA "stm32cube\bundles\gnu-tools-for-stm32"
    if (Test-Path -LiteralPath $bundleRoot) {
        $candidates += Get-ChildItem -LiteralPath $bundleRoot -Directory |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName "bin" }
    }

    $found = Resolve-FirstPath $candidates
    if (-not $found) {
        throw "arm-none-eabi-gcc.exe not found. Pass -ArmGccBin or set ARM_GCC_TOOLCHAIN_BIN."
    }
    return $found
}

function Find-Ninja {
    param([string]$Requested)

    $fromCommand = Get-Command ninja.exe -ErrorAction SilentlyContinue
    $candidates = @($Requested)
    if ($fromCommand) {
        $candidates += $fromCommand.Source
    }

    $bundleRoot = Join-Path $env:LOCALAPPDATA "stm32cube\bundles\ninja"
    if (Test-Path -LiteralPath $bundleRoot) {
        $candidates += Get-ChildItem -LiteralPath $bundleRoot -Directory |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName "bin\ninja.exe" }
    }

    $found = Resolve-FirstPath $candidates
    if (-not $found) {
        throw "ninja.exe not found. Pass -NinjaPath or install Ninja."
    }
    return $found
}

function Find-Sdk {
    param([string]$Requested)

    $candidates = @(
        $Requested,
        $env:MSPM0_SDK_PATH,
        (Join-Path $Root "..\mspm0-sdk"),
        (Join-Path $HOME "SDK\TI\mspm0-sdk")
    )
    $found = Resolve-FirstPath $candidates
    if (-not $found) {
        throw "MSPM0 SDK not found. Pass -SdkPath or set MSPM0_SDK_PATH."
    }
    return $found
}

$Sdk = Find-Sdk $SdkPath
$ArmBin = Find-ArmGccBin $ArmGccBin
$Ninja = Find-Ninja $NinjaPath
$BuildPath = Join-Path $Root $BuildDir
$ToolchainFile = Join-Path $Root "cmake\arm-none-eabi.cmake"

$env:ARM_GCC_TOOLCHAIN_BIN = $ArmBin
$env:MSPM0_SDK_PATH = $Sdk

function Invoke-Step {
    param(
        [string]$Name,
        [scriptblock]$Command
    )

    & $Command
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE."
    }
}

Write-Output "MSPM0 SDK: $Sdk"
Write-Output "ARM GCC: $ArmBin"
Write-Output "Ninja: $Ninja"
Write-Output "Build dir: $BuildPath"

Invoke-Step "CMake configure" {
    cmake -S $Root -B $BuildPath -G Ninja `
        -DCMAKE_MAKE_PROGRAM="$Ninja" `
        -DCMAKE_TOOLCHAIN_FILE="$ToolchainFile" `
        -DMSPM0_SDK_PATH="$Sdk" `
        -DARM_GCC_TOOLCHAIN_BIN="$ArmBin"
}

Invoke-Step "CMake build" {
    cmake --build $BuildPath --parallel
}
