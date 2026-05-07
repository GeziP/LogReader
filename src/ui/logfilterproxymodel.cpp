#include "logfilterproxymodel.h"

#include <QMap>
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

void LogFilterProxyModel::setExtraFieldFilter(const QString& fieldName, const QSet<QString>& acceptedValues)
{
    if (acceptedValues.isEmpty()) {
        m_extraFilters.remove(fieldName);
    } else {
        m_extraFilters[fieldName] = acceptedValues;
    }
    invalidateFilter();
}

void LogFilterProxyModel::clearExtraFieldFilters()
{
    m_extraFilters.clear();
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

    if (!m_extraFilters.isEmpty()) {
        QModelIndex extraIndex = sourceModel()->index(source_row, LogTableModel::ColumnMessage, source_parent);
        QMap<QString, QString> extraFields = sourceModel()->data(extraIndex, LogTableModel::ExtraFieldsRole).value<QMap<QString, QString>>();
        for (auto it = m_extraFilters.constBegin(); it != m_extraFilters.constEnd(); ++it) {
            QString value = extraFields.value(it.key());
            if (!it.value().contains(value))
                return false;
        }
    }

    return true;
}


