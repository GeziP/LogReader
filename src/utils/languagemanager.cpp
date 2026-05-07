/**
 * @file languagemanager.cpp
 * @brief Implementation of Language Management System
 * @details Contains the implementation of LanguageManager class methods
 *          for handling application language switching and translation loading.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#include "languagemanager.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLocale>

#include "appsettings.h"

/**
 * @brief Get singleton instance using thread-safe static local variable
 * @return Reference to the singleton LanguageManager instance
 */
LanguageManager& LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

/**
 * @brief Constructor - initializes language mappings and default settings
 * @details Sets up language code mappings and display names for supported
 * languages. Default language is set to Chinese.
 */
LanguageManager::LanguageManager()
    : QObject(nullptr), currentTranslator(nullptr), currentLanguage(Chinese)
{
    // Initialize language code mappings
    languageCodes[Chinese] = "zh_CN";
    languageCodes[English] = "en";

    // Initialize display name mappings
    displayNames[Chinese] = "中文";
    displayNames[English] = "English";
}

/**
 * @brief Destructor - cleans up translation resources
 */
LanguageManager::~LanguageManager()
{
    removeCurrentTranslation();
}

/**
 * @brief Initialize language manager with user preferences
 * @details Loads saved language preference from settings. If no preference
 * exists, determines default language based on system locale and saves it.
 */
void LanguageManager::initialize()
{
    // Read user's preferred language from settings
    QString savedLanguage = AppSettings::instance().getLanguage();
    Language preferredLanguage = languageFromCode(savedLanguage);

    // If no saved language preference, determine default based on system
    // language
    if (savedLanguage.isEmpty()) {
        QLocale systemLocale = QLocale::system();
        if (systemLocale.language() == QLocale::Chinese) {
            preferredLanguage = Chinese;
        } else {
            preferredLanguage = English;
        }
        // Save default selection
        AppSettings::instance().setLanguage(languageToCode(preferredLanguage));
    }

    setLanguage(preferredLanguage);
}

/**
 * @brief Get current active language
 * @return Current language enumeration value
 */
LanguageManager::Language LanguageManager::getCurrentLanguage() const
{
    return currentLanguage;
}

/**
 * @brief Set application language and trigger UI refresh
 * @param language Target language to switch to
 * @details Removes current translation, loads new one, updates internal state,
 *          saves preference to settings, and triggers callback for UI refresh.
 */
void LanguageManager::setLanguage(Language language)
{
    if (language == currentLanguage) {
        return; // No language change needed
    }

    // Remove current translation
    removeCurrentTranslation();

    // Load new translation
    loadTranslation(language);

    // Update current language
    currentLanguage = language;

    // Save to settings
    AppSettings::instance().setLanguage(languageToCode(language));

#ifdef LOG_DEBUG_ENABLED
    qDebug() << "Language changed to:" << languageToDisplayName(language);
#endif

    // Emit signal to notify connected receivers of language change
    emit languageChanged(language);
}

/**
 * @brief Set language using language code string
 * @param languageCode Language code string (e.g., "zh_CN", "en")
 * @details Convenience method that converts string code to enum and calls
 * setLanguage
 */
void LanguageManager::setLanguage(const QString& languageCode)
{
    Language language = languageFromCode(languageCode);
    setLanguage(language);
}

/**
 * @brief Get all supported languages with display names
 * @return QMap with Language enum as key and display name as value
 */
QMap<LanguageManager::Language, QString>
LanguageManager::getSupportedLanguages() const
{
    return displayNames;
}

/**
 * @brief Convert Language enum to language code string
 * @param language Language enumeration value
 * @return Language code string, defaults to "zh_CN" if not found
 */
QString LanguageManager::languageToCode(Language language) const
{
    return languageCodes.value(language, "zh_CN");
}

/**
 * @brief Convert language code string to Language enum
 * @param code Language code string to convert
 * @return Corresponding Language enum, defaults to Chinese if not found
 */
