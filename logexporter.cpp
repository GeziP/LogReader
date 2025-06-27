#include "logexporter.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

LogExporter::LogExporter(QObject *parent) : QObject(parent)
{
}

bool LogExporter::exportLogs(const QList<LogEntry>& logs, const ExportConfig& config)
{
    if (logs.isEmpty()) {
        emit exportFinished(false, tr("没有日志数据可以导出"));
        return false;
    }
    
    // 如果配置了多格式，使用多格式导出
    if (config.formats.size() > 1) {
        return exportMultipleFormats(logs, config);
    }
    
    // 单格式导出（向后兼容）
    if (config.formats.isEmpty()) {
        emit exportFinished(false, tr("未选择导出格式"));
        return false;
    }
    
    ExportConfig::Format format = config.formats.first();
    QString filePath = config.getFilePathForFormat(format);
    
    if (filePath.isEmpty()) {
        emit exportFinished(false, tr("导出文件路径不能为空"));
        return false;
    }
    
    // 确保目录存在
    QDir dir(QFileInfo(filePath).absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit exportFinished(false, tr("无法创建导出目录"));
            return false;
        }
    }
    
    // 创建临时配置用于单格式导出
    ExportConfig singleConfig = config;
    singleConfig.formats.clear();
    singleConfig.formats.append(format);
    
    bool success = false;
    switch (format) {
    case ExportConfig::TXT:
        success = exportToTxt(logs, singleConfig, filePath);
        break;
    case ExportConfig::CSV:
        success = exportToCsv(logs, singleConfig, filePath);
        break;
    case ExportConfig::JSON:
        success = exportToJson(logs, singleConfig, filePath);
        break;
    }
    
    if (success) {
        emit exportFinished(true, tr("导出成功！文件保存到: %1").arg(filePath));
    }
    
    return success;
}

bool LogExporter::exportMultipleFormats(const QList<LogEntry>& logs, const ExportConfig& config)
{
    if (logs.isEmpty()) {
        emit exportFinished(false, tr("没有日志数据可以导出"));
        return false;
    }
    
    if (config.formats.isEmpty()) {
        emit exportFinished(false, tr("未选择导出格式"));
        return false;
    }
    
    if (config.baseFileName.isEmpty() || config.exportDir.isEmpty()) {
        emit exportFinished(false, tr("导出路径配置不完整"));
        return false;
    }
    
    // 确保目录存在
    QDir dir(config.exportDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit exportFinished(false, tr("无法创建导出目录"));
            return false;
        }
    }
    
    QStringList exportedFiles;
    int totalFormats = config.formats.size();
    int currentFormat = 0;
    
    for (ExportConfig::Format format : config.formats) {
        QString filePath = config.getFilePathForFormat(format);
        
        bool success = false;
        switch (format) {
        case ExportConfig::TXT:
            success = exportToTxt(logs, config, filePath);
            break;
        case ExportConfig::CSV:
            success = exportToCsv(logs, config, filePath);
            break;
        case ExportConfig::JSON:
            success = exportToJson(logs, config, filePath);
            break;
        }
        
        if (success) {
            exportedFiles.append(filePath);
            emit formatExported(ExportConfig::getFormatName(format), filePath);
        } else {
            emit exportFinished(false, tr("导出 %1 格式失败").arg(ExportConfig::getFormatName(format)));
            return false;
        }
        
        // 更新总体进度
        currentFormat++;
        int overallProgress = (currentFormat * 100) / totalFormats;
        emit progressChanged(overallProgress);
    }
    
    QString message = tr("多格式导出成功！已导出 %1 个文件：\n%2")
                     .arg(exportedFiles.size())
                     .arg(exportedFiles.join("\n"));
    emit exportFinished(true, message);
    
    return true;
}

bool LogExporter::exportToTxt(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }
    
    // 写入UTF-8 BOM以确保Windows正确识别编码
    file.write("\xEF\xBB\xBF");
    
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QString line = formatLogEntry(entry, config, ExportConfig::TXT);
        out << line << "\n";
        
        emitProgress(i + 1, logs.size());
    }
    
    return true;
}

