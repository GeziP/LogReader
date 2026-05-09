/**
 * @file logexporter.h
 * @brief Log Export Functionality for Multiple File Formats
 * @details Provides comprehensive log export capabilities supporting TXT, CSV,
 * and JSON formats with configurable field inclusion and batch export
 * functionality. Includes progress tracking and error handling for large
 * dataset exports.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef LOGEXPORTER_H
#define LOGEXPORTER_H

#include <QDir>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "logentry.h"

/**
 * @struct ExportConfig
 * @brief Configuration structure for log export operations
 * @details Defines all parameters needed for exporting log data including
 *          target formats, file paths, field inclusion settings, and naming
 * conventions. Supports simultaneous export to multiple formats with a single
 * configuration.
 *
 * Key features:
 * - Multi-format export support (TXT, CSV, JSON)
 * - Selective field inclusion/exclusion
 * - Flexible file naming and directory specification
 * - Convenience methods for format checking and path generation
 *
 * Usage example:
 * @code
 * ExportConfig config;
 * config.formats = {ExportConfig::TXT, ExportConfig::CSV};
 * config.baseFileName = "filtered_logs";
 * config.exportDir = "/path/to/export";
 * config.includeTimestamp = true;
 * @endcode
 */
struct ExportConfig
{
    /**
     * @enum Format
     * @brief Supported export file formats
     * @details Enumeration of all file formats supported by the export system
     */
    enum Format
    {
        TXT, ///< Plain text format, human-readable
        CSV, ///< Comma-separated values, Excel-compatible
        JSON ///< JavaScript Object Notation, structured data
    };

    QList<Format> formats; ///< List of formats to export simultaneously
    QString baseFileName;  ///< Base filename without extension (extension added
                           ///< automatically)
    QString exportDir;     ///< Target directory for exported files
    bool includeTimestamp; ///< Whether to include timestamp field in export
    bool includeLevel;     ///< Whether to include log level field in export
    bool includeModule;    ///< Whether to include module field in export
    bool includeContent;   ///< Whether to include content field in export

    /**
     * @brief Default constructor with sensible defaults
     * @details Initializes all field inclusion flags to true, allowing complete
     *          log entry export by default. Format list and paths must be set
     * explicitly.
     */
    ExportConfig()
        : includeTimestamp(true),
          includeLevel(true),
          includeModule(true),
          includeContent(true)
    {
    }

    /**
     * @brief Check if a specific format is included in export list
     * @param format Format to check for
     * @return True if format is in the export list, false otherwise
     * @details Convenience method for checking format inclusion without
     *          manually iterating through the formats list.
     */
    bool hasFormat(Format format) const { return formats.contains(format); }

    /**
     * @brief Generate complete file path for a specific format
     * @param format Target export format
     * @return Complete file path including directory, base name, and format
     * extension
     * @details Combines exportDir, baseFileName, and format-specific extension
     *          to create a complete file path for the specified format.
     */
    QString getFilePathForFormat(Format format) const;

    /**
     * @brief Get file extension for a specific format
     * @param format Target format
     * @return File extension string (e.g., ".txt", ".csv", ".json")
     * @details Static utility method for getting the appropriate file extension
     *          for each supported export format.
     */
    static QString getFormatExtension(Format format);

    /**
     * @brief Get human-readable name for a format
     * @param format Target format
     * @return Localized format name for display in UI
     * @details Static utility method for getting user-friendly format names
     *          suitable for display in user interface elements.
     */
    static QString getFormatName(Format format);
};

/**
 * @class LogExporter
 * @brief Multi-format log export engine with progress tracking
 * @details This class provides comprehensive log export functionality
 * supporting multiple file formats (TXT, CSV, JSON) with configurable field
 * inclusion, progress tracking, and batch export capabilities. It inherits from
 * QObject to provide signal-based progress updates and completion
 * notifications.
 *
 * Key features:
 * - Multi-format export (TXT, CSV, JSON)
 * - Configurable field inclusion/exclusion
 * - Progress tracking with percentage updates
 * - Batch export to multiple formats simultaneously
 * - Proper escaping for CSV and JSON formats
 * - Error handling and user feedback
 *
 * The class uses Qt's signal-slot mechanism to communicate export progress
 * and completion status to the user interface, allowing for responsive
 * progress indicators and error reporting.
 *
 * Usage example:
 * @code
 * LogExporter exporter;
 * connect(&exporter, &LogExporter::progressChanged, this,
 * &MyClass::updateProgress); connect(&exporter, &LogExporter::exportFinished,
 * this, &MyClass::handleExportComplete);
 *
 * ExportConfig config;
 * config.formats = {ExportConfig::CSV, ExportConfig::JSON};
 * config.baseFileName = "export_data";
 * config.exportDir = "/tmp";
 *
 * exporter.exportMultipleFormats(logEntries, config);
 * @endcode
 */
