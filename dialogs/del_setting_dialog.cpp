#include "dialogs/del_setting_dialog.h"
#include "dialogs/ui_del_setting_dialog.h"
#include <QButtonGroup>

// ===================== 构造函数/析构函数 =====================
del_setting_dialog::del_setting_dialog(DeleteMode currentMode, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::del_setting_dialog)
    , m_currentMode(currentMode)
    , m_settingsChanged(false) // 初始化设置更改标志
{
    ui->setupUi(this);

    // 设置单选按钮互斥
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->del_only_line);
    buttonGroup->addButton(ui->del_selected_line);
    buttonGroup->addButton(ui->del_Allline);

    // 使用成员变量 m_currentMode
    switch(m_currentMode) {
    case DeleteFirst:
        ui->del_only_line->setChecked(true);
        break;
    case DeleteSelected:
        ui->del_selected_line->setChecked(true);
        break;
    case DeleteAll:
        ui->del_Allline->setChecked(true);
        break;
    }

    // 初始禁用应用按钮
    ui->ApplyButton->setEnabled(false);

    // 连接所有设置控件的信号到onSettingChanged槽
    connect(ui->del_only_line, &QRadioButton::toggled, this, &del_setting_dialog::onSettingChanged);
    connect(ui->del_selected_line, &QRadioButton::toggled, this, &del_setting_dialog::onSettingChanged);
    connect(ui->del_Allline, &QRadioButton::toggled, this, &del_setting_dialog::onSettingChanged);
    connect(ui->remember_cheBox, &QCheckBox::checkStateChanged, this, &del_setting_dialog::onSettingChanged);
}

del_setting_dialog::~del_setting_dialog()
{
    delete ui;
}

// ===================== 设置获取函数 =====================
DeleteMode del_setting_dialog::getDeleteMode() const
{
    if (ui->del_only_line->isChecked()) return DeleteFirst;
    if (ui->del_selected_line->isChecked()) return DeleteSelected;
    return DeleteAll;
}

bool del_setting_dialog::rememberChoice() const
{
    return ui->remember_cheBox->isChecked();
}

// ===================== 按钮槽函数 =====================
void del_setting_dialog::on_OkButton_clicked()
{
    // 如果有未应用的更改，先应用它们
    if (m_settingsChanged) {
        on_ApplyButton_clicked();
    }
    accept(); // 关闭对话框
}

void del_setting_dialog::on_CancelButton_clicked()
{
    reject(); // 关闭对话框，不保存任何未应用的更改
}

void del_setting_dialog::on_ApplyButton_clicked()
{
    // 更新当前模式（虽然外部通过getter获取，但这里我们更新内部状态）
    m_currentMode = getDeleteMode();

    // 重置更改标志并禁用应用按钮
    m_settingsChanged = false;
    ui->ApplyButton->setEnabled(false);

    // 这里可以添加任何其他需要在应用时执行的操作
}

// ===================== 设置更改处理 =====================
void del_setting_dialog::onSettingChanged()
{
    // 设置已更改，启用应用按钮
    m_settingsChanged = true;
    ui->ApplyButton->setEnabled(true);
}
