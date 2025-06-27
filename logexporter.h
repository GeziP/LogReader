#ifndef LOGEXPORTER_H
#define LOGEXPORTER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QObject>
#include <QDir>
#include "logentry.h"

struct ExportConfig {
    enum Format { TXT, CSV, JSON };
    QList<Format> formats;  // 支持多格式
    QString baseFileName;   // 基础文件名（不含扩展名）
    QString exportDir;      // 导出目录
    bool includeTimestamp;
    bool includeLevel;
    bool includeModule;
    bool includeContent;
    
    ExportConfig() : includeTimestamp(true), includeLevel(true), 
                    includeModule(true), includeContent(true) {}
                    
    // 便利方法
    bool hasFormat(Format format) const { return formats.contains(format); }
    QString getFilePathForFormat(Format format) const;
    static QString getFormatExtension(Format format);
    static QString getFormatName(Format format);
};

class LogExporter : public QObject
{
    Q_OBJECT
    
public:
    explicit LogExporter(QObject *parent = nullptr);
    
    bool exportLogs(const QList<LogEntry>& logs, const ExportConfig& config);
    bool exportMultipleFormats(const QList<LogEntry>& logs, const ExportConfig& config);
    
signals:
    void progressChanged(int percentage);
    void exportFinished(bool success, const QString& message);
    void formatExported(const QString& format, const QString& filePath);
    
private:
    bool exportToTxt(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath);
    bool exportToCsv(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath);
    bool exportToJson(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath);
    
    QString formatLogEntry(const LogEntry& entry, const ExportConfig& config, ExportConfig::Format format);
    QString escapeForCsv(const QString& field);
    QString escapeForJson(const QString& field);
    
    void emitProgress(int current, int total);
};

#endif // LOGEXPORTER_H 