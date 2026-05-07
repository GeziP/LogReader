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

QStringList LogFormatTemplate::allFieldNames() const
{
    return m_captureMap.keys();
}

QStringList LogFormatTemplate::extraFieldNames() const
{
    QStringList result;
    for (auto it = m_captureMap.constBegin(); it != m_captureMap.constEnd(); ++it) {
        if (!KNOWN_FIELDS.contains(it.key())) {
            result.append(it.key());
        }
    }
    return result;
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

    int analyzeCount = qMin(10, logLines.size());

    // Determine timestamp format
    bool useBracketedTs = false;
    for (int i = 0; i < qMin(5, logLines.size()); ++i) {
        if (TS_BRACKETED.match(logLines[i]).hasMatch()) {
            useBracketedTs = true;
            break;
        }
    }

    // Analyze structure: collect bracketed fields after timestamp
    // and classify them as level or extra fields
    struct BracketFieldInfo {
        int count = 0;
        bool isLevel = false;
    };
    QMap<QString, BracketFieldInfo> bracketFieldMap; // field content -> info
    int hasLevelCount = 0;
    int hasModuleCount = 0;
    int totalAnalyzed = 0;

    for (int i = 0; i < analyzeCount; ++i) {
        const QString& line = logLines[i];

        // Find timestamp
        QRegularExpressionMatch tsMatch;
        tsMatch = useBracketedTs ? TS_BRACKETED.match(line) : TS_BARE.match(line);
        if (!tsMatch.hasMatch())
            continue;

        totalAnalyzed++;
        QString afterTs = line.mid(tsMatch.capturedEnd()).trimmed();

        // Collect all bracketed fields after timestamp
        QRegularExpression bracketRe(R"(^\s*\[([^\]]+)\])");
        int pos = 0;
        bool foundLevel = false;
        QString remaining = afterTs;

        while (pos < remaining.length()) {
            QRegularExpressionMatch m = bracketRe.match(remaining, pos);
            if (!m.hasMatch())
                break;

            QString content = m.captured(1).trimmed();
            QString key = content.toUpper();

            // Check if this looks like a log level
            bool isLevel = false;
            // Single word that matches known levels
            if (!content.contains(' ') && LOG_LEVELS.contains(key)) {
                isLevel = true;
            }
            // Multi-word bracket content is NOT a level (e.g., "C1 +0001")

            if (isLevel && !foundLevel) {
                foundLevel = true;
                hasLevelCount++;
                // Mark this position as level
                BracketFieldInfo& info = bracketFieldMap["__LEVEL__"];
                info.count++;
                info.isLevel = true;
            } else {
                // Extra field - use position-based naming
                QString fieldKey = QString("field_%1").arg(pos);
                bracketFieldMap[fieldKey].count++;
            }

            pos = m.capturedEnd();
        }

        // If no bracketed level found, check for bare level word
        if (!foundLevel) {
            QString afterBrackets = remaining.mid(pos).trimmed();
            QStringList words = afterBrackets.split(QRegularExpression("\\s+"),
                                                     Qt::SkipEmptyParts);
            if (!words.isEmpty() && LOG_LEVELS.contains(words[0].toUpper())) {
                hasLevelCount++;
            }
        }

        // Check for module name in remaining text
        QString afterBrackets = remaining.mid(pos).trimmed();
        QStringList words = afterBrackets.split(QRegularExpression("\\s+"),
                                                 Qt::SkipEmptyParts);
        if (!words.isEmpty()) {
            QString firstWord = words[0];
            if (QRegularExpression("^[A-Za-z][A-Za-z0-9_]*$").match(firstWord)
                    .hasMatch()) {
                hasModuleCount++;
            }
        }
    }

    if (totalAnalyzed == 0)
        return QString();

    // Build template
    bool hasLevel = hasLevelCount > totalAnalyzed / 2;
    bool hasModule = hasModuleCount > totalAnalyzed / 2;

    QString templateStr;
    if (useBracketedTs) {
        templateStr = "[{timestamp}]";
    } else {
        templateStr = "{timestamp}";
    }

    // Collect bracketed fields in order from first line
    // Re-parse first line to get field order
    QRegularExpressionMatch firstTsMatch;
    firstTsMatch = useBracketedTs ? TS_BRACKETED.match(logLines[0])
                                  : TS_BARE.match(logLines[0]);
    if (firstTsMatch.hasMatch()) {
        QString afterTs = logLines[0].mid(firstTsMatch.capturedEnd()).trimmed();
        QRegularExpression bracketRe(R"(^\s*\[([^\]]+)\])");
        int pos = 0;
        bool levelPlaced = false;
        int extraFieldIdx = 1;

        while (pos < afterTs.length()) {
            QRegularExpressionMatch m = bracketRe.match(afterTs, pos);
            if (!m.hasMatch())
                break;

            QString content = m.captured(1).trimmed();
            bool isLevel = !content.contains(' ') &&
                           LOG_LEVELS.contains(content.toUpper());

            if (isLevel && hasLevel && !levelPlaced) {
                templateStr += " [{level}]";
                levelPlaced = true;
            } else {
                templateStr += QString(" [{field%1}]").arg(extraFieldIdx);
                extraFieldIdx++;
            }

            pos = m.capturedEnd();
        }

        // If level wasn't in brackets but we detected one
        if (hasLevel && !levelPlaced) {
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
