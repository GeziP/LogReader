#include "logtablemodel.h"

#include <QDateTime>
#include <QVariant>

LogTableModel::LogTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int LogTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

int LogTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant LogTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int col = index.column();
    if (row < 0 || row >= m_entries.size())
        return QVariant();

    const LogEntry& entry = m_entries.at(row);

    if (role == Qt::DisplayRole) {
        switch (col) {
        case ColumnLine:
            return row + 1;
        case ColumnTimestamp:
            return entry.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        case ColumnLevel:
            return entry.level;
        case ColumnModule:
            return entry.module;
        case ColumnMessage:
            return entry.message;
        default:
            return QVariant();
        }
    }

    if (role == TimestampRole)
        return entry.timestamp;
    if (role == LevelRole)
        return entry.level;
    if (role == ModuleRole)
        return entry.module;
    if (role == MessageRole)
        return entry.message;

    return QVariant();
}

QVariant LogTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnLine:
            return QObject::tr("行号");
        case ColumnTimestamp:
            return QObject::tr("时间");
        case ColumnLevel:
            return QObject::tr("等级");
        case ColumnModule:
            return QObject::tr("模块");
        case ColumnMessage:
            return QObject::tr("内容");
        default:
            break;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void LogTableModel::clear()
{
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

void LogTableModel::appendRows(const QVector<LogEntry>& rows)
{
    if (rows.isEmpty())
        return;
    int start = m_entries.size();
    int end = start + rows.size() - 1;
    beginInsertRows(QModelIndex(), start, end);
    m_entries += rows;
    endInsertRows();
}

const LogEntry& LogTableModel::at(int row) const
{
    return m_entries.at(row);
}


