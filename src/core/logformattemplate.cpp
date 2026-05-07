#include "logformattemplate.h"

#include <QRegularExpressionMatchIterator>
#include <QSet>

const QString LogFormatTemplate::DEFAULT_TEMPLATE =
    QStringLiteral("[{timestamp}] [{level}] [{module}] : {message}");

const QStringList LogFormatTemplate::KNOWN_FIELDS = {
    QStringLiteral("timestamp"),
    QStringLiteral("level"),
    QStringLiteral("module"),
    QStringLiteral("message")
};

// Common log level names
static const QSet<QString> LOG_LEVELS = {
    "TRACE", "DEBUG", "INFO", "WARN", "WARNING", "ERROR", "FATAL", "SEVERE",
    "START", "END", "EMIT", "WAIT", "READY", "PASS", "FAIL"
};

// Timestamp patterns
static const QRegularExpression TS_BRACKETED(
    R"(\[\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:\s*[+-]\d{2}:?\d{2})?\])");
static const QRegularExpression TS_BARE(
    R"(\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(?:\.\d+)?(?:\s*[+-]\d{2}:?\d{2})?)");

LogFormatTemplate::LogFormatTemplate()
    : LogFormatTemplate(DEFAULT_TEMPLATE)
{
}

LogFormatTemplate::LogFormatTemplate(const QString& templateStr)
    : m_template(templateStr), m_valid(false)
{
    compile();
}

void LogFormatTemplate::setTemplate(const QString& templateStr)
{
    m_template = templateStr;
    compile();
}

QString LogFormatTemplate::templateString() const
{
    return m_template;
}

QRegularExpression LogFormatTemplate::regex() const
{
    return m_regex;
}

int LogFormatTemplate::captureIndex(const QString& fieldName) const
{
    return m_captureMap.value(fieldName, -1);
}

bool LogFormatTemplate::isValid() const
{
    return m_valid;
}

QString LogFormatTemplate::errorMessage() const
{
    return m_errorMessage;
}

void LogFormatTemplate::compile()
{
    m_captureMap.clear();
    m_regex = QRegularExpression();
    m_valid = false;
    m_errorMessage.clear();

    if (m_template.isEmpty()) {
        m_errorMessage = QStringLiteral("Template is empty");
        return;
    }

    QString regexStr;
    int captureCount = 0;
    int i = 0;
    const int len = m_template.length();

    while (i < len) {
        QChar ch = m_template[i];

        // Handle escape sequences
        if (ch == '\\' && i + 1 < len) {
            QChar next = m_template[i + 1];
            if (next == '{' || next == '}' || next == '\\') {
                regexStr += QRegularExpression::escape(QString(next));
                i += 2;
                continue;
            }
        }

        // Handle placeholder
        if (ch == '{') {
            int closeBrace = m_template.indexOf('}', i + 1);
            if (closeBrace < 0) {
                m_errorMessage = QStringLiteral("Unmatched '{' at position %1").arg(i);
                return;
            }

            QString fieldName = m_template.mid(i + 1, closeBrace - i - 1);
            if (!KNOWN_FIELDS.contains(fieldName)) {
                m_errorMessage =
                    QStringLiteral("Unknown placeholder '{%1}'").arg(fieldName);
                return;
            }
            if (m_captureMap.contains(fieldName)) {
                m_errorMessage =
                    QStringLiteral("Duplicate placeholder '{%1}'").arg(fieldName);
                return;
            }

            captureCount++;
            m_captureMap[fieldName] = captureCount;

            if (fieldName == QStringLiteral("message")) {
                regexStr += QStringLiteral("(.*)");
            } else if (fieldName == QStringLiteral("timestamp")) {
                regexStr += QStringLiteral("(.*?)");
            } else {
                regexStr += QStringLiteral("(\\S+)");
            }

            i = closeBrace + 1;
            continue;
        }

        // Literal character: escape for regex
        regexStr += QRegularExpression::escape(QString(ch));
        i++;
    }

    if (m_captureMap.isEmpty()) {
        m_errorMessage = QStringLiteral("No placeholders found in template");
        return;
    }

    m_regex = QRegularExpression(regexStr);
    if (!m_regex.isValid()) {
        m_errorMessage =
            QStringLiteral("Invalid regex: %1").arg(m_regex.errorString());
        return;
    }

    m_regex.optimize();
    m_valid = true;
}

LogFormatTemplate LogFormatTemplate::detect(const QStringList& sampleLines)
{
    if (sampleLines.isEmpty()) {
        return LogFormatTemplate();
    }

    // Step 1: Try preset templates
    const auto presetList = presets();
    int bestPresetIndex = -1;
    int bestPresetCount = 0;

    for (int pi = 0; pi < presetList.size(); ++pi) {
        LogFormatTemplate fmt(presetList[pi].templateStr);
        if (!fmt.isValid())
            continue;

        int matchCount = 0;
        for (const QString& line : sampleLines) {
            if (fmt.regex().match(line).hasMatch()) {
                matchCount++;
            }
        }

        if (matchCount > bestPresetCount) {
            bestPresetCount = matchCount;
            bestPresetIndex = pi;
        }
    }

    int threshold = qMax(1, sampleLines.size() / 5);
    if (bestPresetIndex >= 0 && bestPresetCount >= threshold) {
        return LogFormatTemplate(presetList[bestPresetIndex].templateStr);
    }

    // Step 2: Smart analysis - try to detect field positions dynamically
    QString smartTemplate = analyzeLineStructure(sampleLines);
    if (!smartTemplate.isEmpty()) {
        LogFormatTemplate fmt(smartTemplate);
        if (fmt.isValid()) {
            int matchCount = 0;
            for (const QString& line : sampleLines) {
                if (fmt.regex().match(line).hasMatch()) {
                    matchCount++;
                }
            }
            if (matchCount >= threshold) {
                return fmt;
            }
        }
    }

    // Step 3: Fallback - timestamp only
    LogFormatTemplate fallback(QStringLiteral("[{timestamp}] {message}"));
    int fallbackCount = 0;
    for (const QString& line : sampleLines) {
        if (fallback.regex().match(line).hasMatch()) {
            fallbackCount++;
        }
    }
    if (fallbackCount >= threshold) {
        return fallback;
    }

    return LogFormatTemplate();
}

