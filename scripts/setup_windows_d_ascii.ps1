# LogReader Development Environment Setup Script (Windows) - D Drive Installation
# Usage: .\scripts\setup_windows_d_ascii.ps1

# Set error handling
$ErrorActionPreference = "Stop"

# Check for administrator privileges
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "Please run this script as Administrator!" -ForegroundColor Red
    exit 1
}

# Set installation paths
$baseInstallPath = "D:\DevTools"
$llvmPath = "$baseInstallPath\LLVM"
$cppcheckPath = "$baseInstallPath\Cppcheck"
$pythonPath = "$baseInstallPath\Python"
$vcpkgPath = "$baseInstallPath\vcpkg"

# Create base directory
if (-not (Test-Path $baseInstallPath)) {
    Write-Host "Creating base installation directory: $baseInstallPath" -ForegroundColor Yellow
    New-Item -Path $baseInstallPath -ItemType Directory -Force | Out-Null
}

Write-Host "====================================================" -ForegroundColor Green
Write-Host "     LogReader Development Environment Setup - Windows (D Drive)" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
Write-Host "This script will install the following tools to D:\DevTools:" -ForegroundColor Yellow
Write-Host "- LLVM (clang, clang-tidy, clang-format)" -ForegroundColor Yellow
Write-Host "- Cppcheck" -ForegroundColor Yellow
Write-Host "- Python and pre-commit" -ForegroundColor Yellow
Write-Host "- vcpkg (for GSL)" -ForegroundColor Yellow
Write-Host "====================================================" -ForegroundColor Green

# Create temporary directory for downloads
$tempDir = "$baseInstallPath\temp"
if (-not (Test-Path $tempDir)) {
    New-Item -Path $tempDir -ItemType Directory -Force | Out-Null
}

# Download and install LLVM
if (-not (Test-Path $llvmPath) -or -not (Test-Path "$llvmPath\bin\clang.exe")) {
    Write-Host "Creating LLVM directory: $llvmPath" -ForegroundColor Yellow
    if (Test-Path $llvmPath) {
        Remove-Item -Path $llvmPath -Recurse -Force
    }
    New-Item -Path $llvmPath -ItemType Directory -Force | Out-Null
    
    $llvmUrl = "https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe"
    $llvmInstaller = "$tempDir\LLVM-16.0.0-win64.exe"
    
    Write-Host "Downloading LLVM installer..." -ForegroundColor Yellow
    
    try {
        # Use .NET WebClient to download
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($llvmUrl, $llvmInstaller)
        
        Write-Host "Download complete, installing LLVM to $llvmPath..." -ForegroundColor Yellow
        # Use silent install and specify installation path
        Start-Process -FilePath $llvmInstaller -ArgumentList "/S", "/D=$llvmPath" -Wait
        
        # Add to environment variables
        $env:Path = "$llvmPath\bin;" + $env:Path
        [System.Environment]::SetEnvironmentVariable("Path", "$llvmPath\bin;" + [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User), [System.EnvironmentVariableTarget]::User)
        
        Write-Host "LLVM installation complete!" -ForegroundColor Green
    }
    catch {
        Write-Host "LLVM download or installation failed: $_" -ForegroundColor Red
        Write-Host "Please download LLVM manually and install to $llvmPath" -ForegroundColor Yellow
    }
} else {
    Write-Host "LLVM directory already exists, skipping installation" -ForegroundColor Green
}

# Download and install Cppcheck
if (-not (Test-Path $cppcheckPath) -or -not (Test-Path "$cppcheckPath\cppcheck.exe")) {
    Write-Host "Creating Cppcheck directory: $cppcheckPath" -ForegroundColor Yellow
    if (Test-Path $cppcheckPath) {
        Remove-Item -Path $cppcheckPath -Recurse -Force
    }
    New-Item -Path $cppcheckPath -ItemType Directory -Force | Out-Null
    
    $cppcheckUrl = "https://github.com/danmar/cppcheck/releases/download/2.12.0/cppcheck-2.12.0-x64-Setup.msi"
    $cppcheckInstaller = "$tempDir\cppcheck-2.12.0-x64-Setup.msi"
    
    Write-Host "Downloading Cppcheck installer..." -ForegroundColor Yellow
    
    try {
        # Use .NET WebClient to download
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($cppcheckUrl, $cppcheckInstaller)
        
        Write-Host "Download complete, installing Cppcheck..." -ForegroundColor Yellow
        # Use msiexec to install to specified directory
        Start-Process -FilePath "msiexec.exe" -ArgumentList "/i", $cppcheckInstaller, "INSTALLDIR=$cppcheckPath", "/quiet" -Wait
        
        # Add to environment variables
        $env:Path = "$cppcheckPath;" + $env:Path
        [System.Environment]::SetEnvironmentVariable("Path", "$cppcheckPath;" + [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User), [System.EnvironmentVariableTarget]::User)
        
        Write-Host "Cppcheck installation complete!" -ForegroundColor Green
    }
    catch {
        Write-Host "Cppcheck download or installation failed: $_" -ForegroundColor Red
        Write-Host "Please download Cppcheck manually and install to $cppcheckPath" -ForegroundColor Yellow
    }
} else {
    Write-Host "Cppcheck directory already exists, skipping installation" -ForegroundColor Green
}

