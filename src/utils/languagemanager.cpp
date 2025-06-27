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
#include "appsettings.h"
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QLocale>
#include <QFile>

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
 * @details Sets up language code mappings and display names for supported languages.
 *          Default language is set to Chinese.
 */
LanguageManager::LanguageManager()
    : currentTranslator(nullptr), currentLanguage(Chinese)
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
 * @details Loads saved language preference from settings. If no preference exists,
 *          determines default language based on system locale and saves it.
 */
void LanguageManager::initialize()
{
    // Read user's preferred language from settings
    QString savedLanguage = AppSettings::instance().getLanguage();
    Language preferredLanguage = languageFromCode(savedLanguage);
    
    // If no saved language preference, determine default based on system language
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
    
    qDebug() << "Language changed to:" << languageToDisplayName(language);
    
    // Invoke callback function to trigger UI update
    if (languageChangeCallback) {
        languageChangeCallback(language);
    }
}

/**
 * @brief Set language using language code string
 * @param languageCode Language code string (e.g., "zh_CN", "en")
 * @details Convenience method that converts string code to enum and calls setLanguage
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
QMap<LanguageManager::Language, QString> LanguageManager::getSupportedLanguages() const
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
LanguageManager::Language LanguageManager::languageFromCode(const QString& code) const
{
    for (auto it = languageCodes.constBegin(); it != languageCodes.constEnd(); ++it) {
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
    qDebug() << "=== Language Change Request ===";
    qDebug() << "Switching to language:" << languageToDisplayName(language) << "(" << languageCode << ")";
    
    // For Chinese, no translation file needed (keep original text)
    if (language == Chinese) {
        qDebug() << "Using Chinese (default language, no translation needed)";
        return;
    }
    
    // Create new translator instance
    currentTranslator = new QTranslator(nullptr);
    
    // Build translation file path
    QString appDir = QApplication::applicationDirPath();
    qDebug() << "Application directory:" << appDir;
    
    bool loaded = false;
    QString qmFileName = QString("translation_%1").arg(languageCode);
    qDebug() << "Looking for translation file:" << qmFileName;
    
    // Load translation file from file system
    QStringList searchPaths = {
        appDir + "/translations",           // Production deployment path
        appDir + "/../translations",        // Development build path (one level up)
        appDir + "/../../translations",     // Development source path (two levels up)
        "./translations",                   // Current directory relative path
        "."                                // Current directory
    };
    
    for (const QString& path : searchPaths) {
        qDebug() << "Trying path:" << path;
        QString fullQmPath = path + "/" + qmFileName + ".qm";
        qDebug() << "Full QM path:" << fullQmPath;
        qDebug() << "QM file exists:" << QFile::exists(fullQmPath);
        
        if (currentTranslator->load(qmFileName, path)) {
            loaded = true;
            qDebug() << "✓ Translation loaded successfully from:" << path;
            break;
        } else {
            qDebug() << "✗ Failed to load from:" << path;
        }
    }
    
    if (loaded) {
        // Install translator to QApplication
        bool installed = QApplication::installTranslator(currentTranslator);
        qDebug() << "Translation installed:" << (installed ? "SUCCESS" : "FAILED");
    } else {
        qDebug() << "ERROR: Failed to load translation for:" << languageCode;
        qDebug() << "Make sure translation files are compiled to .qm format";
        
        // Keep translator object even if loading failed to avoid null pointer
        // This still allows UI update to be triggered
    }
    
    qDebug() << "=== End Language Change ===";
}

/**
 * @brief Set callback function for language change notifications
 * @param callback Function to be called when language changes
 * @details The callback is invoked after successful language change to notify
 *          UI components that they should refresh their displayed text.
 */
void LanguageManager::setLanguageChangeCallback(std::function<void(Language)> callback)
{
    languageChangeCallback = callback;
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
        delete currentTranslator;  // Use delete since no longer inheriting QObject
        currentTranslator = nullptr;
    }
} 