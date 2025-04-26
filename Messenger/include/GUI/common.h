#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <QLineEdit>

using IconPtr = std::shared_ptr<QIcon>;

static void create_password_toggle_action(QLineEdit* line_edit, IconPtr close, IconPtr open)
{
    QAction* toggle_action = new QAction;
    toggle_action->setIcon(*close);

    line_edit->addAction(toggle_action, QLineEdit::TrailingPosition);

    QAction::connect(toggle_action, &QAction::triggered, line_edit,
        [line_edit, toggle_action, close, open]()
        {
            bool is_password_visible = (line_edit->echoMode() == QLineEdit::Normal);
            line_edit->setEchoMode(is_password_visible ? QLineEdit::Password : QLineEdit::Normal);
            toggle_action->setIcon(is_password_visible ? *close : *open);
        });
}

#endif // COMMON_H