# Check if Python directory is empty
$pythonEmpty = $false
if (Test-Path $pythonPath) {
    $pythonFiles = Get-ChildItem -Path $pythonPath -Force
    if ($pythonFiles.Count -eq 0) {
        Write-Host "Python directory exists but is empty" -ForegroundColor Yellow
        $pythonEmpty = $true
    }
}

# Download and install Python
if (-not (Test-Path $pythonPath) -or -not (Test-Path "$pythonPath\python.exe") -or $pythonEmpty) {
    Write-Host "Creating Python directory: $pythonPath" -ForegroundColor Yellow
    if (Test-Path $pythonPath) {
        Remove-Item -Path $pythonPath -Recurse -Force
    }
    New-Item -Path $pythonPath -ItemType Directory -Force | Out-Null
    
    $pythonUrl = "https://www.python.org/ftp/python/3.11.5/python-3.11.5-amd64.exe"
    $pythonInstaller = "$tempDir\python-3.11.5-amd64.exe"
    
    Write-Host "Downloading Python installer..." -ForegroundColor Yellow
    
    try {
        # Use .NET WebClient to download
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($pythonUrl, $pythonInstaller)
        
        Write-Host "Download complete, installing Python to $pythonPath..." -ForegroundColor Yellow
        # Use silent install and specify installation path
        Start-Process -FilePath $pythonInstaller -ArgumentList "/quiet", "InstallAllUsers=0", "DefaultJustForMeTargetDir=$pythonPath", "PrependPath=1", "Include_test=0" -Wait
        
        # Add to environment variables
        $env:Path = "$pythonPath;$pythonPath\Scripts;" + $env:Path
        [System.Environment]::SetEnvironmentVariable("Path", "$pythonPath;$pythonPath\Scripts;" + [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User), [System.EnvironmentVariableTarget]::User)
        
        Write-Host "Python installation complete!" -ForegroundColor Green
        
        # Install pre-commit
        Write-Host "Installing pre-commit..." -ForegroundColor Yellow
        Start-Process -FilePath "$pythonPath\python.exe" -ArgumentList "-m", "pip", "install", "pre-commit" -Wait
        Write-Host "pre-commit installation complete!" -ForegroundColor Green
    }
    catch {
        Write-Host "Python download or installation failed: $_" -ForegroundColor Red
        Write-Host "Please download Python manually and install to $pythonPath" -ForegroundColor Yellow
    }
} else {
    Write-Host "Python directory already exists, skipping installation" -ForegroundColor Green
    
    # Ensure pre-commit is installed
    Write-Host "Checking if pre-commit is installed..." -ForegroundColor Yellow
    try {
        $preCommitPath = "$pythonPath\Scripts\pre-commit.exe"
        if (Test-Path $preCommitPath) {
            $preCommitVersion = & "$preCommitPath" --version
            Write-Host "pre-commit is installed: $preCommitVersion" -ForegroundColor Green
        } else {
            Write-Host "pre-commit not found, installing..." -ForegroundColor Yellow
            Start-Process -FilePath "$pythonPath\python.exe" -ArgumentList "-m", "pip", "install", "pre-commit" -Wait
            Write-Host "pre-commit installation complete!" -ForegroundColor Green
        }
    }
    catch {
        Write-Host "Error checking pre-commit: $_" -ForegroundColor Red
        Write-Host "Trying to install pre-commit..." -ForegroundColor Yellow
        try {
            Start-Process -FilePath "$pythonPath\python.exe" -ArgumentList "-m", "pip", "install", "pre-commit" -Wait
            Write-Host "pre-commit installation complete!" -ForegroundColor Green
        }
        catch {
            Write-Host "pre-commit installation failed: $_" -ForegroundColor Red
        }
    }
}

# Check if vcpkg directory is empty
$vcpkgEmpty = $false
if (Test-Path $vcpkgPath) {
    $vcpkgFiles = Get-ChildItem -Path $vcpkgPath -Force
    if ($vcpkgFiles.Count -eq 0) {
        Write-Host "vcpkg directory exists but is empty" -ForegroundColor Yellow
        $vcpkgEmpty = $true
    }
}

