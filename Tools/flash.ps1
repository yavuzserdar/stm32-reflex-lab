param([string]$Programmer = $env:STM32_PROGRAMMER)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1')
if (!$Programmer) {
    $found = Get-Command STM32_Programmer_CLI -ErrorAction SilentlyContinue
    if ($found) { $Programmer = $found.Source }
    else {
        $Programmer = Get-ChildItem 'C:\ST' -Filter STM32_Programmer_CLI.exe -Recurse -ErrorAction SilentlyContinue |
            Select-Object -First 1 -ExpandProperty FullName
    }
}
if (!$Programmer -or !(Test-Path -LiteralPath $Programmer)) {
    throw 'STM32CubeProgrammer not found. Set STM32_PROGRAMMER to STM32_Programmer_CLI.exe.'
}
$elf = Join-Path (Split-Path $PSScriptRoot -Parent) 'build/reflex-lab.elf'
& $Programmer -c port=SWD -w $elf -v -rst
if ($LASTEXITCODE -ne 0) { throw 'Flash failed. Check the ST-LINK USB data cable and close other debug sessions.' }
