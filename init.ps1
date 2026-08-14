<#
    Windows counterpart of init.sh.

    Scaffolds usr/, copies the starter files, puts ss.cmd in the project root
    and sets up the venv. Like init.sh it is safe to re-run: nothing that
    already exists is overwritten.

    Left out on purpose: telnetlib3, because flashing goes through
    STM32_Programmer_CLI rather than the openocd telnet port. ./ss canflash has
    no windows equivalent either, since socketcan does not exist here.

    Usage, from inside the fse_pb_bsp folder:

        powershell -ExecutionPolicy Bypass -File .\init.ps1
        powershell -ExecutionPolicy Bypass -File .\init.ps1 -SkipLibopencm3
#>

[CmdletBinding()]
param(
    [switch]$SkipLibopencm3,
    [switch]$SkipVenv
)

$ErrorActionPreference = "Stop"

# imgtool needs cryptography/click/intelhex/pyyaml/cbor2 - see
# fse_pb_bsp/tools/imgtool/*.py. cbor2 rather than cbor: boot_record.py prefers
# it and falls back to cbor.
# cantools is for ./ss can_gen, which only parses dbc files - it pulls in
# python-can, which installs fine here even though socketcan does not exist.
$PipPackages = @("cryptography", "click", "intelhex", "pyyaml", "cbor2", "cantools")

$BspDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$Root = Split-Path -Parent $BspDir
$ToolsDir = Join-Path $BspDir "tools"

Write-Host "bsp     : $BspDir"
Write-Host "project : $Root"
Write-Host ""


function Test-LongPaths {
    try {
        $v = Get-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
                              -Name "LongPathsEnabled" -ErrorAction Stop
        return $v.LongPathsEnabled -eq 1
    }
    catch {
        return $false
    }
}

# libopencm3 and pip both build very deep paths under the project root; with a
# long root and MAX_PATH still in force they fail in ways that look unrelated
if ($Root.Length -gt 80 -and -not (Test-LongPaths)) {
    Write-Warning "the project path is $($Root.Length) characters and long path support is off."
    Write-Warning "expect MAX_PATH failures. Either move the project somewhere shorter, or run"
    Write-Warning "as admin: New-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem' ``"
    Write-Warning "         -Name LongPathsEnabled -Value 1 -PropertyType DWORD -Force"
    Write-Host ""
}


function New-Dir($path) {
    if (Test-Path -LiteralPath $path) {
        return
    }

    New-Item -ItemType Directory -Force -Path $path | Out-Null
    Write-Host "created  $path"
}

# cp -n : never clobber
function Copy-IfAbsent($from, $to) {
    if (-not (Test-Path -LiteralPath $from)) {
        Write-Warning "missing starter file: $from"
        return
    }

    if (Test-Path -LiteralPath $to) {
        Write-Host "kept     $to"
        return
    }

    Copy-Item -LiteralPath $from -Destination $to
    Write-Host "copied   $to"
}

function Find-Python {
    $py = Get-Command py -ErrorAction SilentlyContinue
    if ($py) {
        return @($py.Source, "-3")
    }

    $python = Get-Command python -ErrorAction SilentlyContinue
    if ($python) {
        # the store stub exits 9009 and opens the store instead of running
        if ($python.Source -notlike "*WindowsApps*") {
            return @($python.Source)
        }
    }

    throw "no usable python found. install python 3.10+ from python.org (the Microsoft Store stub does not work here)"
}


###############################################################################
Write-Host "--- directories ---"

New-Dir (Join-Path $Root "usr\src")
New-Dir (Join-Path $Root "usr\inc")
New-Dir (Join-Path $Root "usr\simulink")


###############################################################################
Write-Host ""
Write-Host "--- starter files ---"

$TestDir = Join-Path $BspDir "test"
$UsrInc = Join-Path $Root "usr\inc"

