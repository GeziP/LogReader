#include "logloader.h"

#include "logformattemplate.h"

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

LogLoader::LogLoader(const QString& filePath, const QString& encoding,
                     int chunkSize, const QString& formatTemplate,
                     QObject* parent)
    : QObject(parent),
      m_filePath(filePath),
      m_encoding(encoding),
      m_chunkSize(chunkSize),
      m_formatTemplate(formatTemplate)
{
}

void LogLoader::process()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error(QObject::tr("无法打开日志文件：%1").arg(m_filePath));
        emit finished();
        return;
    }

    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec* codec = QTextCodec::codecForName(m_encoding.toUtf8());
    if (!codec) {
        emit error(QObject::tr("不支持的编码格式：%1").arg(m_encoding));
        emit finished();
        return;
    }
    in.setCodec(codec);
#else
    auto enc = QStringConverter::encodingForName(m_encoding.toUtf8());
    if (enc.has_value()) {
        in.setEncoding(*enc);
    } else {
        emit error(
            QObject::tr("不支持的编码格式：%1，已回退为UTF-8").arg(m_encoding));
        in.setEncoding(QStringConverter::Utf8);
    }
#endif

    // Determine format template: use provided or auto-detect
    LogFormatTemplate fmt;
    if (m_formatTemplate.isEmpty()) {
        // Auto-detect: read first 100 lines for probing
        QStringList sampleLines;
        qint64 startPos = in.pos();
        while (!in.atEnd() && sampleLines.size() < 100) {
            sampleLines.append(in.readLine());
        }
        in.seek(startPos);
        LogFormatTemplate::DetectInfo detectInfo =
            LogFormatTemplate::detectWithInfo(sampleLines);
        fmt = LogFormatTemplate(detectInfo.templateStr);
        emit detectInfoReady(detectInfo.templateStr, detectInfo.reason);
    } else {
        fmt = LogFormatTemplate(m_formatTemplate);
        emit detectInfoReady(m_formatTemplate, QString());
    }

    if (!fmt.isValid()) {
        emit error(QObject::tr("Invalid log format template: %1")
                       .arg(fmt.errorMessage()));
        emit finished();
        return;
    }

    QRegularExpression regex = fmt.regex();
    QVector<LogEntry> buffer;
    buffer.reserve(m_chunkSize);

    // Cache capture indices outside the loop
    int tsIdx = fmt.captureIndex("timestamp");
    int lvIdx = fmt.captureIndex("level");
    int modIdx = fmt.captureIndex("module");
    int msgIdx = fmt.captureIndex("message");
    QStringList extraFieldNames = fmt.extraFieldNames();
    QVector<int> extraIdxList;
    extraIdxList.reserve(extraFieldNames.size());
    for (const QString& fn : extraFieldNames) {
        extraIdxList.append(fmt.captureIndex(fn));
    }

    bool hasTime = false;
    QDateTime minTime;
    QDateTime maxTime;
    QSet<QString> modulesSet;
    QSet<QString> levelsSet;
    QMap<QString, QSet<QString>> extraFieldSets;

    qint64 totalBytes = file.size();
    qint64 processedBytes = 0;
    int lineCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        lineCount++;
        // 每100行才调用一次file.pos()，减少系统调用频率
        if (lineCount % 100 == 0) {
            processedBytes = file.pos();
        }
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            LogEntry entry;
            entry.rawLine = line;
            entry.matched = true;

            if (tsIdx >= 0) {
                QString tsStr = match.captured(tsIdx).trimmed();
                entry.timestamp = QDateTime::fromString(tsStr, "yyyy-MM-dd HH:mm:ss.zzz");
                if (!entry.timestamp.isValid()) {
                    entry.timestamp = QDateTime::fromString(tsStr, "yyyy-MM-dd HH:mm:ss");
                }
            }
            entry.level = (lvIdx >= 0) ? match.captured(lvIdx).trimmed()
                                       : QString();
            entry.module = (modIdx >= 0) ? match.captured(modIdx).trimmed()
                                         : QString();
            entry.message = (msgIdx >= 0) ? match.captured(msgIdx)
                                          : QString();

            for (int ei = 0; ei < extraFieldNames.size(); ++ei) {
                int idx = extraIdxList[ei];
                if (idx >= 0) {
                    QString value = match.captured(idx).trimmed();
                    entry.extraFields[extraFieldNames[ei]] = value;
                    extraFieldSets[extraFieldNames[ei]].insert(value);
                }
            }

            if (entry.timestamp.isValid()) {
                if (!hasTime) {
                    minTime = maxTime = entry.timestamp;
                    hasTime = true;
                } else {
                    if (entry.timestamp < minTime)
                        minTime = entry.timestamp;
                    if (entry.timestamp > maxTime)
                        maxTime = entry.timestamp;
                }
            }
            if (!entry.module.isEmpty())
                modulesSet.insert(entry.module);
            if (!entry.level.isEmpty())
                levelsSet.insert(entry.level);

            buffer.append(entry);
        } else {
            // Unmatched line: preserve as raw text
            LogEntry entry;
            entry.rawLine = line;
            entry.matched = false;
            buffer.append(entry);
        }

        if (buffer.size() >= m_chunkSize) {
            emit chunkReady(buffer);
            buffer.clear();
            if (totalBytes > 0) {
                int percent =
                    static_cast<int>((processedBytes * 100) / totalBytes);
                emit progress(percent);
            }
        }
    }

    if (!buffer.isEmpty()) {
        emit chunkReady(buffer);
        buffer.clear();
    }

    // 最后确保获得准确的文件位置
    processedBytes = file.pos();

    QStringList modules = QStringList(modulesSet.cbegin(), modulesSet.cend());
    QStringList levels = QStringList(levelsSet.cbegin(), levelsSet.cend());
    modules.sort(Qt::CaseInsensitive);
    levels.sort(Qt::CaseInsensitive);

    QMap<QString, QStringList> extraFieldValues;
    for (auto it = extraFieldSets.constBegin(); it != extraFieldSets.constEnd(); ++it) {
        QStringList values = QStringList(it.value().cbegin(), it.value().cend());
        values.sort(Qt::CaseInsensitive);
        extraFieldValues[it.key()] = values;
    }

    emit summaryReady(minTime, maxTime, modules, levels, extraFieldNames, extraFieldValues);
    emit progress(100);
    emit finished();
}