# Download and install vcpkg
if (-not (Test-Path $vcpkgPath) -or -not (Test-Path "$vcpkgPath\vcpkg.exe") -or $vcpkgEmpty) {
    Write-Host "Creating vcpkg directory: $vcpkgPath" -ForegroundColor Yellow
    if (Test-Path $vcpkgPath) {
        Remove-Item -Path $vcpkgPath -Recurse -Force
    }
    New-Item -Path $vcpkgPath -ItemType Directory -Force | Out-Null
    
    $vcpkgZipUrl = "https://github.com/microsoft/vcpkg/archive/refs/heads/master.zip"
    $vcpkgZip = "$tempDir\vcpkg-master.zip"
    
    Write-Host "Downloading vcpkg..." -ForegroundColor Yellow
    
    try {
        # Use .NET WebClient to download
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($vcpkgZipUrl, $vcpkgZip)
        
        Write-Host "Download complete, extracting vcpkg..." -ForegroundColor Yellow
        # Extract zip file
        Expand-Archive -Path $vcpkgZip -DestinationPath $tempDir -Force
        
        # Move extracted files to target directory
        Get-ChildItem -Path "$tempDir\vcpkg-master\*" | Move-Item -Destination $vcpkgPath -Force
        
        # Run bootstrap script
        Write-Host "Initializing vcpkg..." -ForegroundColor Yellow
        Push-Location $vcpkgPath
        & ".\bootstrap-vcpkg.bat"
        & ".\vcpkg.exe" integrate install
        
        # Install GSL
        Write-Host "Installing Microsoft GSL..." -ForegroundColor Yellow
        & ".\vcpkg.exe" install ms-gsl:x64-windows
        Pop-Location
        
        # Add to environment variables
        $env:Path = "$vcpkgPath;" + $env:Path
        [System.Environment]::SetEnvironmentVariable("Path", "$vcpkgPath;" + [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::User), [System.EnvironmentVariableTarget]::User)
        
        Write-Host "vcpkg and Microsoft GSL installation complete!" -ForegroundColor Green
    }
    catch {
        Write-Host "vcpkg download or installation failed: $_" -ForegroundColor Red
        Write-Host "Please download vcpkg manually and install to $vcpkgPath" -ForegroundColor Yellow
    }
} else {
    Write-Host "vcpkg directory already exists, skipping installation" -ForegroundColor Green
}

# Set up pre-commit hooks
Write-Host "Setting up pre-commit hooks..." -ForegroundColor Yellow
try {
    # Use installed pre-commit
    if (Test-Path "$pythonPath\Scripts\pre-commit.exe") {
        & "$pythonPath\Scripts\pre-commit.exe" install
        Write-Host "pre-commit hooks setup successful!" -ForegroundColor Green
    } else {
        Write-Host "Could not find pre-commit executable, please make sure pre-commit is properly installed" -ForegroundColor Yellow
    }
}
catch {
    Write-Host "pre-commit hooks setup failed: $_" -ForegroundColor Red
}

# Create initial build directory and generate compile_commands.json
Write-Host "Creating build directory and generating compile_commands.json..." -ForegroundColor Yellow
if (-not (Test-Path "build")) {
    New-Item -Path "build" -ItemType Directory -Force | Out-Null
}
Push-Location build
try {
    # Use vcpkg toolchain
    & cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE="$vcpkgPath\scripts\buildsystems\vcpkg.cmake"
    Write-Host "Build directory creation successful!" -ForegroundColor Green
}
catch {
    Write-Host "Build directory creation failed: $_" -ForegroundColor Red
}
Pop-Location

# Clean up temporary files
if (Test-Path $tempDir) {
    Write-Host "Cleaning up temporary files..." -ForegroundColor Yellow
    Remove-Item -Path $tempDir -Recurse -Force
}

Write-Host "====================================================" -ForegroundColor Green
Write-Host "     LogReader Development Environment Setup Complete!" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
Write-Host "Installation paths:" -ForegroundColor Yellow
Write-Host "- LLVM: $llvmPath" -ForegroundColor Yellow
Write-Host "- Cppcheck: $cppcheckPath" -ForegroundColor Yellow
Write-Host "- Python: $pythonPath" -ForegroundColor Yellow
Write-Host "- vcpkg: $vcpkgPath" -ForegroundColor Yellow
Write-Host "====================================================" -ForegroundColor Green
Write-Host "You can now:" -ForegroundColor Yellow
Write-Host "1. Use clang-format to format code" -ForegroundColor Yellow
Write-Host "2. Use clang-tidy for static analysis" -ForegroundColor Yellow
Write-Host "3. Use cppcheck for static analysis" -ForegroundColor Yellow
Write-Host "4. Use pre-commit to automatically check commits" -ForegroundColor Yellow
Write-Host "5. Use GSL library for development" -ForegroundColor Yellow
Write-Host "====================================================" -ForegroundColor Green 