QString LogFormatTemplate::analyzeLineStructure(const QStringList& lines)
{
    // Find lines that look like actual log entries (with timestamp)
    QStringList logLines;
    for (const QString& line : lines) {
        if (TS_BRACKETED.match(line).hasMatch() || TS_BARE.match(line).hasMatch()) {
            logLines.append(line);
        }
    }

    if (logLines.isEmpty()) {
        return QString();
    }

    // Analyze first few log lines to determine structure
    int analyzeCount = qMin(10, logLines.size());
    QMap<QString, int> fieldPositions; // field name -> position count

    for (int i = 0; i < analyzeCount; ++i) {
        const QString& line = logLines[i];

        // Find timestamp
        QRegularExpressionMatch tsMatch;
        bool tsBracketed = true;
        tsMatch = TS_BRACKETED.match(line);
        if (!tsMatch.hasMatch()) {
            tsMatch = TS_BARE.match(line);
            tsBracketed = false;
        }

        if (!tsMatch.hasMatch())
            continue;

        int tsStart = tsMatch.capturedStart();
        int tsEnd = tsMatch.capturedEnd();

        // Get content after timestamp
        QString afterTs = line.mid(tsEnd).trimmed();

        // Check if there's a second bracketed field (level or cycle info)
        QRegularExpression bracketField(R"(^\s*\[([^\]]+)\])");
        QRegularExpressionMatch bracketMatch = bracketField.match(afterTs);

        QString level;
        QString rest;

        if (bracketMatch.hasMatch()) {
            // Has bracketed field after timestamp
            level = bracketMatch.captured(1).trimmed();
            rest = afterTs.mid(bracketMatch.capturedEnd()).trimmed();
        } else {
            // No bracket - check for bare level word
            QStringList words = afterTs.split(QRegularExpression("\\s+"),
                                               Qt::SkipEmptyParts);
            if (!words.isEmpty() && LOG_LEVELS.contains(words[0].toUpper())) {
                level = words[0];
                rest = words.mid(1).join(" ");
            }
        }

        // Try to detect module name (first word of rest that looks like identifier)
        if (!rest.isEmpty()) {
            QStringList words = rest.split(QRegularExpression("\\s+"),
                                            Qt::SkipEmptyParts);
            if (!words.isEmpty()) {
                QString firstWord = words[0];
                // Check if it looks like a module name (alphanumeric, no special chars)
                if (QRegularExpression("^[A-Za-z][A-Za-z0-9_]*$").match(firstWord)
                        .hasMatch()) {
                    // Likely a module name
                    if (!level.isEmpty()) {
                        fieldPositions["has_module"]++;
                    }
                }
            }
        }

        if (!level.isEmpty()) {
            fieldPositions["has_level"]++;
        }
    }

    // Build template based on analysis
    bool hasLevel = fieldPositions.value("has_level", 0) > analyzeCount / 2;
    bool hasModule = fieldPositions.value("has_module", 0) > analyzeCount / 2;

    // Determine timestamp format
    bool useBracketedTs = false;
    for (int i = 0; i < qMin(5, logLines.size()); ++i) {
        if (TS_BRACKETED.match(logLines[i]).hasMatch()) {
            useBracketedTs = true;
            break;
        }
    }

    QString templateStr;
    if (useBracketedTs) {
        templateStr = "[{timestamp}]";
    } else {
        templateStr = "{timestamp}";
    }

    if (hasLevel) {
        // Check if level is in brackets
        QRegularExpression bracketLevel(R"(\[\s*\w+\s*\])");
        bool levelInBrackets = false;
        for (int i = 0; i < qMin(5, logLines.size()); ++i) {
            QRegularExpressionMatch tsMatch = useBracketedTs
                                                  ? TS_BRACKETED.match(logLines[i])
                                                  : TS_BARE.match(logLines[i]);
            if (tsMatch.hasMatch()) {
                QString afterTs = logLines[i].mid(tsMatch.capturedEnd()).trimmed();
                if (bracketLevel.match(afterTs).hasMatch()) {
                    levelInBrackets = true;
                    break;
                }
            }
        }

        if (levelInBrackets) {
            templateStr += " [{level}]";
        } else {
            templateStr += " {level}";
        }
    }

    if (hasModule) {
        templateStr += " {module}";
    }

    templateStr += " {message}";

    return templateStr;
}

QList<LogFormatTemplate::Preset> LogFormatTemplate::presets()
{
    return {
        {QStringLiteral("Default"),
         QStringLiteral("[{timestamp}] [{level}] [{module}] : {message}")},
        {QStringLiteral("Log4j"),
         QStringLiteral("{timestamp} {level} {module} - {message}")},
        {QStringLiteral("Simple"),
         QStringLiteral("{timestamp} [{level}] {message}")},
        {QStringLiteral("Android Logcat"),
         QStringLiteral("[{timestamp}] {level}/{module}: {message}")},
        {QStringLiteral("Bracket Level"),
         QStringLiteral("[{timestamp}] [{level}] {module} {message}")},
        {QStringLiteral("Timestamp Only"),
         QStringLiteral("[{timestamp}] {message}")}
    };
}
