$BuildDir = "build-win"

Write-Host "Configuring..." -ForegroundColor Cyan
cmake -S . -B $BuildDir -G "Visual Studio 17 2022"
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed"; exit 1 }

Write-Host "Building..." -ForegroundColor Cyan
cmake --build $BuildDir --config Debug
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit 1 }

Write-Host "Launching SpenceDew..." -ForegroundColor Green
& ".\$BuildDir\Debug\SpenceDew.exe"
