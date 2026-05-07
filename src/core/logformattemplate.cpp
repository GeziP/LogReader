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
                // Check if this placeholder is inside brackets
                // by looking at the preceding non-whitespace in the regex
                bool insideBrackets = false;
                for (int j = regexStr.length() - 1; j >= 0; --j) {
                    if (regexStr[j].isSpace())
                        continue;
                    if (regexStr[j] == QLatin1Char('[') &&
                        j > 0 && regexStr[j - 1] == QLatin1Char('\\')) {
                        insideBrackets = true;
                    }
                    break;
                }
                if (insideBrackets) {
                    // Inside brackets: match everything except ']'
                    // PCRE: [^]]  means "not ]", the first ] closes the class
                    regexStr += QChar::fromLatin1('(');
                    regexStr += QChar::fromLatin1('[');
                    regexStr += QChar::fromLatin1('^');
                    regexStr += QChar::fromLatin1(']');
                    regexStr += QChar::fromLatin1(']');
                    regexStr += QChar::fromLatin1('+');
                    regexStr += QChar::fromLatin1(')');
                } else {
                    // Match non-whitespace: need \\S in the regex string
                    regexStr += QChar::fromLatin1('(');
                    regexStr += QChar::fromLatin1('\\');
                    regexStr += QChar::fromLatin1('S');
                    regexStr += QChar::fromLatin1('+');
                    regexStr += QChar::fromLatin1(')');
                }
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

    int threshold = qMax(1, sampleLines.size() / 5);

    // Step 1: Smart analysis - detect field positions dynamically
    QString smartTemplate = analyzeLineStructure(sampleLines);
    int smartMatchCount = 0;
    bool smartHasExtraFields = false;
    if (!smartTemplate.isEmpty()) {
        LogFormatTemplate fmt(smartTemplate);
        if (fmt.isValid()) {
            smartHasExtraFields = !fmt.extraFieldNames().isEmpty();
            for (const QString& line : sampleLines) {
                if (fmt.regex().match(line).hasMatch()) {
                    smartMatchCount++;
                }
            }
        }
    }

    // Step 2: Try preset templates
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

    // Step 3: Choose between smart analysis and presets
    // Prefer presets when they match equally well (they have proper field names)
    // Only prefer smart analysis when it has extra fields AND matches significantly better
    if (bestPresetIndex >= 0 && bestPresetCount >= threshold) {
        if (smartHasExtraFields && smartMatchCount >= bestPresetCount) {
            return LogFormatTemplate(smartTemplate);
        }
        return LogFormatTemplate(presetList[bestPresetIndex].templateStr);
    }

    // Step 4: Use smart template even without preset match
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

    // Step 5: Fallback - timestamp only
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

    // Parse first line to determine structure
    QRegularExpressionMatch firstTsMatch;
    firstTsMatch = useBracketedTs ? TS_BRACKETED.match(logLines[0])
                                  : TS_BARE.match(logLines[0]);
    if (!firstTsMatch.hasMatch())
        return QString();

    QString afterTs = logLines[0].mid(firstTsMatch.capturedEnd()).trimmed();

    // Collect bracketed fields
    QRegularExpression bracketRe(R"(\[([^\]]+)\])");
    int pos = 0;
    struct BracketInfo { QString content; bool isLevel; };
    QList<BracketInfo> bracketFields;

    while (pos < afterTs.length()) {
        QRegularExpressionMatch m = bracketRe.match(afterTs, pos);
        if (!m.hasMatch())
            break;

        // Check there's only whitespace between previous end and this match
        QString between = afterTs.mid(pos, m.capturedStart() - pos).trimmed();
        if (!between.isEmpty())
            break; // Non-bracket content found, stop

        QString content = m.captured(1).trimmed();
        bool isLevel = !content.contains(' ') &&
                       LOG_LEVELS.contains(content.toUpper());
        bracketFields.append({content, isLevel});
        pos = m.capturedEnd();
    }

    // Get remaining text after all brackets
    QString afterBrackets = afterTs.mid(pos).trimmed();
    QStringList remainingWords = afterBrackets.split(QRegularExpression("\\s+"),
                                                     Qt::SkipEmptyParts);

    // Analyze remaining words to identify fields
    // Strategy: check multiple lines to find consistent word count before key=value pairs
    int wordsBeforeKV = remainingWords.size(); // default: all words are fields
    for (int wi = 0; wi < remainingWords.size(); ++wi) {
        if (remainingWords[wi].contains('=')) {
            wordsBeforeKV = wi;
            break;
        }
    }

    // Check if the first remaining word is a known log level/state
    bool hasBareLevel = false;
    if (!remainingWords.isEmpty() &&
        LOG_LEVELS.contains(remainingWords[0].toUpper())) {
        hasBareLevel = true;
    }

    // Validate across multiple lines: count how many have the same bracket count
    int bracketCount = bracketFields.size();
    int consistentBracketCount = 0;
    int consistentBareLevel = 0;
    int consistentWordsBeforeKV = 0;

    for (int i = 0; i < analyzeCount; ++i) {
        QRegularExpressionMatch tsM = useBracketedTs
                                          ? TS_BRACKETED.match(logLines[i])
                                          : TS_BARE.match(logLines[i]);
        if (!tsM.hasMatch())
            continue;
        QString after = logLines[i].mid(tsM.capturedEnd()).trimmed();

        // Count brackets
        int bc = 0;
        int p = 0;
        while (p < after.length()) {
            QRegularExpressionMatch bm = bracketRe.match(after, p);
            if (!bm.hasMatch()) break;
            QString gap = after.mid(p, bm.capturedStart() - p).trimmed();
            if (!gap.isEmpty()) break;
            bc++;
            p = bm.capturedEnd();
        }
        if (bc == bracketCount) consistentBracketCount++;

        // Check bare level
        QString rest = after.mid(p).trimmed();
        QStringList words = rest.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (!words.isEmpty() && LOG_LEVELS.contains(words[0].toUpper())) {
            consistentBareLevel++;
        }

        // Count words before first key=value
        int wkv = words.size();
        for (int wi = 0; wi < words.size(); ++wi) {
            if (words[wi].contains('=')) { wkv = wi; break; }
        }
        if (wkv == wordsBeforeKV) consistentWordsBeforeKV++;
    }

    // Build template
    QString templateStr;
    if (useBracketedTs) {
        templateStr = "[{timestamp}]";
    } else {
        templateStr = "{timestamp}";
    }

    // Add bracketed fields
    int extraFieldIdx = 1;
    bool levelPlaced = false;
    for (const BracketInfo& bi : bracketFields) {
        if (bi.isLevel && !levelPlaced) {
            templateStr += " [{level}]";
            levelPlaced = true;
        } else {
            templateStr += QString(" [{field%1}]").arg(extraFieldIdx);
            extraFieldIdx++;
        }
    }

    // Add bare level if found consistently and not already placed
    if (!levelPlaced && consistentBareLevel > analyzeCount / 2) {
        templateStr += " {level}";
        levelPlaced = true;
    }

    // Add remaining words as fields (before key=value part)
    if (consistentWordsBeforeKV > analyzeCount / 2) {
        int wordIdx = 1;
        int startWord = hasBareLevel && levelPlaced ? 1 : 0; // skip level if already placed
        for (int wi = startWord; wi < wordsBeforeKV; ++wi) {
            templateStr += QString(" {field%1}").arg(extraFieldIdx);
            extraFieldIdx++;
        }
    } else if (!remainingWords.isEmpty()) {
        // No key=value detected, check if first word looks like a module
        if (remainingWords.size() >= 2) {
            // First word might be task, second might be module
            templateStr += " {module}";
        }
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
