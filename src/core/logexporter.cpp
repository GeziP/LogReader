/**
 * @file logexporter.cpp
 * @brief Implementation of Log Export Functionality
 * @details Contains the complete implementation of LogExporter class providing
 *          multi-format log export capabilities with progress tracking and
 * error handling. Supports TXT, CSV, and JSON export formats with proper
 *          character encoding and data escaping.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#include "logexporter.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

/**
 * @brief Constructor for LogExporter
 * @param parent Parent QObject for memory management
 */
LogExporter::LogExporter(QObject* parent) : QObject(parent) {}

/**
 * @brief Export logs to a single format (legacy compatibility method)
 * @param logs List of log entries to export
 * @param config Export configuration
 * @return True if export succeeded, false otherwise
 * @details This method provides backward compatibility for single-format
 * exports. If multiple formats are configured, it delegates to
 * exportMultipleFormats. Creates necessary directories and handles all error
 * cases with appropriate signal emissions for UI feedback.
 */
bool LogExporter::exportLogs(const QList<LogEntry>& logs,
                             const ExportConfig& config)
{
    if (logs.isEmpty()) {
        emit exportFinished(false, tr("没有日志数据可以导出"));
        return false;
    }

    // If multiple formats configured, use multi-format export
    if (config.formats.size() > 1) {
        return exportMultipleFormats(logs, config);
    }

    // Single format export (backward compatibility)
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

    // Ensure directory exists
    QDir dir(QFileInfo(filePath).absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            emit exportFinished(false, tr("无法创建导出目录"));
            return false;
        }
    }

    // Create temporary configuration for single format export
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

/**
 * @brief Export logs to multiple formats simultaneously
 * @param logs List of log entries to export
 * @param config Export configuration with multiple formats
 * @return True if all exports succeeded, false if any failed
 * @details Exports log data to all specified formats in sequence, providing
 *          progress updates for each format. Creates all necessary directories
 *          and handles errors gracefully. Emits formatExported signal for each
 *          completed format and exportFinished when all are complete.
 */
bool LogExporter::exportMultipleFormats(const QList<LogEntry>& logs,
                                        const ExportConfig& config)
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

    // Ensure directory exists
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

    // Export each format sequentially
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
            emit exportFinished(false,
                                tr("导出 %1 格式失败")
                                    .arg(ExportConfig::getFormatName(format)));
            return false;
        }

        // Update overall progress across all formats
        currentFormat++;
        int overallProgress = (currentFormat * 100) / totalFormats;
        emit progressChanged(overallProgress);
    }

    // Report successful completion of all formats
    QString message = tr("多格式导出成功！已导出 %1 个文件：\n%2")
                          .arg(exportedFiles.size())
                          .arg(exportedFiles.join("\n"));
    emit exportFinished(true, message);

    return true;
}

/**
 * @brief Export logs to plain text format
 * @param logs List of log entries to export
 * @param config Export configuration specifying field inclusion
 * @param filePath Target file path for export
 * @return True if export succeeded, false otherwise
 * @details Creates a human-readable text file with one log entry per line.
 *          Uses UTF-8 encoding with BOM for Windows compatibility. Includes
 *          progress updates during the export process.
 */
bool LogExporter::exportToTxt(const QList<LogEntry>& logs,
                              const ExportConfig& config,
                              const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }

    // Write UTF-8 BOM to ensure Windows correctly recognizes encoding
    file.write("\xEF\xBB\xBF");

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    // Write each log entry as a formatted line
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QString line = formatLogEntry(entry, config, ExportConfig::TXT);
        out << line << "\n";

        emitProgress(i + 1, logs.size());
    }

    return true;
}

/**
 * @brief Export logs to CSV format
 * @param logs List of log entries to export
 * @param config Export configuration specifying field inclusion
 * @param filePath Target file path for export
 * @return True if export succeeded, false otherwise
 * @details Creates a comma-separated values file with proper CSV escaping.
 *          Includes header row and uses UTF-8 encoding with BOM for Excel
 *          compatibility. Handles special characters and commas correctly.
 */
bool LogExporter::exportToCsv(const QList<LogEntry>& logs,
                              const ExportConfig& config,
                              const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }

    // Write UTF-8 BOM to ensure Windows correctly recognizes encoding
    file.write("\xEF\xBB\xBF");

    QTextStream out(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    out.setCodec("UTF-8");
#else
    out.setEncoding(QStringConverter::Utf8);
#endif

    // Write CSV header row based on included fields
    QStringList headers;
    if (config.includeTimestamp)
        headers << QObject::tr("时间戳");
    if (config.includeLevel)
        headers << QObject::tr("日志等级");
    if (config.includeModule)
        headers << QObject::tr("模块");
    if (config.includeContent)
        headers << QObject::tr("内容");

    out << headers.join(",") << "\n";

    // Write each log entry as a CSV row
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QString line = formatLogEntry(entry, config, ExportConfig::CSV);
        out << line << "\n";

        emitProgress(i + 1, logs.size());
    }

    return true;
}

/**
 * @brief Export logs to JSON format
 * @param logs List of log entries to export
 * @param config Export configuration specifying field inclusion
 * @param filePath Target file path for export
 * @return True if export succeeded, false otherwise
 * @details Creates a structured JSON file with proper JSON escaping.
 *          Uses UTF-8 encoding and creates an array of log entry objects.
 *          Suitable for programmatic processing and data exchange.
 */
