#include "appsettings.h"
#include <QStandardPaths>
#include <QDir>

// 配置键名定义
const QString AppSettings::KEY_RECENT_LOG_DIR = "paths/recentLogDir";
const QString AppSettings::KEY_RECENT_EXPORT_DIR = "paths/recentExportDir";
const QString AppSettings::KEY_LAST_EXPORT_FORMATS = "export/lastFormats";
const QString AppSettings::KEY_INCLUDE_TIMESTAMP = "export/includeTimestamp";
const QString AppSettings::KEY_INCLUDE_LEVEL = "export/includeLevel";
const QString AppSettings::KEY_INCLUDE_MODULE = "export/includeModule";
const QString AppSettings::KEY_INCLUDE_CONTENT = "export/includeContent";

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings()
{
    settings = new QSettings("LogViewer", "LogViewer");
}

AppSettings::~AppSettings()
{
    delete settings;
}

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
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
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
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
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
    return settings->value(KEY_LAST_EXPORT_FORMATS, defaultFormats).toStringList();
}

void AppSettings::setLastIncludeFields(bool timestamp, bool level, bool module, bool content)
{
    settings->setValue(KEY_INCLUDE_TIMESTAMP, timestamp);
    settings->setValue(KEY_INCLUDE_LEVEL, level);
    settings->setValue(KEY_INCLUDE_MODULE, module);
    settings->setValue(KEY_INCLUDE_CONTENT, content);
}

void AppSettings::getLastIncludeFields(bool& timestamp, bool& level, bool& module, bool& content) const
{
    // 默认全部包含
    timestamp = settings->value(KEY_INCLUDE_TIMESTAMP, true).toBool();
    level = settings->value(KEY_INCLUDE_LEVEL, true).toBool();
    module = settings->value(KEY_INCLUDE_MODULE, true).toBool();
    content = settings->value(KEY_INCLUDE_CONTENT, true).toBool();
} 