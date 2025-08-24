#ifndef SETTING_DIALOG_H
#define SETTING_DIALOG_H

#include <QDialog>
#include "data_models/tablecolumns.h"
#include <QCheckBox>
#include "delegates/deletemode.h"
#include "delegates/exportmode.h"
#include "data_models/tablemanager.h" // 添加包含

class MainWindow;

namespace Ui {
class Setting_Dialog;
}

class Setting_Dialog : public QDialog
{
    Q_OBJECT

public:
    // 修改构造函数参数，使用 TableManager 而不是 ColumnManager
    explicit Setting_Dialog(TableManager* tableManager, QWidget *parent = nullptr);
    ~Setting_Dialog();

private slots:
    void onSelectAllStateChanged(int state);
    void onOptionCheckboxChanged();
    void handleStateSettingButtonClicked();
    void on_statesetting_Button_2_clicked();  // 直接处理导出设置按钮点击

    void on_OkButton_clicked();
    void on_CancelButton_clicked();
    void on_ApplyButton_clicked();
    void onSettingChanged(); // 新增：处理设置更改

private:
    Ui::Setting_Dialog *ui;
    TableManager* m_tableManager; // 改为 TableManager 指针
    QCheckBox* m_selectAllCheckBox; // 添加这行声明

    // 添加删除模式相关成员
    DeleteMode m_currentDeleteMode;
    bool m_currentRememberChoice;

    // 添加导出模式相关成员
    ExportMode m_currentExportMode;
    bool m_currentExportRememberChoice;

    MainWindow* m_mainWindow;

    // 添加对话框显示控制成员变量
    bool m_exportDialogShowing;
    bool m_deleteDialogShowing;

    // 新增：跟踪设置是否已更改
    bool m_settingsChanged;

    void updateSelectAllState();
    void updateDeleteModeDisplay();
    void updateExportModeDisplay();

    // 新增：应用所有设置
    void applySettings();
};

#endif // SETTING_DIALOG_H
