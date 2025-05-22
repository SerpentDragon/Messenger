#ifndef CHATPARAMSWINDOW_H
#define CHATPARAMSWINDOW_H

#include "../../../API/common_structure.h"
#include "chatmemberform.h"

#include <QListView>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QWidget>
#include <vector>

namespace Ui {
class ChatParamsWindow;
}

class ChatParamsWindow : public QWidget
{
    Q_OBJECT

public:

    explicit ChatParamsWindow(QWidget *parent = nullptr);

    ~ChatParamsWindow();

    void display_contacts(const std::vector<Contact>& contacts);

private:

    void move_form(int id, bool add);

    void delete_widget_from_list(QListView* list, int id);

    void add_widget_to_list(QListView* list, QWidget* widget);

private slots:

    void on_OK_button_pressed();

    void on_CANCEL_button_pressed();

signals:

    void create_new_chat(const QString& name, qint64 time, const std::vector<int>& members);

private:

    Ui::ChatParamsWindow *ui;

    std::vector<std::shared_ptr<ChatMemberForm>> contacts_;
    std::vector<std::shared_ptr<ChatMemberForm>> members_;
};

#endif // CHATPARAMSWINDOW_H
