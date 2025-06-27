#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
#include <QStringList>
#include <QSettings>

class AppSettings
{
public:
    static AppSettings& instance();
    
    // 路径记忆功能
    void setRecentLogDir(const QString& path);
    QString getRecentLogDir() const;
    void setRecentExportDir(const QString& path);
    QString getRecentExportDir() const;
    
    // 导出偏好设置
    void setLastExportFormats(const QStringList& formats);
    QStringList getLastExportFormats() const;
    void setLastIncludeFields(bool timestamp, bool level, bool module, bool content);
    void getLastIncludeFields(bool& timestamp, bool& level, bool& module, bool& content) const;
    
private:
    AppSettings();
    ~AppSettings();
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;
    
    QSettings* settings;
    
    // 配置键名
    static const QString KEY_RECENT_LOG_DIR;
    static const QString KEY_RECENT_EXPORT_DIR;
    static const QString KEY_LAST_EXPORT_FORMATS;
    static const QString KEY_INCLUDE_TIMESTAMP;
    static const QString KEY_INCLUDE_LEVEL;
    static const QString KEY_INCLUDE_MODULE;
    static const QString KEY_INCLUDE_CONTENT;
};

#endif // APPSETTINGS_H 