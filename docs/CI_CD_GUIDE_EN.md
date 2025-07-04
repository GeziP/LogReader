# CI/CD Guide for LogReader

This guide explains the Continuous Integration (CI) and Continuous Deployment (CD) processes for the LogReader project.

## Table of Contents

- [Overview](#overview)
- [Local Validation](#local-validation)
- [Pre-commit Hooks](#pre-commit-hooks)
- [GitHub Actions Workflows](#github-actions-workflows)
- [Release Process](#release-process)
- [Troubleshooting](#troubleshooting)
- [Local One-Click CI Check Script](#local-one-click-ci-check-script)

## Overview

The LogReader project uses a comprehensive CI/CD pipeline to ensure code quality and automate the release process:

1. **Local Validation**: Scripts to verify code quality before committing
2. **Pre-commit Hooks**: Automatic checks run before each commit
3. **GitHub Actions**: CI/CD workflows that run on the GitHub infrastructure
4. **Release Automation**: Automated build and release process

## Local Validation

### Local CI Test Script

The project includes a local CI test script (`local_ci_test_ascii.bat`) that performs the same checks as the CI pipeline:

```bash
# Run from project root or scripts directory
.\scripts\local_ci_test_ascii.bat
```

This script checks:

1. **Code Format**: Uses clang-format to verify code style
2. **Translation Files**: Validates translation files (*.ts) and compiles them to .qm files
3. **CMake Configuration**: Ensures the project can be configured with CMake
4. **Static Analysis**: Runs cppcheck to detect potential issues
5. **File Structure**: Verifies essential files and directories exist

### Running the Script

The script can be run from:
- The project root directory: `.\scripts\local_ci_test_ascii.bat`
- The scripts directory: `.\local_ci_test_ascii.bat`

It automatically detects the location and adjusts paths accordingly.

### Environment Setup

For the script to work correctly, ensure your PATH includes:
- D:\DevTools\LLVM\bin (for clang-format)
- D:\DevTools\Cppcheck (for cppcheck)
- Qt bin directory (e.g., E:\software\QT\5.14.2\mingw73_32\bin)

You can add these temporarily in PowerShell:
```powershell
$env:Path += ";D:\DevTools\LLVM\bin;D:\DevTools\Cppcheck;E:\software\QT\5.14.2\mingw73_32\bin"
```

## Pre-commit Hooks

The project uses pre-commit hooks to automatically check code before each commit.

### Installed Hooks

The following hooks are configured in `.pre-commit-config.yaml`:

1. **Basic Checks**:
   - trailing-whitespace: Removes trailing whitespace
   - end-of-file-fixer: Ensures files end with a newline
   - check-yaml: Validates YAML files
   - check-added-large-files: Prevents committing large files

2. **C++ Checks**:
   - clang-format: Formats code according to style rules
   - cppcheck: Performs static analysis

3. **CMake Checks**:
   - cmakelint: Validates CMake files

### Using Pre-commit

Pre-commit runs automatically when you commit changes. If any check fails, the commit is aborted.

To manually run all pre-commit checks:
```bash
pre-commit run --all-files
```

To skip pre-commit hooks (not recommended):
```bash
git commit -m "Your message" --no-verify
```

## GitHub Actions Workflows

The project uses GitHub Actions for CI/CD automation.

### Quality CI Workflow

The `quality.yml` workflow runs on every pull request and push to main/develop branches:

#### Static Analysis Job
- Runs on Ubuntu, Windows, and macOS
- Checks code formatting with clang-format
- Performs static analysis with clang-tidy and cppcheck

#### Sanitizers Job
- Runs on Ubuntu and macOS
- Builds with Address and Undefined Behavior sanitizers
- Runs tests to detect memory issues

#### Coverage Job
- Runs on Ubuntu
- Generates code coverage reports
- Uploads results to Codecov

### Release Workflow

The `release.yml` workflow runs when a tag is pushed or manually triggered:

#### Build Jobs
- Builds for Windows, Linux, and macOS
- Compiles translation files
- Creates platform-specific packages

#### Release Job
- Creates a GitHub release
- Attaches build artifacts
- Generates release notes

## Release Process

To create a new release:

1. **Update Version**:
   - Update version numbers in relevant files
   - Commit the changes

2. **Create Tag**:
   ```bash
   git tag v1.0.0  # Replace with actual version
   git push origin v1.0.0
   ```

3. **Monitor Workflow**:
   - The release workflow will automatically start
   - Check the Actions tab on GitHub for progress

4. **Verify Release**:
   - Once completed, verify the release on the GitHub Releases page
   - Download and test the artifacts

## Troubleshooting

### Common Issues

#### Local CI Test Failures

- **Clang-format Not Found**: Ensure LLVM is installed and in PATH
- **Cppcheck Not Found**: Ensure Cppcheck is installed and in PATH
- **CMake Configuration Fails**: Check Qt installation and PATH
- **Translation File Errors**: Ensure Qt tools (lrelease) are available

#### Pre-commit Hook Failures

- **Hook Installation Fails**: Check Python installation and permissions
- **Clang-format Errors**: Run `clang-format -i <file>` to fix formatting
- **Cppcheck Errors**: Address the reported issues in your code

#### GitHub Actions Failures

- **Build Errors**: Check the workflow logs for specific error messages
- **Test Failures**: Investigate and fix failing tests
- **Deployment Issues**: Verify GitHub permissions and secrets

### Getting Help

If you encounter persistent issues:

1. Check the detailed logs in GitHub Actions
2. Search for similar issues in the project repository
3. Create a new issue with detailed information about the problem

## Local One-Click CI Check Script

The project provides `scripts/auto_format_and_check.bat` for Windows, which performs code formatting, static analysis, translation file check, structure check, and CMake build test in one step, ensuring local and CI consistency.

### Usage
```bat
cd scripts
./auto_format_and_check.bat
```
Or from the project root:
```bat
scripts\auto_format_and_check.bat
```

### Environment Variables & Dependencies
- The script automatically sets QT_ROOT, Qt5_DIR, CMAKE_PREFIX_PATH, and MinGW paths.
- Requires local installation of: Qt (MinGW version), MinGW, CMake, clang-format, cppcheck, lrelease

### Common Issues
- **CMake cannot find Qt/MinGW**: The script sets environment variables, but if errors persist, check your actual install paths.
- **g++ not detected**: Ensure MinGW is installed and the path is correct.
- **Format/Static analysis/Translation check failed**: Fix your source code as prompted by the script output.
- **Build failed**: First confirm you can build in Qt Creator, then use the script to verify.

### Recommended Workflow
1. Develop and debug in Qt Creator first.
2. Run auto_format_and_check.bat before commit to ensure all checks pass.
3. If all checks pass, push/PR and CI will succeed.

## Consistency of Formatting Toolchain (clang-format)

- The project root contains a single `.clang-format` file, enforced by both CI and local scripts.
- CI specifies the clang-format version (e.g., 14/15/16); developers should install the same version locally via package manager.
- Recommended: choco for Windows, apt for Linux, brew for macOS.
- If formatting fails, run `clang-format -i <file>` locally to fix.

## CI Qt Architecture Auto-Adaptation

- CI workflows now specify Qt architecture based on runner platform:
  - Windows: `arch: windows_x64`
  - Linux: `arch: linux_x64`
  - macOS: `arch: clang_64`
- To support new architectures, extend the `arch` parameter in the workflow.
- If you see linker errors about architecture mismatch, check the Qt install step and runner architecture in CI logs.

## Common CI Architecture Troubleshooting

- Linker errors like `undefined symbols for architecture ...` usually mean Qt library and runner architecture mismatch.
- Check the `Install Qt` step's arch parameter matches the runner platform.
- To force a specific platform, adjust the `runs-on` parameter in the workflow. 