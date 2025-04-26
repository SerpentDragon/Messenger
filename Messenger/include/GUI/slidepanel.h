#ifndef SLIDEPANEL_H
#define SLIDEPANEL_H

#include "common.h"

#include <QLabel>
#include <QMainWindow>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace Ui {
class SlidePanel;
}

class SlidePanel : public QWidget
{
    Q_OBJECT

public:

    explicit SlidePanel(int y, int width, int height, QWidget* parent = nullptr);

    void show_panel();

    void hide_panel();

    void set_coordinates(int y, int width, int height);

private:

    void hide_password_widgets();

    void setup_icons();

private slots:

    void on_change_passwd_label_linkActivated(const QString &link);

private:

    Ui::SlidePanel* ui;

    int y_;
    int width_, height_;
};

#endif // SLIDEPANEL_H
