#include "logformattemplate.h"

const QString LogFormatTemplate::DEFAULT_TEMPLATE =
    QStringLiteral("[{timestamp}] [{level}] [{module}] : {message}");

const QStringList LogFormatTemplate::KNOWN_FIELDS = {
    QStringLiteral("timestamp"),
    QStringLiteral("level"),
    QStringLiteral("module"),
    QStringLiteral("message")
};

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

    const auto presetList = presets();
    int bestIndex = -1;
    int bestMatchCount = 0;

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

        if (matchCount > bestMatchCount) {
            bestMatchCount = matchCount;
            bestIndex = pi;
        }
    }

    // Require at least 20% match rate (for mixed-content logs)
    int threshold = qMax(1, sampleLines.size() / 5);
    if (bestIndex >= 0 && bestMatchCount >= threshold) {
        return LogFormatTemplate(presetList[bestIndex].templateStr);
    }

    // Fallback: try to match timestamp-only pattern
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
