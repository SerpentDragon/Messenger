#include "../../include/GUI/autoexpandtextedit.h"

AutoExpandTextEdit::AutoExpandTextEdit(QWidget* parent)
    : QTextEdit(parent), min_height_(50), max_height_(150)
{
    setFixedHeight(min_height_);
    setPlaceholderText("Message");
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setGeometry(0, this->parentWidget()->height() - this->height(),
                this->parentWidget()->width(), this->height());

    setStyleSheet(
        "QTextEdit {"
        " padding-top: 6px;"
        " padding-bottom: 6px;"
        " background: white;"
        " border: 1px solid gray;"
        " border-radius: 12px;"
        "}"
        );

    connect(this, &QTextEdit::textChanged, this, &AutoExpandTextEdit::adjust_height);
}

void AutoExpandTextEdit::set_coordinates(int y, int width)
{
    setGeometry(0, y - this->height(), width, this->height());
}

void AutoExpandTextEdit::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (QWidget* p = parentWidget())
    {
        int parent_width = p->width();
        int parent_height = p->height();

        setGeometry(0, parent_height - height(), parent_width, this->height());
    }
}

void AutoExpandTextEdit::adjust_height()
{
    document()->setTextWidth(width());

    int doc_height = document()->size().height() + 5;

    if (doc_height < min_height_)
    {
        doc_height = min_height_;
    }
    else if (doc_height > max_height_)
    {
        doc_height = max_height_;
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    setFixedHeight(doc_height);

    set_coordinates(this->parentWidget()->height(), this->width());
}
