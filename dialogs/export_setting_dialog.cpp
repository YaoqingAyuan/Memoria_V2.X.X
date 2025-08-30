#include "dialogs/export_setting_dialog.h"
#include "dialogs/ui_export_setting_dialog.h"
#include <QRadioButton>
#include <QCheckBox>

// ===================== 构造函数/析构函数 =====================
export_setting_dialog::export_setting_dialog(QWidget *parent,
                                             ExportMode currentMode,
                                             bool rememberChoice)
    : QDialog(parent)
    , ui(new Ui::export_setting_dialog)
    , m_currentMode(currentMode)
    , m_rememberChoice(rememberChoice)
    , m_settingsChanged(false) // 初始化设置更改标志
{
    ui->setupUi(this);

    // 根据传入的当前模式设置单选按钮
    switch(m_currentMode) {
    case ExportSelected:
        ui->exportSelected->setChecked(true);
        break;
    case ExportAll:
        ui->exportAll->setChecked(true);
        break;
    case ExportSingle:
    default:
        ui->exportSingle->setChecked(true);
        break;
    }

    ui->remember_cheBox2->setChecked(m_rememberChoice);

    // 初始禁用应用按钮
    ui->ApplyButton->setEnabled(false);

    // 连接所有设置控件的信号到onSettingChanged槽
    connect(ui->exportSingle, &QRadioButton::toggled, this, &export_setting_dialog::onSettingChanged);
    connect(ui->exportSelected, &QRadioButton::toggled, this, &export_setting_dialog::onSettingChanged);
    connect(ui->exportAll, &QRadioButton::toggled, this, &export_setting_dialog::onSettingChanged);
    connect(ui->remember_cheBox2, &QCheckBox::checkStateChanged, this, &export_setting_dialog::onSettingChanged);
}

export_setting_dialog::~export_setting_dialog()
{
    delete ui;
}

ExportMode export_setting_dialog::getExportMode() const
{
    if (ui->exportSelected->isChecked()) {
        return ExportSelected;
    } else if (ui->exportAll->isChecked()) {
        return ExportAll;
    } else {
        return ExportSingle;
    }
}

bool export_setting_dialog::rememberChoice() const
{
    return ui->remember_cheBox2->isChecked();
}

// ===================== 按钮槽函数 =====================
void export_setting_dialog::on_OkButton_clicked()
{
    // 如果有未应用的更改，先应用它们
    if (m_settingsChanged) {
        on_ApplyButton_clicked();
    }
    accept(); // 关闭对话框
}

void export_setting_dialog::on_CancelButton_clicked()
{
    reject(); // 关闭对话框，不保存任何未应用的更改
}

void export_setting_dialog::on_ApplyButton_clicked()
{
    // 更新当前模式和记住选择状态
    m_currentMode = getExportMode();
    m_rememberChoice = rememberChoice();

    // 重置更改标志并禁用应用按钮
    m_settingsChanged = false;
    ui->ApplyButton->setEnabled(false);

    // 这里可以添加任何其他需要在应用时执行的操作
}

// ===================== 设置更改处理 =====================
void export_setting_dialog::onSettingChanged()
{
    // 设置已更改，启用应用按钮
    m_settingsChanged = true;
    ui->ApplyButton->setEnabled(true);
}
