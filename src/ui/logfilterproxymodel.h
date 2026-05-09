#ifndef LOGFILTERPROXYMODEL_H
#define LOGFILTERPROXYMODEL_H

#include <QDateTime>
#include <QMap>
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
    void setExtraFieldFilter(const QString& fieldName, const QSet<QString>& acceptedValues);
    void clearExtraFieldFilters();
    void setHideUnmatched(bool hide);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QDateTime m_start;
    QDateTime m_end;
    QSet<QString> m_levelSet;
    bool m_levelFilterActive = false;
    QSet<QString> m_moduleSet;
    bool m_moduleFilterActive = false;
    QMap<QString, QSet<QString>> m_extraFilters;
    bool m_hideUnmatched = false;
};

#endif // LOGFILTERPROXYMODEL_H


