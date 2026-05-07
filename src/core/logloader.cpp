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
        fmt = LogFormatTemplate::detect(sampleLines);
    } else {
        fmt = LogFormatTemplate(m_formatTemplate);
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

    bool hasTime = false;
    QDateTime minTime;
    QDateTime maxTime;
    QSet<QString> modulesSet;
    QSet<QString> levelsSet;
    QStringList extraFieldNames = fmt.extraFieldNames();
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
            int tsIdx = fmt.captureIndex("timestamp");
            int lvIdx = fmt.captureIndex("level");
            int modIdx = fmt.captureIndex("module");
            int msgIdx = fmt.captureIndex("message");

            if (tsIdx >= 0) {
                entry.timestamp = QDateTime::fromString(
                    match.captured(tsIdx).trimmed(),
                    "yyyy-MM-dd HH:mm:ss.zzz");
                if (!entry.timestamp.isValid()) {
                    entry.timestamp = QDateTime::fromString(
                        match.captured(tsIdx).trimmed(),
                        "yyyy-MM-dd HH:mm:ss");
                }
            }
            entry.level = (lvIdx >= 0) ? match.captured(lvIdx).trimmed()
                                       : QString();
            entry.module = (modIdx >= 0) ? match.captured(modIdx).trimmed()
                                         : QString();
            entry.message = (msgIdx >= 0) ? match.captured(msgIdx)
                                          : QString();

            for (const QString& fieldName : extraFieldNames) {
                int idx = fmt.captureIndex(fieldName);
                if (idx >= 0) {
                    QString value = match.captured(idx).trimmed();
                    entry.extraFields[fieldName] = value;
                    extraFieldSets[fieldName].insert(value);
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
            modulesSet.insert(entry.module);
            levelsSet.insert(entry.level);

            buffer.append(entry);
            if (buffer.size() >= m_chunkSize) {
                emit chunkReady(buffer);
                buffer.clear();
                // 仅在块处理完成时计算和发射进度，减少计算频率
                if (totalBytes > 0) {
                    int percent =
                        static_cast<int>((processedBytes * 100) / totalBytes);
                    emit progress(percent);
                }
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
