#ifndef CONTACTENTRY_H
#define CONTACTENTRY_H

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class ContactEntry : public QWidget
{
    Q_OBJECT

public:

    ContactEntry(const QPixmap& photo, const QString& name, int id, int chat, QWidget* parent = nullptr);

    std::tuple<const QString&, int, int> get_data() const;

private:

    QString name_;

    int id_;
    int chat_;
};

#endif // CONTACTENTRY_H
