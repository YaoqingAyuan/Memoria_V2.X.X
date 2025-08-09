#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include "tablecolumns.h"

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

private:
    Ui::Setting_Dialog *ui;
    ColumnManager* m_columnManager;
};


#endif // SETTING_DIALOG_H
