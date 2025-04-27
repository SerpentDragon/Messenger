#ifndef AUTOEXPANDTEXTEDIT_H
#define AUTOEXPANDTEXTEDIT_H

#include <QTextEdit>

class AutoExpandTextEdit : public QTextEdit
{
    Q_OBJECT

public:

    AutoExpandTextEdit(QWidget* parent = nullptr);

    void set_coordinates(int y, int width);

private:

    void showEvent(QShowEvent* event);

private slots:

    void adjust_height();

private:

    int min_height_;
    int max_height_;
};

#endif // AUTOEXPANDTEXTEDIT_H
