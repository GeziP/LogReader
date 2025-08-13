#include "logfilterproxymodel.h"

#include <QVariant>

#include "logtablemodel.h"

LogFilterProxyModel::LogFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void LogFilterProxyModel::setTimeRange(const QDateTime& start, const QDateTime& end)
{
    m_start = start;
    m_end = end;
    invalidateFilter();
}

void LogFilterProxyModel::setLevels(const QStringList& levels)
{
    m_levelSet = QSet<QString>(levels.cbegin(), levels.cend());
    invalidateFilter();
}

void LogFilterProxyModel::setModules(const QStringList& modules)
{
    m_moduleSet = QSet<QString>(modules.cbegin(), modules.cend());
    invalidateFilter();
}

bool LogFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex tsIndex = sourceModel()->index(source_row, LogTableModel::ColumnTimestamp, source_parent);
    QModelIndex lvlIndex = sourceModel()->index(source_row, LogTableModel::ColumnLevel, source_parent);
    QModelIndex modIndex = sourceModel()->index(source_row, LogTableModel::ColumnModule, source_parent);

    QDateTime ts = sourceModel()->data(tsIndex, LogTableModel::TimestampRole).toDateTime();
    QString level = sourceModel()->data(lvlIndex, LogTableModel::LevelRole).toString();
    QString module = sourceModel()->data(modIndex, LogTableModel::ModuleRole).toString();

    // Only apply time range filter when both start and end are valid
    if (m_start.isValid() && m_end.isValid() && ts.isValid()) {
        if (ts < m_start || ts > m_end)
            return false;
    }

    if (!m_levelSet.isEmpty() && !m_levelSet.contains(level))
        return false;

    if (!m_moduleSet.isEmpty() && !m_moduleSet.contains(module))
        return false;

    return true;
}


