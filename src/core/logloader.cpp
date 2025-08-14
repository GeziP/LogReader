#include "logloader.h"

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif

LogLoader::LogLoader(const QString& filePath, const QString& encoding, int chunkSize, QObject* parent)
    : QObject(parent), m_filePath(filePath), m_encoding(encoding), m_chunkSize(chunkSize)
{
}

void LogLoader::process()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error(QObject::tr("无法打开日志文件。"));
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
        emit error(QObject::tr("不支持的编码格式：%1，已回退为UTF-8").arg(m_encoding));
        in.setEncoding(QStringConverter::Utf8);
    }
#endif

    // More tolerant spacing: allow variable spaces around tokens and colon
    QRegularExpression regex(R"((?:\[\s*(.*?)\s*\])\s*(?:\[\s*(.*?)\s*\])\s*(?:\[\s*(.*?)\s*\])\s*:\s*(.*)$)");
    regex.setPatternOptions(QRegularExpression::OptimizeOnFirstUsageOption);
    QVector<LogEntry> buffer;
    buffer.reserve(m_chunkSize);

    bool hasTime = false;
    QDateTime minTime;
    QDateTime maxTime;
    QSet<QString> modulesSet;
    QSet<QString> levelsSet;

    qint64 totalBytes = file.size();
    qint64 processedBytes = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        processedBytes = file.pos();
        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            LogEntry entry;
            entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd HH:mm:ss.zzz");
            if (!entry.timestamp.isValid()) {
                entry.timestamp = QDateTime::fromString(match.captured(1), "yyyy-MM-dd HH:mm:ss");
            }
            entry.level = match.captured(2).trimmed();
            entry.module = match.captured(3).trimmed();
            entry.message = match.captured(4);

            if (entry.timestamp.isValid()) {
                if (!hasTime) {
                    minTime = maxTime = entry.timestamp;
                    hasTime = true;
                } else {
                    if (entry.timestamp < minTime) minTime = entry.timestamp;
                    if (entry.timestamp > maxTime) maxTime = entry.timestamp;
                }
            }
            modulesSet.insert(entry.module);
            levelsSet.insert(entry.level);

            buffer.append(entry);
            if (buffer.size() >= m_chunkSize) {
                emit chunkReady(buffer);
                buffer.clear();
                int percent = totalBytes > 0 ? static_cast<int>((processedBytes * 100) / totalBytes) : 0;
                emit progress(percent);
            }
        }
    }

    if (!buffer.isEmpty()) {
        emit chunkReady(buffer);
        buffer.clear();
    }

    QStringList modules = QStringList(modulesSet.cbegin(), modulesSet.cend());
    QStringList levels = QStringList(levelsSet.cbegin(), levelsSet.cend());
    modules.sort(Qt::CaseInsensitive);
    levels.sort(Qt::CaseInsensitive);
    emit summaryReady(minTime, maxTime, modules, levels);
    emit progress(100);
    emit finished();
}


