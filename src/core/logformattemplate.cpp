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
    "START", "END", "EMIT", "WAIT", "READY", "PASS", "FAIL", "FAULT"
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
    bool prevWasPlaceholder = false;

    while (i < len) {
        QChar ch = m_template[i];

        // Handle escape sequences
        if (ch == '\\' && i + 1 < len) {
            QChar next = m_template[i + 1];
            if (next == '{' || next == '}' || next == '\\') {
                regexStr += QRegularExpression::escape(QString(next));
                prevWasPlaceholder = false;
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

            prevWasPlaceholder = true;
            i = closeBrace + 1;
            continue;
        }

        // Handle whitespace: use \s+ between placeholders for flexible spacing
        if (ch.isSpace()) {
            // Collect the run of whitespace
            int spaceStart = i;
            while (i < len && m_template[i].isSpace())
                i++;

            // Check if next non-space char starts a placeholder
            bool nextIsPlaceholder = (i < len && m_template[i] == '{');

            if (prevWasPlaceholder && nextIsPlaceholder) {
                // Between two placeholders: use \s+ for flexible matching
                regexStr += QStringLiteral("\\s+");
            } else {
                // Literal whitespace: preserve as-is
                regexStr += QRegularExpression::escape(m_template.mid(spaceStart, i - spaceStart));
            }
            continue;
        }

        // Literal character: escape for regex
        regexStr += QRegularExpression::escape(QString(ch));
        prevWasPlaceholder = false;
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
    return LogFormatTemplate(detectWithInfo(sampleLines).templateStr);
}

LogFormatTemplate::DetectInfo LogFormatTemplate::detectWithInfo(const QStringList& sampleLines)
{
    DetectInfo info;
    if (sampleLines.isEmpty()) {
        info.reason = QStringLiteral("无样本行");
        return info;
    }

    int threshold = qMax(1, sampleLines.size() / 5);
    int totalLines = sampleLines.size();

    // Step 1: Smart analysis - detect field positions dynamically.
    QString smartTemplate = analyzeLineStructure(sampleLines);
    int smartMatchCount = 0;
    int smartFieldCount = 0;
    if (!smartTemplate.isEmpty()) {
        LogFormatTemplate fmt(smartTemplate);
        if (fmt.isValid()) {
            smartFieldCount = fmt.allFieldNames().size();
            for (const QString& line : sampleLines) {
                if (fmt.regex().match(line).hasMatch()) {
                    smartMatchCount++;
                }
            }
        }
    }

    // Step 2: Find best preset (for fallback comparison)
    static const QRegularExpression tsValidation(
        R"(\d{4}[-/]\d{2}[-/]\d{2})");
    const auto presetList = presets();
    int bestPresetIndex = -1;
    int bestPresetCount = 0;

    for (int pi = 0; pi < presetList.size(); ++pi) {
        LogFormatTemplate fmt(presetList[pi].templateStr);
        if (!fmt.isValid())
            continue;

        int tsIdx = fmt.captureIndex("timestamp");
        int matchCount = 0;
        for (const QString& line : sampleLines) {
            QRegularExpressionMatch m = fmt.regex().match(line);
            if (!m.hasMatch())
                continue;
            if (tsIdx >= 0) {
                QString ts = m.captured(tsIdx).trimmed();
                if (!tsValidation.match(ts).hasMatch())
                    continue;
            }
            matchCount++;
        }

        if (matchCount > bestPresetCount) {
            bestPresetCount = matchCount;
            bestPresetIndex = pi;
        }
    }

    // Step 3: Choose the best template.
    if (smartMatchCount >= threshold && smartFieldCount > 0) {
        int presetFieldCount = (bestPresetIndex >= 0)
            ? LogFormatTemplate(presetList[bestPresetIndex].templateStr).allFieldNames().size()
            : 0;

        // Prefer preset when it matches well — it preserves semantic field
        // names (e.g. module) that smart detection may replace with generic
        // placeholders (e.g. field1).
        if (bestPresetIndex >= 0 && bestPresetCount >= threshold
            && presetFieldCount >= smartFieldCount) {
            info.templateStr = presetList[bestPresetIndex].templateStr;
            info.reason = QStringLiteral("使用预设'%1', 匹配%2/%3行")
                              .arg(presetList[bestPresetIndex].name)
                              .arg(bestPresetCount)
                              .arg(totalLines);
            return info;
        }

        if (smartFieldCount > presetFieldCount) {
            info.templateStr = smartTemplate;
            int extra = smartFieldCount - 4; // subtract known fields
            info.reason = QStringLiteral("智能检测: %1个字段, 匹配%2/%3行")
                              .arg(smartFieldCount)
                              .arg(smartMatchCount)
                              .arg(totalLines);
            if (extra > 0)
                info.reason += QStringLiteral(", 含%1个自定义字段").arg(extra);
            return info;
        }
        info.templateStr = smartTemplate;
        info.reason = QStringLiteral("智能检测: %1个字段, 匹配%2/%3行")
                          .arg(smartFieldCount)
                          .arg(smartMatchCount)
                          .arg(totalLines);
        return info;
    }

    // Step 4: Smart didn't work — use best preset
    if (bestPresetIndex >= 0 && bestPresetCount >= threshold) {
        info.templateStr = presetList[bestPresetIndex].templateStr;
        info.reason = QStringLiteral("使用预设'%1', 匹配%2/%3行")
                          .arg(presetList[bestPresetIndex].name)
                          .arg(bestPresetCount)
                          .arg(totalLines);
        return info;
    }

    // Step 5: Fallback - timestamp only (with validation)
    LogFormatTemplate fallback(QStringLiteral("[{timestamp}] {message}"));
    int fbTsIdx = fallback.captureIndex("timestamp");
    int fallbackCount = 0;
    for (const QString& line : sampleLines) {
        QRegularExpressionMatch m = fallback.regex().match(line);
        if (!m.hasMatch())
            continue;
        if (fbTsIdx >= 0) {
            QString ts = m.captured(fbTsIdx).trimmed();
            if (!tsValidation.match(ts).hasMatch())
                continue;
        }
        fallbackCount++;
    }
    if (fallbackCount >= threshold) {
        info.templateStr = QStringLiteral("[{timestamp}] {message}");
        info.reason = QStringLiteral("回退: 仅检测到时间戳, 匹配%1/%2行")
                          .arg(fallbackCount)
                          .arg(totalLines);
        return info;
    }

    info.reason = QStringLiteral("无法识别日志格式");
    return info;
}

QString LogFormatTemplate::analyzeLineStructure(const QStringList& lines)
{
    // Find lines that look like actual log entries (with timestamp),
    // skipping separator/banner lines (e.g. "===...", "---...")
    QStringList logLines;
    for (const QString& line : lines) {
        if (TS_BRACKETED.match(line).hasMatch() || TS_BARE.match(line).hasMatch()) {
            // Skip separator lines: content after timestamp has no letters
            QRegularExpressionMatch tsMatch = TS_BRACKETED.match(line);
            if (!tsMatch.hasMatch())
                tsMatch = TS_BARE.match(line);
            QString afterTs = line.mid(tsMatch.capturedEnd()).trimmed();
            bool hasLetter = false;
            for (const QChar& c : afterTs) {
                if (c.isLetter()) {
                    hasLetter = true;
                    break;
                }
            }
            if (hasLetter)
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

    // Build a base template (bracket fields only, no bare word fields)
    QString baseTemplate = templateStr + " {message}";

    // Try adding bare level if found consistently
    if (!levelPlaced && consistentBareLevel > analyzeCount / 2) {
        templateStr += " {level}";
        levelPlaced = true;
    }

    // Add remaining words as fields (before key=value part)
    if (consistentWordsBeforeKV > analyzeCount / 2) {
        int startWord = hasBareLevel && levelPlaced ? 1 : 0;
        // First word after level is typically the module/task ID
        if (startWord < wordsBeforeKV) {
            templateStr += " {module}";
            startWord++;
        }
        for (int wi = startWord; wi < wordsBeforeKV; ++wi) {
            templateStr += QString(" {field%1}").arg(extraFieldIdx);
            extraFieldIdx++;
        }
    }

    templateStr += " {message}";

    // Validate: if the detailed template doesn't match well due to
    // multi-space alignment or other issues, fall back to the simpler base
    if (!bracketFields.isEmpty()) {
        LogFormatTemplate detailed(templateStr);
        LogFormatTemplate simple(baseTemplate);
        if (detailed.isValid() && simple.isValid()) {
            int detailedCount = 0;
            int simpleCount = 0;
            for (int i = 0; i < analyzeCount; ++i) {
                if (detailed.regex().match(logLines[i]).hasMatch())
                    detailedCount++;
                if (simple.regex().match(logLines[i]).hasMatch())
                    simpleCount++;
            }
            // Use the simpler template if it matches significantly better
            if (simpleCount > detailedCount) {
                return baseTemplate;
            }
        }
    }

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
