#ifndef LOGFILTERPROXYMODEL_H
#define LOGFILTERPROXYMODEL_H

#include <QDateTime>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QStringList>

/**
 * @class LogFilterProxyModel
 * @brief Proxy model that filters log entries by time, level, and module.
 */
class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit LogFilterProxyModel(QObject* parent = nullptr);

    void setTimeRange(const QDateTime& start, const QDateTime& end);
    void setLevels(const QStringList& levels);
    void setModules(const QStringList& modules);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QDateTime m_start;
    QDateTime m_end;
    QSet<QString> m_levelSet;
    QSet<QString> m_moduleSet;
};

#endif // LOGFILTERPROXYMODEL_H


