#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "autoexpandtextedit.h"
#include "slidepanel.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QPainter>
#include <QStandardItem>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget* parent = nullptr);

    ~MainWindow();

private:

    void setup_panel();

    void setup_icons();

    void resizeEvent(QResizeEvent* event);

private:
    Ui::MainWindow* ui;

    SlidePanel* panel_;

    AutoExpandTextEdit* msg_edit_;
};
#endif // MAINWINDOW_H