bool LogExporter::exportToJson(const QList<LogEntry>& logs,
                               const ExportConfig& config,
                               const QString& filePath)
{
    QJsonArray logArray;

    // Convert each log entry to JSON object
    for (int i = 0; i < logs.size(); ++i) {
        const LogEntry& entry = logs[i];
        QJsonObject logObject;

        // Add fields based on configuration
        if (config.includeTimestamp) {
            logObject["timestamp"] =
                entry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        }
        if (config.includeLevel) {
            logObject["level"] = entry.level;
        }
        if (config.includeModule) {
            logObject["module"] = entry.module;
        }
        if (config.includeContent) {
            logObject["content"] = entry.message;
        }

        logArray.append(logObject);
        emitProgress(i + 1, logs.size());
    }

    // Write JSON document to file
    QJsonDocument doc(logArray);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFinished(false, tr("无法创建文件: %1").arg(filePath));
        return false;
    }

    // Write UTF-8 BOM for consistency
    file.write("\xEF\xBB\xBF");
    file.write(doc.toJson(QJsonDocument::Indented));

    return true;
}

/**
 * @brief Format a single log entry for specified export format
 * @param entry Log entry to format
 * @param config Export configuration
 * @param format Target export format
 * @return Formatted string representation
 * @details Converts a LogEntry to its string representation appropriate for
 *          the specified format, respecting field inclusion settings and
 *          applying proper escaping for CSV format.
 */
QString LogExporter::formatLogEntry(const LogEntry& entry,
                                    const ExportConfig& config,
                                    ExportConfig::Format format)
{
    QStringList fields;

    if (config.includeTimestamp) {
        QString timestamp = entry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        if (format == ExportConfig::CSV) {
            timestamp = escapeForCsv(timestamp);
        }
        fields << timestamp;
    }

    if (config.includeLevel) {
        QString level = entry.level;
        if (format == ExportConfig::CSV) {
            level = escapeForCsv(level);
        }
        fields << level;
    }

    if (config.includeModule) {
        QString module = entry.module;
        if (format == ExportConfig::CSV) {
            module = escapeForCsv(module);
        }
        fields << module;
    }

    if (config.includeContent) {
        QString content = entry.message;
        if (format == ExportConfig::CSV) {
            content = escapeForCsv(content);
        }
        fields << content;
    }

    // Join fields with appropriate separator
    if (format == ExportConfig::CSV) {
        return fields.join(",");
    } else {
        return fields.join(" ");
    }
}

/**
 * @brief Escape special characters for CSV format
 * @param field Text field to escape
 * @return Properly escaped text safe for CSV
 * @details Handles CSV escaping according to RFC 4180 standards:
 *          - Wraps fields containing commas, quotes, or newlines in quotes
 *          - Escapes internal quotes by doubling them
 */
QString LogExporter::escapeForCsv(const QString& field)
{
    QString escaped = field;

    // If field contains comma, quote, or newline, wrap in quotes
    if (escaped.contains(',') || escaped.contains('"') ||
        escaped.contains('\n') || escaped.contains('\r')) {
        // Escape internal quotes by doubling them
        escaped.replace('"', "\"\"");
        // Wrap entire field in quotes
        escaped = "\"" + escaped + "\"";
    }

    return escaped;
}

/**
 * @brief Escape special characters for JSON format
 * @param field Text field to escape
 * @return Properly escaped text safe for JSON
 * @details Handles JSON escaping according to JSON standards:
 *          - Escapes quotes, backslashes, and control characters
 *          - Note: Qt's JSON classes handle this automatically
 */
QString LogExporter::escapeForJson(const QString& field)
{
    // Qt's JSON classes handle escaping automatically
    // This method is provided for completeness and future use
    return field;
}

/**
 * @brief Emit progress signal with calculated percentage
 * @param current Current item being processed (1-based)
 * @param total Total number of items to process
 * @details Helper method that calculates percentage and emits progressChanged
 *          signal for consistent progress reporting across export methods.
 */
void LogExporter::emitProgress(int current, int total)
{
    if (total > 0) {
        int percentage = (current * 100) / total;
        emit progressChanged(percentage);
    }
}

/**
 * @brief Generate complete file path for a specific format
 * @param format Target export format
 * @return Complete file path including directory, base name, and extension
 * @details Combines exportDir, baseFileName, and format-specific extension
 *          to create a complete file path for the specified format.
 */
QString ExportConfig::getFilePathForFormat(Format format) const
{
    QString extension = getFormatExtension(format);
    QString fileName = baseFileName + extension;
    QDir dir(exportDir);
    return dir.absoluteFilePath(fileName);
}

/**
 * @brief Get file extension for a specific format
 * @param format Target format
 * @return File extension string including the dot
 * @details Static utility method returning appropriate file extensions
 *          for each supported export format.
 */
QString ExportConfig::getFormatExtension(Format format)
{
    switch (format) {
    case TXT:
        return ".txt";
    case CSV:
        return ".csv";
    case JSON:
        return ".json";
    default:
        return ".txt";
    }
}

/**
 * @brief Get human-readable name for a format
 * @param format Target format
 * @return Localized format name for display in UI
 * @details Static utility method returning user-friendly format names
 *          suitable for display in user interface elements.
 */
QString ExportConfig::getFormatName(Format format)
{
    switch (format) {
    case TXT:
        return QObject::tr("文本格式");
    case CSV:
        return QObject::tr("CSV格式");
    case JSON:
        return QObject::tr("JSON格式");
    default:
        return QObject::tr("未知格式");
    }
}