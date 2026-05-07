#ifndef LOGLOADER_H
#define LOGLOADER_H

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include "logentry.h"

/**
 * @class LogLoader
 * @brief Background loader that parses log files in chunks and emits progress.
 */
class LogLoader : public QObject
{
    Q_OBJECT

public:
    explicit LogLoader(const QString& filePath, const QString& encoding,
                       int chunkSize = 5000,
                       const QString& formatTemplate = QString(),
                       QObject* parent = nullptr);

public slots:
    void process();

signals:
    void chunkReady(QVector<LogEntry> chunk);
    void progress(int percentage);
    void summaryReady(const QDateTime& minTime, const QDateTime& maxTime,
                      const QStringList& modules, const QStringList& levels,
                      const QStringList& extraFieldNames,
                      const QMap<QString, QStringList>& extraFieldValues);
    void finished();
    void error(const QString& message);

private:
    QString m_filePath;
    QString m_encoding;
    int m_chunkSize;
    QString m_formatTemplate;
};

#endif // LOGLOADER_H
