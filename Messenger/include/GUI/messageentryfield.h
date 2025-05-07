#ifndef MESSAGEENTRYFIELD_H
#define MESSAGEENTRYFIELD_H

#include "autoexpandtextedit.h"

#include <QCheckBox>
#include <QMenu>
#include <QPushButton>
#include <QWidget>
#include <QWidgetAction>

class MessageEntryField : public QWidget
{
    Q_OBJECT

public:

    explicit MessageEntryField(QWidget *parent);

    void calculate_widgets_pos();

private:

    void showEvent(QShowEvent* event);

    void set_buttons_style();

    void set_buttons_geometry();

    void msg_params_button_pressed();

    void send_msg_button_pressed();

private slots:

    void handle_height_change(int height);

signals:

    void send_msg_text(const QString& text);

private:

    AutoExpandTextEdit* msg_edit_;

    QPushButton* msg_params_;
    QPushButton* attach_file_;
    QPushButton* send_msg_;

    constexpr static int button_sz_ = 50;
};

#endif // MESSAGEENTRYFIELD_H
