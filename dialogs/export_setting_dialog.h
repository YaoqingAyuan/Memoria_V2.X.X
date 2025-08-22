#ifndef EXPORT_SETTING_DIALOG_H
#define EXPORT_SETTING_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QPushButton>
#include "delegates/exportmode.h"  // 包含ExportMode枚举

namespace Ui {
class export_setting_dialog;
}

class export_setting_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit export_setting_dialog(QWidget *parent = nullptr,
                                   ExportMode currentMode = ExportSingle,
                                   bool rememberChoice = false);
    ~export_setting_dialog();

    ExportMode getExportMode() const;
    bool rememberChoice() const;

private slots:
    void on_OkButton_clicked();
    void on_CancelButton_clicked();
    void on_ApplyButton_clicked();
    void onSettingChanged(); // 新增：处理设置更改

private:
    Ui::export_setting_dialog *ui;
    ExportMode m_currentMode;
    bool m_rememberChoice;
    bool m_settingsChanged; // 新增：跟踪设置是否已更改
};

#endif // EXPORT_SETTING_DIALOG_H
