#ifndef SINGLELINE_IMPORT_DIALOG_H
#define SINGLELINE_IMPORT_DIALOG_H

#include <QDialog>

namespace Ui {
class singleline_import_dialog;
}

class singleline_import_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit singleline_import_dialog(QWidget *parent = nullptr);
    ~singleline_import_dialog();

private slots:
    void on_VideoInputButton_clicked();

    void on_AudioInputButton_clicked();

    void on_TitlefolderButton_clicked();

private:
    Ui::singleline_import_dialog *ui;
};

#endif // SINGLELINE_IMPORT_DIALOG_H
