#ifndef DEL_SETTING_DIALOG_H
#define DEL_SETTING_DIALOG_H

#include <QDialog>
#include "delegates/deletemode.h"

namespace Ui {
class del_setting_dialog;
}

class del_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit del_setting_dialog(DeleteMode currentMode, QWidget *parent = nullptr);
    ~del_setting_dialog();

    // 获取用户选择的删除模式
    DeleteMode getDeleteMode() const;
    // 是否记住选择
    bool rememberChoice() const;

private slots:
    void on_OkButton_clicked();
    void on_CancelButton_clicked();
    void on_ApplyButton_clicked();
    void onSettingChanged(); // 新增：处理设置更改

private:
    Ui::del_setting_dialog *ui;
    DeleteMode m_currentMode;
    bool m_settingsChanged; // 新增：跟踪设置是否已更改
};

#endif // DEL_SETTING_DIALOG_H