Copy-IfAbsent (Join-Path $TestDir "main.c")           (Join-Path $Root "main.c")
Copy-IfAbsent (Join-Path $TestDir "Makefile")         (Join-Path $Root "Makefile")
Copy-IfAbsent (Join-Path $TestDir ".gitignore")       (Join-Path $Root ".gitignore")
Copy-IfAbsent (Join-Path $TestDir "FreeRTOSConfig.h") (Join-Path $UsrInc "FreeRTOSConfig.h")

# note: init.sh copies "test/ss_config" here, which does not exist - the file is
# ss_config.h, so on linux usr/inc/ss_config.h never gets created
Copy-IfAbsent (Join-Path $TestDir "ss_config.h")      (Join-Path $UsrInc "ss_config.h")


###############################################################################
Write-Host ""
Write-Host "--- launcher ---"

# windows has no symlinks without admin rights, so ss.cmd gets copied to where
# the ./ss symlink sits on linux
Copy-IfAbsent (Join-Path $ToolsDir "ss.cmd") (Join-Path $Root "ss.cmd")


###############################################################################
Write-Host ""
Write-Host "--- venv ---"

$VenvPy = Join-Path $Root ".venv\Scripts\python.exe"

if ($SkipVenv) {
    Write-Host "skipped (-SkipVenv)"
}
else {
    if (Test-Path -LiteralPath $VenvPy) {
        Write-Host "kept     $Root\.venv"
    }
    else {
        $python = Find-Python
        Write-Host "creating $Root\.venv with $($python -join ' ')"

        & $python[0] @($python[1..($python.Length - 1)]) -m venv (Join-Path $Root ".venv")
        if ($LASTEXITCODE -ne 0) { throw "venv creation failed" }
    }

    # nice to have, not worth failing the whole init over - it is also the step
    # most likely to trip over MAX_PATH, since pip unpacks very deep temp trees
    & $VenvPy -m pip install --upgrade pip --quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "could not upgrade pip, continuing with the bundled version"
    }

    Write-Host "installing: $($PipPackages -join ', ')"
    & $VenvPy -m pip install --quiet @PipPackages
    if ($LASTEXITCODE -ne 0) { throw "pip install failed" }

    Write-Host "ok"
}


###############################################################################
Write-Host ""
Write-Host "--- libopencm3 ---"

if ($SkipLibopencm3) {
    Write-Host "skipped (-SkipLibopencm3)"
}
else {
    $py = if (Test-Path -LiteralPath $VenvPy) { $VenvPy } else { (Find-Python)[0] }

    # ssw.build_libopencm3 finds CubeCLT, make and sh itself - duplicating that
    # discovery in powershell would guarantee the two drift apart
    Push-Location $ToolsDir
    try {
        & $py -c "import ssw; raise SystemExit(ssw.init_libopencm3())"
        $rc = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }

    if ($rc -ne 0) {
        Write-Host ""
        Write-Warning "libopencm3 was not built. The rest of the scaffolding is in place."
        Write-Warning "Install the missing tool and re-run:  .\init.ps1 -SkipVenv"
    }
}


###############################################################################
Write-Host ""
Write-Host "--- vscode ---"

$py = if (Test-Path -LiteralPath $VenvPy) { $VenvPy } else { (Find-Python)[0] }

# regenerated on every init: the paths carry the CubeCLT version, so a stale
# .vscode is worse than none
Push-Location $ToolsDir
try {
    & $py -c "import ssw; raise SystemExit(ssw.init_vscode())"
    $rc = $LASTEXITCODE
}
finally {
    Pop-Location
}

if ($rc -ne 0) {
    Write-Warning "no .vscode config written - install STM32CubeCLT and re-run"
}


###############################################################################
Write-Host ""
Write-Host "done. from the project root:"
Write-Host "    .\ss.cmd build"
Write-Host "    .\ss.cmd flash"
Write-Host "    .\ss.cmd bootloader --bin_file zephyr.bin"
Write-Host "    .\ss.cmd clean"
Write-Host ""
Write-Host "for F5 debugging in vscode, install the extension:"
Write-Host "    code --install-extension marus25.cortex-debug"