LanguageManager::Language LanguageManager::languageFromCode(
    const QString& code) const
{
    for (auto it = languageCodes.constBegin(); it != languageCodes.constEnd();
         ++it) {
        if (it.value() == code) {
            return it.key();
        }
    }
    return Chinese; // Default to Chinese
}

/**
 * @brief Get user-friendly display name for a language
 * @param language Language enumeration value
 * @return Localized display name, defaults to "中文" if not found
 */
QString LanguageManager::languageToDisplayName(Language language) const
{
    return displayNames.value(language, "中文");
}

/**
 * @brief Load translation file for specified language
 * @param language Target language to load translation for
 * @details Searches for translation files in multiple paths and loads the first
 *          found .qm file. For Chinese language, no translation file is needed.
 *          Provides detailed debug output for troubleshooting.
 */
void LanguageManager::loadTranslation(Language language)
{
    QString languageCode = languageToCode(language);
#ifdef LOG_DEBUG_ENABLED
    qDebug() << "=== Language Change Request ===";
    qDebug() << "Switching to language:" << languageToDisplayName(language)
             << "(" << languageCode << ")";    
#endif

    // For Chinese, no translation file needed (keep original text)
    if (language == Chinese) {
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Using Chinese (default language, no translation needed)";
#endif
        return;
    }

    // Create new translator instance
    currentTranslator = new QTranslator(nullptr);

    // Build translation file path
    QString appDir = QApplication::applicationDirPath();
#ifdef LOG_DEBUG_ENABLED
    qDebug() << "Application directory:" << appDir;
#endif

    bool loaded = false;
    QString qmFileName = QString("translation_%1").arg(languageCode);
#ifdef LOG_DEBUG_ENABLED
    qDebug() << "Looking for translation file:" << qmFileName;
#endif

    // Load translation file from file system
    QStringList searchPaths = {
        appDir + "/translations",    // Production deployment path
        appDir + "/../translations", // Development build path (one level up)
        appDir +
            "/../../translations",   // Development source path (two levels up)
        "./translations",            // Current directory relative path
        "."                          // Current directory
    };

    for (const QString& path : searchPaths) {
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Trying path:" << path;
#endif
        QString fullQmPath = path + "/" + qmFileName + ".qm";
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Full QM path:" << fullQmPath;
        qDebug() << "QM file exists:" << QFile::exists(fullQmPath);
#endif

        if (currentTranslator->load(qmFileName, path)) {
            loaded = true;
#ifdef LOG_DEBUG_ENABLED
            qDebug() << "✓ Translation loaded successfully from:" << path;
#endif
            break;
        } else {
#ifdef LOG_DEBUG_ENABLED
            qDebug() << "✗ Failed to load from:" << path;
#endif
        }
    }

    if (loaded) {
        // Install translator to QApplication
        bool installed = QApplication::installTranslator(currentTranslator);
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "Translation installed:"
                 << (installed ? "SUCCESS" : "FAILED");
#endif
    } else {
#ifdef LOG_DEBUG_ENABLED
        qDebug() << "ERROR: Failed to load translation for:" << languageCode;
        qDebug() << "Make sure translation files are compiled to .qm format";
#endif

        // Keep translator object even if loading failed to avoid null pointer
        // This still allows UI update to be triggered
    }

#ifdef LOG_DEBUG_ENABLED
    qDebug() << "=== End Language Change ===";
#endif
}

/**
 * @brief Remove currently loaded translation from QApplication
 * @details Uninstalls current translator and frees memory. Safe to call
 *          multiple times or when no translator is loaded.
 */
void LanguageManager::removeCurrentTranslation()
{
    if (currentTranslator) {
        QApplication::removeTranslator(currentTranslator);
        delete currentTranslator; // Use delete since no longer inheriting
                                  // QObject
        currentTranslator = nullptr;
    }
}