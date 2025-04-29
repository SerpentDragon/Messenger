#ifndef AUTOEXPANDTEXTEDIT_H
#define AUTOEXPANDTEXTEDIT_H

#include <QTextEdit>

class AutoExpandTextEdit : public QTextEdit
{
    Q_OBJECT

public:

    AutoExpandTextEdit(QWidget* parent = nullptr);

    void set_width(int width);

signals:

    void change_height(int);

private slots:

    void adjust_height();

private:

    int min_height_;
    int max_height_;
};

#endif // AUTOEXPANDTEXTEDIT_H
