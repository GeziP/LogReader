/**
 * @file appsettings.cpp
 * @brief Implementation of Application Settings Management System
 * @details Contains the implementation of AppSettings class methods for
 *          handling persistent storage of application configuration data.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#include "appsettings.h"

#include <QDir>
#include <QStandardPaths>

// 配置键名定义
const QString AppSettings::KEY_RECENT_LOG_DIR = "paths/recentLogDir";
const QString AppSettings::KEY_RECENT_EXPORT_DIR = "paths/recentExportDir";
const QString AppSettings::KEY_LAST_EXPORT_FORMATS = "export/lastFormats";
const QString AppSettings::KEY_INCLUDE_TIMESTAMP = "export/includeTimestamp";
const QString AppSettings::KEY_INCLUDE_LEVEL = "export/includeLevel";
const QString AppSettings::KEY_INCLUDE_MODULE = "export/includeModule";
const QString AppSettings::KEY_INCLUDE_CONTENT = "export/includeContent";
const QString AppSettings::KEY_LANGUAGE = "ui/language";
const QString AppSettings::KEY_LOG_FORMAT_TEMPLATE = "log/formatTemplate";
const QString AppSettings::KEY_FORMAT_MODE = "log/formatMode";
const QString AppSettings::KEY_HIDE_UNMATCHED = "log/hideUnmatched";

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings()
{
    settings = std::make_unique<QSettings>("LogViewer", "LogViewer");
}

AppSettings::~AppSettings() = default;

void AppSettings::setRecentLogDir(const QString& path)
{
    if (!path.isEmpty()) {
        QDir dir(path);
        if (dir.exists()) {
            settings->setValue(KEY_RECENT_LOG_DIR, dir.absolutePath());
        }
    }
}

QString AppSettings::getRecentLogDir() const
{
    QString defaultPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return settings->value(KEY_RECENT_LOG_DIR, defaultPath).toString();
}

void AppSettings::setRecentExportDir(const QString& path)
{
    if (!path.isEmpty()) {
        QDir dir(path);
        if (dir.exists()) {
            settings->setValue(KEY_RECENT_EXPORT_DIR, dir.absolutePath());
        }
    }
}

QString AppSettings::getRecentExportDir() const
{
    QString defaultPath =
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return settings->value(KEY_RECENT_EXPORT_DIR, defaultPath).toString();
}

void AppSettings::setLastExportFormats(const QStringList& formats)
{
    settings->setValue(KEY_LAST_EXPORT_FORMATS, formats);
}

QStringList AppSettings::getLastExportFormats() const
{
    // 默认选择TXT格式
    QStringList defaultFormats;
    defaultFormats << "TXT";
    return settings->value(KEY_LAST_EXPORT_FORMATS, defaultFormats)
        .toStringList();
}

void AppSettings::setLastIncludeFields(bool timestamp, bool level, bool module,
                                       bool content)
{
    settings->setValue(KEY_INCLUDE_TIMESTAMP, timestamp);
    settings->setValue(KEY_INCLUDE_LEVEL, level);
    settings->setValue(KEY_INCLUDE_MODULE, module);
    settings->setValue(KEY_INCLUDE_CONTENT, content);
}

void AppSettings::getLastIncludeFields(bool& timestamp, bool& level,
                                       bool& module, bool& content) const
{
    // 默认全部包含
    timestamp = settings->value(KEY_INCLUDE_TIMESTAMP, true).toBool();
    level = settings->value(KEY_INCLUDE_LEVEL, true).toBool();
    module = settings->value(KEY_INCLUDE_MODULE, true).toBool();
    content = settings->value(KEY_INCLUDE_CONTENT, true).toBool();
}

/**
 * @brief Set application language preference
 * @param language Language code string to save
 * @details Saves the language preference to persistent storage using QSettings.
 *          The preference will be loaded on next application startup.
 */
void AppSettings::setLanguage(const QString& language)
{
    if (!language.isEmpty()) {
        settings->setValue(KEY_LANGUAGE, language);
    }
}

/**
 * @brief Get saved language preference
 * @return Language code string, defaults to "zh_CN" if not set
 * @details Retrieves language preference from persistent storage.
 *          Returns Chinese ("zh_CN") as the default language if no
 *          preference has been previously saved.
 */
QString AppSettings::getLanguage() const
{
    // 默认返回中文
    return settings->value(KEY_LANGUAGE, "zh_CN").toString();
}

void AppSettings::setLogFormatTemplate(const QString& formatTemplate)
{
    settings->setValue(KEY_LOG_FORMAT_TEMPLATE, formatTemplate);
}

QString AppSettings::getLogFormatTemplate() const
{
    // 空字符串表示自动识别
    return settings->value(KEY_LOG_FORMAT_TEMPLATE, QString()).toString();
}

void AppSettings::setFormatMode(int mode)
{
    settings->setValue(KEY_FORMAT_MODE, mode);
}

int AppSettings::getFormatMode() const
{
    // 0 = 自动识别 (默认)
    return settings->value(KEY_FORMAT_MODE, 0).toInt();
}

void AppSettings::setHideUnmatched(bool hide)
{
    settings->setValue(KEY_HIDE_UNMATCHED, hide);
}

bool AppSettings::getHideUnmatched() const
{
    // 默认显示所有行（不隐藏）
    return settings->value(KEY_HIDE_UNMATCHED, false).toBool();
}