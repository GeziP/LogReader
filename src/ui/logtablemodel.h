#ifndef LOGTABLEMODEL_H
#define LOGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QVariant>

#include "../core/logentry.h"

/**
 * @class LogTableModel
 * @brief Lightweight table model for displaying log entries efficiently
 * @details This model stores log entries in a contiguous QVector and exposes
 *          them via a flat, read-only table interface. It minimizes per-cell
 *          object allocations compared to QStandardItemModel and is optimized
 *          for very large datasets.
 */
class LogTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        ColumnLine = 0,
        ColumnTimestamp,
        ColumnLevel,
        ColumnModule,
        ColumnMessage,
        ColumnCount
    };

    enum Roles
    {
        TimestampRole = Qt::UserRole + 1,
        LevelRole,
        ModuleRole,
        MessageRole
    };

    explicit LogTableModel(QObject* parent = nullptr);

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Data operations
    void clear();
    void appendRows(const QVector<LogEntry>& rows);
    void appendRows(QVector<LogEntry>&& rows);
    const LogEntry& at(int row) const;
    int size() const { return m_entries.size(); }

private:
    QVector<LogEntry> m_entries;
};

#endif // LOGTABLEMODEL_H


