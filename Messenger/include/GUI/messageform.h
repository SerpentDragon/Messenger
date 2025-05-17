#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include "../common_defs.h"

#include <QWidget>

namespace Ui {
class MessageForm;
}

class MessageForm : public QWidget
{
    Q_OBJECT

public:

    explicit MessageForm(ClientMessage* message, bool left_align, QWidget *parent = nullptr);

    ~MessageForm();

public:

    Ui::MessageForm* ui;

    ClientMessage* msg;
};

#endif // MESSAGEFORM_H
