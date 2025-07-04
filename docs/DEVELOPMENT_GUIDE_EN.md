# Development Guide for LogReader

This guide provides detailed instructions for setting up the development environment, installing tools, and understanding the CI/CD workflow for the LogReader project.

## Table of Contents

- [Development Environment Setup](#development-environment-setup)
- [Tool Installation](#tool-installation)
- [Development Workflow](#development-workflow)
- [CI/CD Pipeline](#cicd-pipeline)
- [Code Quality Standards](#code-quality-standards)
- [Troubleshooting](#troubleshooting)

## Development Environment Setup

### System Requirements

- **Operating System**: Windows 10+, macOS 10.14+, Linux (Ubuntu 18.04+)
- **Qt Version**: 5.12 or higher (5.14.2 recommended)
- **Compiler**: GCC 7+, Clang 6+, MSVC 2017+
- **Disk Space**: At least 10GB free space

### Required Tools

- **Qt Framework**: UI development
- **CMake**: Build system
- **LLVM/Clang**: Code formatting and static analysis
- **Cppcheck**: Static analysis
- **Python**: Scripts and pre-commit hooks
- **vcpkg**: C++ package manager

## Tool Installation

### Windows (D: Drive Installation)

We recommend installing development tools on the D: drive for better disk space management.

```powershell
# Run PowerShell as Administrator
cd <project_root>
.\scripts\setup_windows_d_ascii.ps1
```

This script will install:
- LLVM/Clang (with clang-format and clang-tidy)
- Cppcheck
- Python with pre-commit
- vcpkg with Microsoft GSL

### Linux

```bash
# Install dependencies
sudo apt update
sudo apt install cmake build-essential clang clang-format clang-tidy cppcheck python3 python3-pip git

# Install pre-commit
pip3 install pre-commit

# Setup pre-commit hooks
cd <project_root>
pre-commit install
```

### macOS

```bash
# Install dependencies using Homebrew
brew update
brew install cmake llvm cppcheck python git

# Add LLVM to PATH
echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# Install pre-commit
pip3 install pre-commit

# Setup pre-commit hooks
cd <project_root>
pre-commit install
```

## Development Workflow

### 1. Issue Creation

Start by creating an issue that describes the feature or bug to be addressed.

### 2. Branch Creation

Create a feature branch from the main branch:

```bash
git checkout main
git pull
git checkout -b feature/your-feature-name
```

### 3. Local Development

Develop your feature or fix the bug on your branch.

### 4. Local Validation

Before committing, run the local CI test script to validate your changes:

```bash
# Windows
.\scripts\local_ci_test_ascii.bat

# Linux/macOS
./scripts/run_cppcheck.sh
```

This script checks:
- Code formatting (clang-format)
- Translation files
- CMake configuration
- Static analysis (cppcheck)
- File structure

### 5. Pre-commit Checks

When committing, pre-commit hooks will automatically run to check:
- Code formatting
- Static analysis
- CMake file correctness
- Basic checks (trailing whitespace, file endings, etc.)

### 6. Pull Request

Create a pull request to the main or develop branch. GitHub Actions will automatically run CI/CD workflows.

## CI/CD Pipeline

### Quality CI Workflow

The `quality.yml` workflow runs on every pull request and push to main/develop branches:

- **Static Analysis**: Runs on Ubuntu, Windows, and macOS
  - clang-format check
  - clang-tidy analysis
  - cppcheck analysis

- **Sanitizers**: Runs on Ubuntu and macOS
  - Address and Undefined Behavior sanitizers

- **Coverage**: Runs on Ubuntu
  - Code coverage analysis
  - Uploads results to Codecov

### Release Workflow

The `release.yml` workflow runs when a tag is pushed or manually triggered:

- **Build Windows**: Creates Windows executable and packages it
- **Build Linux**: Creates Linux executable and packages it
- **Build macOS**: Creates macOS app bundle and packages it
- **Create Release**: Creates a GitHub release with all artifacts

## Code Quality Standards

### Coding Style

- Follow the project's `.clang-format` configuration
- Use C++17 features where appropriate
- Follow Qt coding conventions for Qt-related code

### Documentation

- Document all public API functions and classes
- Add comments for complex algorithms or logic
- Keep the documentation in sync with code changes

### Testing

- Write unit tests for new functionality
- Ensure existing tests pass with your changes
- Aim for high code coverage

## Troubleshooting

### Common Issues

#### CMake Configuration Fails

- Check that Qt is properly installed and in PATH
- Verify that CMAKE_PREFIX_PATH includes Qt directory
- For Windows, try: `-DCMAKE_PREFIX_PATH="E:\software\QT\5.14.2\mingw73_32"`

#### Pre-commit Hooks Fail

- Update pre-commit: `pre-commit autoupdate`
- Check network connectivity for GitHub repositories
- Run with `--no-verify` if needed, but fix issues before PR

#### Build Errors

- Check compiler compatibility with Qt version
- Verify all dependencies are installed
- Clean build directory and rebuild

### Getting Help

If you encounter issues not covered here, please:
1. Check existing GitHub issues
2. Create a new issue with detailed information
3. Contact the maintainers via email 