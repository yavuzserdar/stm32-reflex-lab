param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildDir = Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$test = Join-Path $buildDir 'test-reflex.exe'
& $Compiler -std=c11 -Wall -Wextra -Werror ('-I' + (Join-Path $projectRoot 'Core/Inc')) (Join-Path $projectRoot 'Core/Src/reflex.c') (Join-Path $projectRoot 'tests/test_reflex.c') -o $test
if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed. A host GCC compiler is required.' }
& $test
if ($LASTEXITCODE -ne 0) { throw 'Reflex tests failed.' }
