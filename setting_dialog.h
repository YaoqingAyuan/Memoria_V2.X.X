#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include "tablecolumns.h"
#include <QCheckBox> //AI连这种低级错误都检查不出来，干什么吃的？

namespace Ui {
class Setting_Dialog;
}

class Setting_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Setting_Dialog(ColumnManager* columnManager, QWidget *parent = nullptr);
    ~Setting_Dialog();

private slots:
    void onAccepted();
    void onSelectAllStateChanged(int state);
    void onOptionCheckboxChanged();

private:
    Ui::Setting_Dialog *ui;
    ColumnManager* m_columnManager;
    QCheckBox* m_selectAllCheckBox;

    void updateSelectAllState();

};


#endif // SETTING_DIALOG_H
