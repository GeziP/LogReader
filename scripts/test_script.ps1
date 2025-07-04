# Test Script
Write-Host "====================================================" -ForegroundColor Green
Write-Host "     Test Script - Running" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green

# Test Environment Variables
Write-Host "Current User: $env:USERNAME" -ForegroundColor Yellow
Write-Host "System Directory: $env:SystemRoot" -ForegroundColor Yellow

# Test Admin Privileges
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if ($isAdmin) {
    Write-Host "Running as Administrator" -ForegroundColor Green
} else {
    Write-Host "Not running as Administrator" -ForegroundColor Red
}

# Test File Operations
$testDir = ".\test_dir"
if (-not (Test-Path $testDir)) {
    Write-Host "Creating test directory..." -ForegroundColor Yellow
    New-Item -Path $testDir -ItemType Directory -Force | Out-Null
    Write-Host "Test directory created" -ForegroundColor Green
} else {
    Write-Host "Test directory already exists" -ForegroundColor Yellow
}

# Test Complete
Write-Host "====================================================" -ForegroundColor Green
Write-Host "     Test Complete" -ForegroundColor Green
Write-Host "====================================================" -ForegroundColor Green
