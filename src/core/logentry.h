/**
 * @file logentry.h
 * @brief Log Entry Data Structure Definition
 * @details Defines the LogEntry structure used throughout the LogReader
 * application to represent individual log entries with their associated
 * metadata. This structure serves as the primary data model for log analysis.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <QDateTime>
#include <QString>

/**
 * @struct LogEntry
 * @brief Data structure representing a single log entry
 * @details This structure encapsulates all the essential information of a log
 * entry including its timestamp, severity level, source module, and content.
 * It is designed to be lightweight and efficiently processed for filtering,
 * searching, and display operations.
 *
 * The structure follows the common log format pattern:
 * [timestamp] [level] [module] : content
 *
 * Example log entry:
 * [2025-06-27 14:30:15.123] [ERROR] [Database] : Connection failed
 *
 * Usage:
 * @code
 * LogEntry entry;
 * entry.timestamp = QDateTime::currentDateTime();
 * entry.level = "INFO";
 * entry.module = "Application";
 * entry.message = "Application started successfully";
 * @endcode
 */
struct LogEntry {
    /**
     * @brief Timestamp when the log entry was created
     * @details Precise date and time information including milliseconds
     *          when available. Used for chronological ordering and
     *          time-range filtering operations.
     */
    QDateTime timestamp;

    /**
     * @brief Log severity level
     * @details Indicates the importance or severity of the log entry.
     *          Common values include: DEBUG, INFO, WARN, ERROR, FATAL.
     *          Used for filtering logs by severity level.
     */
    QString level;

    /**
     * @brief Source module or component name
     * @details Identifies the software module, component, or subsystem
     *          that generated this log entry. Used for module-based
     *          filtering and organization. Examples: "Database", "UI",
     * "Network"
     */
    QString module;

    /**
     * @brief Actual log message content
     * @details The descriptive text of the log entry containing the
     *          actual information being logged. This is the primary
     *          content that users search through and analyze.
     */
    QString message;
};

#endif // LOGENTRY_H
