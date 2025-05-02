#ifndef CHATPARAMSWINDOW_H
#define CHATPARAMSWINDOW_H

#include <QWidget>

namespace Ui {
class ChatParamsWindow;
}

class ChatParamsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatParamsWindow(QWidget *parent = nullptr);
    ~ChatParamsWindow();

private slots:
    void on_OK_button_pressed();

    void on_CANCEL_button_pressed();

private:
    Ui::ChatParamsWindow *ui;
};

#endif // CHATPARAMSWINDOW_H
