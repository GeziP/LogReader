QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 隐藏控制台窗口 (Windows)
win32 {
    CONFIG += windows
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/ui/logviewer.cpp \
    src/ui/logtablemodel.cpp \
    src/ui/logfilterproxymodel.cpp \
    src/ui/highlightdelegate.cpp \
    src/ui/exportdialog.cpp \
    src/ui/formattemplatedialog.cpp \
    src/core/logexporter.cpp \
    src/core/logloader.cpp \
    src/core/logformattemplate.cpp \
    src/utils/appsettings.cpp \
    src/utils/languagemanager.cpp

HEADERS += \
    src/core/logentry.h \
    src/ui/logviewer.h \
    src/ui/logtablemodel.h \
    src/ui/logfilterproxymodel.h \
    src/ui/highlightdelegate.h \
    src/ui/exportdialog.h \
    src/ui/formattemplatedialog.h \
    src/core/logexporter.h \
    src/core/logloader.h \
    src/core/logformattemplate.h \
    src/utils/appsettings.h \
    src/utils/languagemanager.h

FORMS +=

# Include paths for headers
INCLUDEPATH += src/ui src/core src/utils

# Translation files
TRANSLATIONS += \
    translations/translation_zh_CN.ts \
    translations/translation_en.ts

# Generate .qm files from .ts files during build
isEmpty(QMAKE_LRELEASE) {
    win32: QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else: QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

# Define output directory for .qm files
QMAKE_LRELEASE_FLAGS += -qm translations/

# Create .qm files during build
updateqm.input = TRANSLATIONS
updateqm.output = translations/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Copy translation files to output directory
win32 {
    CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
    CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug
} else {
    DESTDIR = $$OUT_PWD
}

# Ensure translations directory exists in output
translations.path = $$DESTDIR/translations
translations.files = translations/*.qm
translations.CONFIG += no_check_exist
INSTALLS += translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources/resources.qrc