class LogExporter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for LogExporter
     * @param parent Parent QObject for memory management (optional)
     * @details Initializes the log exporter instance. The parent parameter
     *          follows Qt's object ownership model for automatic cleanup.
     */
    explicit LogExporter(QObject* parent = nullptr);

    /**
     * @brief Export logs to a single format
     * @param logs List of log entries to export
     * @param config Export configuration specifying format and options
     * @return True if export succeeded, false if an error occurred
     * @details Exports the provided log entries using the first format
     * specified in the configuration. Emits progress signals during export
     * process.
     */
    bool exportLogs(const QList<LogEntry>& logs, const ExportConfig& config);

    /**
     * @brief Export logs to multiple formats simultaneously
     * @param logs List of log entries to export
     * @param config Export configuration specifying formats and options
     * @return True if all exports succeeded, false if any export failed
     * @details Exports the provided log entries to all formats specified in
     *          the configuration. Progress is tracked across all formats.
     *          Emits formatExported signal for each completed format.
     */
    bool exportMultipleFormats(const QList<LogEntry>& logs,
                               const ExportConfig& config);

signals:
    /**
     * @brief Signal emitted during export to report progress
     * @param percentage Completion percentage (0-100)
     * @details Emitted periodically during export operations to allow UI
     *          components to display progress indicators to the user.
     */
    void progressChanged(int percentage);

    /**
     * @brief Signal emitted when export operation completes
     * @param success True if export succeeded, false if it failed
     * @param message Descriptive message about the export result
     * @details Emitted once when the entire export operation finishes,
     *          providing final status and any relevant messages for user
     * feedback.
     */
    void exportFinished(bool success, const QString& message);

    /**
     * @brief Signal emitted when each individual format export completes
     * @param format Name of the completed format
     * @param filePath Full path to the exported file
     * @details Emitted for each format when doing multi-format exports,
     *          allowing UI to provide detailed feedback about each exported
     * file.
     */
    void formatExported(const QString& format, const QString& filePath);

private:
    /**
     * @brief Export logs to TXT format
     * @param logs Log entries to export
     * @param config Export configuration
     * @param filePath Target file path
     * @return True if export succeeded, false otherwise
     * @details Creates a human-readable text file with one log entry per line,
     *          formatted according to the field inclusion settings.
     */
    bool exportToTxt(const QList<LogEntry>& logs, const ExportConfig& config,
                     const QString& filePath);

    /**
     * @brief Export logs to CSV format
     * @param logs Log entries to export
     * @param config Export configuration
     * @param filePath Target file path
     * @return True if export succeeded, false otherwise
     * @details Creates a comma-separated values file with proper escaping,
     *          suitable for import into spreadsheet applications like Excel.
     */
    bool exportToCsv(const QList<LogEntry>& logs, const ExportConfig& config,
                     const QString& filePath);

    /**
     * @brief Export logs to JSON format
     * @param logs Log entries to export
     * @param config Export configuration
     * @param filePath Target file path
     * @return True if export succeeded, false otherwise
     * @details Creates a structured JSON file with proper escaping,
     *          suitable for programmatic processing and data exchange.
     */
    bool exportToJson(const QList<LogEntry>& logs, const ExportConfig& config,
                      const QString& filePath);

    /**
     * @brief Format a single log entry for specified export format
     * @param entry Log entry to format
     * @param config Export configuration specifying field inclusion
     * @param format Target export format
     * @return Formatted string representation of the log entry
     * @details Converts a LogEntry object to its string representation
     *          appropriate for the specified format, respecting field inclusion
     * settings.
     */
    QString formatLogEntry(const LogEntry& entry, const ExportConfig& config,
                           ExportConfig::Format format,
                           const QStringList& extraFieldNames = QStringList());

    /**
     * @brief Escape special characters for CSV format
     * @param field Text field to escape
     * @return Properly escaped text safe for CSV format
     * @details Handles comma, quote, and newline escaping according to CSV
     * standards, ensuring data integrity in the exported CSV file.
     */
    QString escapeForCsv(const QString& field);

    /**
     * @brief Emit progress signal with calculated percentage
     * @param current Current item being processed
     * @param total Total number of items to process
     * @details Helper method that calculates percentage and emits
     * progressChanged signal, providing consistent progress reporting across
     * different export methods.
     */
    void emitProgress(int current, int total);
};

#endif // LOGEXPORTER_H