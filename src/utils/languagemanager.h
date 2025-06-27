/**
 * @file languagemanager.h
 * @brief Language Management System for LogReader Application
 * @details Provides centralized language switching and translation management
 *          functionality using Qt's internationalization framework. Supports
 *          dynamic language switching without application restart.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QTranslator>
#include <QApplication>
#include <QString>
#include <QMap>
#include <functional>

/**
 * @class LanguageManager
 * @brief Singleton class for managing application language and translations
 * @details This class provides centralized language management for the LogReader
 *          application. It handles translation file loading, language switching,
 *          and UI refresh callbacks. The class follows the singleton pattern to
 *          ensure consistent language state across the entire application.
 * 
 * Key features:
 * - Dynamic language switching without restart
 * - Automatic translation file discovery and loading
 * - Persistent language preference storage
 * - Callback mechanism for UI refresh notifications
 * 
 * Usage example:
 * @code
 * LanguageManager& manager = LanguageManager::instance();
 * manager.initialize();
 * manager.setLanguage(LanguageManager::English);
 * @endcode
 */
class LanguageManager
{
public:
    /**
     * @enum Language
     * @brief Enumeration of supported languages
     * @details Defines all languages supported by the application.
     *          Each language corresponds to a specific translation file.
     */
    enum Language {
        Chinese,    ///< 中文 - Chinese language (default)
        English     ///< 英文 - English language
    };

    /**
     * @brief Get the singleton instance of LanguageManager
     * @return Reference to the singleton LanguageManager instance
     * @details Thread-safe singleton implementation using static local variable
     */
    static LanguageManager& instance();
    
    /**
     * @brief Initialize the language manager
     * @details Loads user's preferred language from settings and applies it.
     *          This method should be called once during application startup.
     *          If no preference is found, defaults to system language or Chinese.
     */
    void initialize();
    
    /**
     * @brief Get the current active language
     * @return Current language enumeration value
     * @details Returns the currently active language setting
     */
    Language getCurrentLanguage() const;
    
    /**
     * @brief Set the application language
     * @param language Target language to switch to
     * @details Changes the application language and triggers UI refresh.
     *          The new language preference is automatically saved to settings.
     *          Translation files are loaded from multiple search paths.
     */
    void setLanguage(Language language);
    
    /**
     * @brief Set the application language using language code
     * @param languageCode Language code string (e.g., "zh_CN", "en_US")
     * @details Convenient overload that accepts string language codes.
     *          Automatically converts to Language enum before processing.
     */
    void setLanguage(const QString& languageCode);
    
    /**
     * @brief Get map of all supported languages
     * @return QMap with Language enum as key and language code as value
     * @details Returns a complete mapping of supported languages for UI generation
     */
    QMap<Language, QString> getSupportedLanguages() const;
    
    /**
     * @brief Convert Language enum to language code string
     * @param language Language enumeration value
     * @return Language code string (e.g., "zh_CN" for Chinese)
     * @details Used for file naming and settings storage
     */
    QString languageToCode(Language language) const;
    
    /**
     * @brief Convert language code string to Language enum
     * @param code Language code string
     * @return Corresponding Language enumeration value
     * @details Returns Chinese if the code is not recognized
     */
    Language languageFromCode(const QString& code) const;
    
    /**
     * @brief Get user-friendly display name for a language
     * @param language Language enumeration value
     * @return Localized display name for UI presentation
     * @details Returns names like "中文" for Chinese, "English" for English
     */
    QString languageToDisplayName(Language language) const;
    
    /**
     * @brief Set callback function for language change notifications
     * @param callback Function to call when language changes
     * @details The callback is invoked after successful language change to
     *          allow UI components to refresh their displayed text
     */
    void setLanguageChangeCallback(std::function<void(Language)> callback);

private:
    /**
     * @brief Private constructor for singleton pattern
     * @details Initializes internal data structures and sets up default language mappings
     */
    LanguageManager();
    
    /**
     * @brief Private destructor
     * @details Cleans up translator resources
     */
    ~LanguageManager();
    
    // Disable copy construction and assignment
    LanguageManager(const LanguageManager&) = delete;            ///< Deleted copy constructor
    LanguageManager& operator=(const LanguageManager&) = delete; ///< Deleted assignment operator
    
    /**
     * @brief Load translation file for specified language
     * @param language Target language to load
     * @details Searches for translation files in multiple paths:
     *          - Application directory
     *          - Current working directory  
     *          - Qt resource system
     *          Special handling for Chinese (no translation file needed)
     */
    void loadTranslation(Language language);
    
    /**
     * @brief Remove currently loaded translation
     * @details Uninstalls current translator from QApplication and cleans up memory
     */
    void removeCurrentTranslation();
    
    QTranslator* currentTranslator;                              ///< Currently active translator instance
    Language currentLanguage;                                    ///< Current language setting
    QMap<Language, QString> languageCodes;                       ///< Mapping of Language enum to language codes
    QMap<Language, QString> displayNames;                        ///< Mapping of Language enum to display names
    std::function<void(Language)> languageChangeCallback;        ///< Callback function for language change events
};

#endif // LANGUAGEMANAGER_H 