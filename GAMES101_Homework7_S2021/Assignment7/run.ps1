param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [switch]$CheckScene,
    [ValidateRange(1, 8192)]
    [int]$Size = 784,
    [ValidateRange(1, 8192)]
    [int]$Spp = 16,
    [ValidateRange(0, 256)]
    [int]$Threads = 0,
    [ValidateSet('diffuse', 'microfacet')]
    [string]$Material = 'diffuse',
    [ValidateRange(0.05, 1)]
    [double]$Roughness = 0.4,
    [ValidateRange(0, 1)]
    [double]$Metallic = 0.85,
    [uint32]$Seed = 42,
    [string]$Output = 'binary.ppm'
)

$ErrorActionPreference = 'Stop'
$preset = 'clion-' + $Configuration.ToLowerInvariant()
$toolchainBin = 'D:\games101\toolchains\gcc-11.2\mingw64\bin'
$previousPath = $env:PATH
# MinGW 运行库与编译器使用同一套工具链。
$env:PATH = "$toolchainBin;$previousPath"
Push-Location $PSScriptRoot
try {
    & cmake --preset $preset
    if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
    & cmake --build --preset $preset --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    Push-Location (Join-Path $PSScriptRoot "cmake-build-$($Configuration.ToLowerInvariant())")
    try {
        $runArguments = @('--size', $Size, '--spp', $Spp, '--threads', $Threads,
            '--material', $Material, '--roughness', $Roughness.ToString([cultureinfo]::InvariantCulture),
            '--metallic', $Metallic.ToString([cultureinfo]::InvariantCulture), '--seed', $Seed, '--output', $Output)
        if ($CheckScene) { $runArguments += '--check-scene' }
        & .\RayTracing.exe @runArguments
        if ($LASTEXITCODE -ne 0) { throw 'Run failed. Check the error above and README.md.' }
    } finally { Pop-Location }
} finally {
    Pop-Location
    $env:PATH = $previousPath
}
