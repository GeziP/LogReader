#ifndef HIGHLIGHTDELEGATE_H
#define HIGHLIGHTDELEGATE_H

#include <QStyledItemDelegate>
#include <QString>

/**
 * @class HighlightDelegate
 * @brief Paints highlighted substrings in the content column without mutating model data.
 */
class HighlightDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit HighlightDelegate(QObject* parent = nullptr);

    void setHighlightText(const QString& text);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString m_text;
};

#endif // HIGHLIGHTDELEGATE_H


