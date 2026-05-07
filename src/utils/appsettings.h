/**
 * @file appsettings.h
 * @brief Application Settings Management System
 * @details Provides centralized configuration management for LogReader
 * application using Qt's QSettings framework. Handles persistent storage of
 * user preferences including language settings and window states.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QString>
#include <QStringList>
#include <memory>

/**
 * @class AppSettings
 * @brief Singleton class for managing application configuration settings
 * @details This class provides a centralized interface for reading and writing
 *          application settings using Qt's QSettings framework. It follows the
 *          singleton pattern to ensure consistent access to configuration data
 *          throughout the application.
 *
 * Key features:
 * - Persistent storage of user preferences
 * - Language preference management
 * - Window state and geometry persistence
 * - Thread-safe singleton implementation
 *
 * Settings are automatically saved to the system's standard location:
 * - Windows: Registry (HKEY_CURRENT_USER)
 * - macOS: plist files in ~/Library/Preferences
 * - Linux: INI files in ~/.config
 *
 * Usage example:
 * @code
 * AppSettings& settings = AppSettings::instance();
 * settings.setLanguage("en");
 * QString currentLang = settings.getLanguage();
 * @endcode
 */
class AppSettings
{
public:
    /**
     * @brief Get the singleton instance of AppSettings
     * @return Reference to the singleton AppSettings instance
     * @details Thread-safe singleton implementation using static local variable
     */
    static AppSettings& instance();

    // 路径记忆功能
    void setRecentLogDir(const QString& path);
    QString getRecentLogDir() const;
    void setRecentExportDir(const QString& path);
    QString getRecentExportDir() const;

    // 导出偏好设置
    void setLastExportFormats(const QStringList& formats);
    QStringList getLastExportFormats() const;
    void setLastIncludeFields(bool timestamp, bool level, bool module,
                              bool content);
    void getLastIncludeFields(bool& timestamp, bool& level, bool& module,
                              bool& content) const;

    /**
     * @brief Set the application language preference
     * @param language Language code string (e.g., "zh_CN", "en")
     * @details Saves the language preference to persistent storage.
     *          The setting takes effect on next application start or
     *          when language manager reloads the configuration.
     */
    void setLanguage(const QString& language);

    /**
     * @brief Get the current language preference
     * @return Language code string, defaults to "zh_CN" if not set
     * @details Retrieves the saved language preference from persistent storage.
     *          Returns Chinese as default if no preference has been saved.
     */
    QString getLanguage() const;

private:
    /**
     * @brief Private constructor for singleton pattern
     * @details Initializes QSettings with application-specific organization
     *          and application name for proper settings storage location.
     */
    AppSettings();

    /**
     * @brief Private destructor
     * @details Cleans up QSettings resources
     */
    ~AppSettings();

    // Disable copy construction and assignment
    AppSettings(const AppSettings&) = delete; ///< Deleted copy constructor
    AppSettings& operator=(const AppSettings&) =
        delete;                               ///< Deleted assignment operator

    std::unique_ptr<QSettings> settings; ///< Qt settings object for persistent storage

    // 配置键名
    static const QString KEY_RECENT_LOG_DIR;
    static const QString KEY_RECENT_EXPORT_DIR;
    static const QString KEY_LAST_EXPORT_FORMATS;
    static const QString KEY_INCLUDE_TIMESTAMP;
    static const QString KEY_INCLUDE_LEVEL;
    static const QString KEY_INCLUDE_MODULE;
    static const QString KEY_INCLUDE_CONTENT;
    static const QString
        KEY_LANGUAGE; ///< Configuration key for language preference
};

#endif // APPSETTINGS_H