#include "../../include/GUI/contactentry.h"

ContactEntry::ContactEntry(const QPixmap& photo, const QString& name,
                           int id, int chat, QWidget* parent) : name_(name), id_(id), chat_(chat)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel* photoLabel = new QLabel(this);
    photoLabel->setPixmap(photo.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(photoLabel);

    QLabel* textLabel = new QLabel(name_, this);
    layout->addWidget(textLabel);
}

std::tuple<const QString&, int, int> ContactEntry::get_data() const
{
    return { name_, id_, chat_ };
}
