# LogReader Build Instructions

## Build Requirements
- Qt5.12 or higher (supports Qt5/Qt6)
- C++17 compiler
- CMake 3.15+ (optional, can also use qmake)

## Build Methods

### Method 1: Using Qt Creator (Recommended)
1. Open Qt Creator
2. Select "Open Project"
3. Choose `LogReader.pro` file
4. Configure build toolchain
5. Click "Build" button

### Method 2: Using qmake Command Line
```bash
qmake LogReader.pro
make          # Linux/macOS
nmake         # Windows with MSVC
mingw32-make  # Windows with MinGW
```

### Method 3: Using CMake
```bash
cmake . -B build
cmake --build build
```

## Multi-language Support

The project has implemented complete multi-language support:

### Features
- 🌐 Chinese/English interface switching
- ⚡ Instant language switching from toolbar, no restart required
- 💾 Automatic language preference saving
- 🔧 Extensible interface for adding more languages

### Usage
1. Find the "Language:" dropdown in the toolbar
2. Select "中文" or "English"
3. Interface switches language instantly, including all sub-dialogs
4. Language preference is automatically saved and restored on restart

### Architecture Design
- **LanguageManager**: Singleton pattern for language management
- **AppSettings**: Persistent language configuration
- **Translation Files**: Complete UI element translation support
- **Dynamic Refresh**: retranslateUI for instant interface updates

## Adding New Language Support

1. Create new translation file: `translations/translation_xx.ts`
2. Add language enum and mapping in LanguageManager
3. Update translation file list in build files
4. Use Qt Linguist tool to translate text

## Troubleshooting

### Build Issues
- Ensure Qt environment is properly installed and PATH configured
- Check Qt version is 5.12+
- Using Qt Creator avoids most environment issues

### Language Switching Issues
- Check if translation files are properly generated (.qm files)
- Verify translation file paths are correct
- Check console debug output

### Translation File Compilation
- Both qmake and CMake are configured to auto-compile .ts to .qm
- Build process automatically copies .qm files to output directory
- For manual compilation: `lrelease translations/*.ts` 