param([string]$Toolchain = $env:STM32_TOOLCHAIN)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
if ($Toolchain) { $compiler = Join-Path $Toolchain 'arm-none-eabi-gcc.exe' }
else {
    $found = Get-Command arm-none-eabi-gcc -ErrorAction SilentlyContinue
    if ($found) { $compiler = $found.Source }
    else {
        $compiler = Get-ChildItem 'C:\ST' -Filter arm-none-eabi-gcc.exe -Recurse -ErrorAction SilentlyContinue |
            Select-Object -First 1 -ExpandProperty FullName
    }
}
if (!$compiler -or !(Test-Path -LiteralPath $compiler)) {
    throw 'STM32 compiler not found. Install STM32CubeIDE or set STM32_TOOLCHAIN to its tools/bin directory.'
}
$buildDir = Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$includes = @('Core/Inc', 'Drivers/STM32F4xx_HAL_Driver/Inc',
    'Drivers/STM32F4xx_HAL_Driver/Inc/Legacy', 'Drivers/CMSIS/Device/ST/STM32F4xx/Include', 'Drivers/CMSIS/Include')
$arguments = @('-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard',
    '-std=c11', '-Og', '-g3', '-Wall', '-Wextra', '-ffunction-sections', '-fdata-sections',
    '-DUSE_HAL_DRIVER', '-DSTM32F446xx', '--specs=nano.specs', '--specs=nosys.specs')
foreach ($include in $includes) { $arguments += '-I' + (Join-Path $projectRoot $include) }
$sources = @(Get-ChildItem (Join-Path $projectRoot 'Core/Src') -Filter '*.c' | Select-Object -ExpandProperty FullName)
$sources += @(Get-ChildItem (Join-Path $projectRoot 'Drivers/STM32F4xx_HAL_Driver/Src') -Filter '*.c' | Select-Object -ExpandProperty FullName)
$sources += Join-Path $projectRoot 'Core/Startup/startup_stm32f446retx.s'
$elf = Join-Path $buildDir 'reflex-lab.elf'
$arguments += $sources
$arguments += @('-T', (Join-Path $projectRoot 'STM32F446RETX_FLASH.ld'),
    '-Wl,--gc-sections', ('-Wl,-Map=' + (Join-Path $buildDir 'reflex-lab.map')),
    '-Wl,--start-group', '-lc', '-lm', '-Wl,--end-group', '-o', $elf)
Write-Host 'Building STM32 Reflex Lab...'
& $compiler @arguments
if ($LASTEXITCODE -ne 0) { throw 'Firmware build failed.' }
$compilerDir = Split-Path $compiler -Parent
& (Join-Path $compilerDir 'arm-none-eabi-size.exe') $elf
if ($LASTEXITCODE -ne 0) { throw 'Size check failed.' }
& (Join-Path $compilerDir 'arm-none-eabi-objcopy.exe') -O binary $elf (Join-Path $buildDir 'reflex-lab.bin')
if ($LASTEXITCODE -ne 0) { throw 'Binary generation failed.' }
Write-Host "Build OK: $elf"
