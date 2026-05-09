#include "highlightdelegate.h"

#include <QApplication>
#include <QPainter>
#include <QTextLayout>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QPalette>

HighlightDelegate::HighlightDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void HighlightDelegate::setHighlightText(const QString& text)
{
    if (m_text == text)
        return;
    m_text = text;
}

void HighlightDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    // Only highlight on the message column (fixed index 4)
    if (m_text.isEmpty() || index.column() != 4) {
        QStyledItemDelegate::paint(painter, opt, index);
        return;
    }

    // Prepare text and prevent default text drawing (avoid double painting)
    QString text = opt.text;
    opt.text.clear();

    // Draw background and focus as usual (without text)
    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    // Prepare text painting
    painter->save();
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, opt.widget);

    // Case-insensitive highlight
    QString lower = text.toLower();
    QString needle = m_text.toLower();

    int pos = 0;
    int last = 0;

    // Colors
    QColor highlightBg = QColor(255, 255, 0);
    QColor highlightFg = opt.palette.color(QPalette::Text);

    QFontMetrics fm(opt.font);
    int x = textRect.left();
    int y = textRect.top() + fm.ascent() + (textRect.height() - fm.height()) / 2;

    while (true) {
        pos = lower.indexOf(needle, last);
        if (pos < 0)
            break;

        QString before = text.mid(last, pos - last);
        if (!before.isEmpty()) {
            painter->setPen(opt.palette.color(QPalette::Text));
            painter->drawText(x, y, before);
            x += fm.horizontalAdvance(before);
        }

        QString match = text.mid(pos, needle.size());
        if (!match.isEmpty()) {
            int w = fm.horizontalAdvance(match);
            QRect r(x, textRect.top(), w, textRect.height());
            painter->fillRect(r, highlightBg);
            painter->setPen(highlightFg);
            painter->drawText(x, y, match);
            x += w;
        }

        last = pos + needle.size();
    }

    QString tail = text.mid(last);
    if (!tail.isEmpty()) {
        painter->setPen(opt.palette.color(QPalette::Text));
        painter->drawText(x, y, tail);
    }

    painter->restore();
}