bool LogExporter::exportToCsv(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }
    
    // 写入UTF-8 BOM以确保Windows正确识别编码
    file.write("\xEF\xBB\xBF");
    
    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif
    
    // 写入CSV头部
    QStringList headers;
    if (config.includeTimestamp) headers << "时间戳";
    if (config.includeLevel) headers << "日志等级";
    if (config.includeModule) headers << "模块";
    if (config.includeContent) headers << "内容";
    
    out << headers.join(",") << "\n";
    
    // 写入数据
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QStringList fields;
        
        if (config.includeTimestamp) 
            fields << escapeForCsv(entry.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        if (config.includeLevel) 
            fields << escapeForCsv(entry.level);
        if (config.includeModule) 
            fields << escapeForCsv(entry.module);
        if (config.includeContent) 
            fields << escapeForCsv(entry.content);
        
        out << fields.join(",") << "\n";
        
        emitProgress(i + 1, logs.size());
    }
    
    return true;
}

bool LogExporter::exportToJson(const QList<LogEntry>& logs, const ExportConfig& config, const QString& filePath)
{
    QJsonArray logArray;
    
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QJsonObject logObject;
        
        if (config.includeTimestamp)
            logObject["timestamp"] = entry.timestamp.toString(Qt::ISODate);
        if (config.includeLevel)
            logObject["level"] = entry.level;
        if (config.includeModule)
            logObject["module"] = entry.module;
        if (config.includeContent)
            logObject["content"] = entry.content;
        
        logArray.append(logObject);
        
        emitProgress(i + 1, logs.size());
    }
    
    QJsonDocument doc(logArray);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }
    
    // 写入UTF-8 BOM以确保Windows正确识别编码
    file.write("\xEF\xBB\xBF");
    
    // 将JSON转换为UTF-8字节数组并写入
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    file.write(jsonData);
    
    return true;
}

QString LogExporter::formatLogEntry(const LogEntry& entry, const ExportConfig& config, ExportConfig::Format format)
{
    QStringList parts;
    
    if (config.includeTimestamp) {
        parts << entry.timestamp.toString("yyyy-MM-dd hh:mm:ss");
    }
    if (config.includeLevel) {
        parts << QString("[%1]").arg(entry.level);
    }
    if (config.includeModule) {
        parts << QString("<%1>").arg(entry.module);
    }
    if (config.includeContent) {
        parts << entry.content;
    }
    
    return parts.join(" ");
}

QString LogExporter::escapeForCsv(const QString& field)
{
    QString escaped = field;
    escaped.replace("\"", "\"\"");
    
    if (escaped.contains(",") || escaped.contains("\"") || escaped.contains("\n")) {
        escaped = "\"" + escaped + "\"";
    }
    
    return escaped;
}

QString LogExporter::escapeForJson(const QString& field)
{
    QString escaped = field;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

void LogExporter::emitProgress(int current, int total)
{
    if (total > 0) {
        int percentage = (current * 100) / total;
        emit progressChanged(percentage);
    }
}

// ExportConfig便利方法的实现
QString ExportConfig::getFilePathForFormat(Format format) const
{
    if (exportDir.isEmpty() || baseFileName.isEmpty()) {
        return QString();
    }
    
    QDir dir(exportDir);
    QString fileName = baseFileName + getFormatExtension(format);
    return dir.filePath(fileName);
}

QString ExportConfig::getFormatExtension(Format format)
{
    switch (format) {
    case TXT: return ".txt";
    case CSV: return ".csv";
    case JSON: return ".json";
    }
    return ".txt";
}

QString ExportConfig::getFormatName(Format format)
{
    switch (format) {
    case TXT: return "TXT";
    case CSV: return "CSV";
    case JSON: return "JSON";
    }
    return "TXT";
} 