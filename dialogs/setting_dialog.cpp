#include "dialogs/setting_dialog.h"
#include "dialogs/ui_setting_dialog.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>
#include "del_setting_dialog.h"
#include "dialogs/export_setting_dialog.h"
#include "mainwindow.h"
#include "data_models/tablemanager.h" // 添加包含

// ===================== 构造函数/析构函数 =====================
Setting_Dialog::Setting_Dialog(TableManager* tableManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Setting_Dialog)
    , m_tableManager(tableManager)
    , m_selectAllCheckBox(nullptr)
    , m_exportDialogShowing(false)
    , m_deleteDialogShowing(false)
    , m_settingsChanged(false)
{
    qDebug() << "Setting_Dialog created" << this;
    qDebug() << "Parent:" << parent;

    ui->setupUi(this);

    // 通过 TableManager 获取 ColumnManager
    ColumnManager* columnManager = &m_tableManager->columnManager();

    // 获取主窗口引用
    m_mainWindow = qobject_cast<MainWindow*>(parent);

    // 初始化模式状态
    if (m_mainWindow) {
        m_currentDeleteMode = m_mainWindow->getDeleteMode();
        m_currentRememberChoice = m_mainWindow->getRememberChoice();
        updateDeleteModeDisplay();

        m_currentExportMode = m_mainWindow->getExportMode();
        m_currentExportRememberChoice = m_mainWindow->getExportRememberChoice();
        updateExportModeDisplay();
    }

    // 设置选项卡标题
    ui->tabWidget->setTabText(0, "列设置");
    ui->tabWidget->setTabText(1, "其他设置");

    // 确保 columnsContainer 有垂直布局
    QVBoxLayout* columnsLayout = new QVBoxLayout(ui->columnsContainer);
    ui->columnsContainer->setLayout(columnsLayout);

    // 创建全选复选框
    m_selectAllCheckBox = new QCheckBox(ui->columnsContainer);
    m_selectAllCheckBox->setObjectName("selectAllCheckBox");
    m_selectAllCheckBox->setText("全选/全不选");
    m_selectAllCheckBox->setTristate(true);
    m_selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    columnsLayout->addWidget(m_selectAllCheckBox);

    // 添加可选列复选框
    QList<TableColumns> optionalColumns = columnManager->getOptionalColumns();
    for (TableColumns column : optionalColumns) {
        // 修复：使用 columnManager 获取列名，而不是未定义的 columnName
        QString columnName = columnManager->getColumnName(column);
        // 修复：明确指定父对象为 ui->columnsContainer
        QCheckBox* checkBox = new QCheckBox(columnName, ui->columnsContainer);

        checkBox->setChecked(columnManager->isColumnVisible(column));
        checkBox->setProperty("column", static_cast<int>(column));
        columnsLayout->addWidget(checkBox);

        // 连接每个复选框的信号到设置更改处理
        connect(checkBox, &QCheckBox::stateChanged, this, &Setting_Dialog::onSettingChanged);
    }

    // 添加弹簧使复选框顶部对齐
    columnsLayout->addStretch();

    // 连接信号槽 - 确保只连接一次
    static bool connected = false;
    if (!connected) {
        connect(m_selectAllCheckBox, &QCheckBox::checkStateChanged,
                this, &Setting_Dialog::onSelectAllStateChanged);
        connected = true;
    }

    // 初始更新全选状态
    updateSelectAllState();

    // 连接状态设置按钮
    connect(ui->statesetting_Button, &QPushButton::clicked,
            this, &Setting_Dialog::handleStateSettingButtonClicked);

    // 初始禁用应用按钮
    ui->ApplyButton->setEnabled(false);
}

Setting_Dialog::~Setting_Dialog()
{
    qDebug() << "~Setting_Dialog()" << this;
    qDebug() << "Deleting UI";
    delete ui;
    qDebug() << "UI deleted";
    // 移除 disconnect() 调用（Qt会自动处理）
}

// ===================== 应用设置函数 =====================
void Setting_Dialog::applySettings()
{
    // 通过 TableManager 获取 ColumnManager
    ColumnManager* columnManager = &m_tableManager->columnManager();

    // 应用列设置
    for (QCheckBox* checkBox : ui->columnsContainer->findChildren<QCheckBox*>()) {
        if (checkBox != m_selectAllCheckBox && checkBox->property("column").isValid()) {
            TableColumns column = static_cast<TableColumns>(checkBox->property("column").toInt());
            columnManager->setColumnVisibility(column, checkBox->isChecked());
        }
    }

    // 应用删除模式设置
    if (m_mainWindow) {
        m_mainWindow->setDeleteSettings(m_currentDeleteMode, m_currentRememberChoice);
        m_mainWindow->setExportSettings(m_currentExportMode, m_currentExportRememberChoice);
    }
}

// ===================== 删除模式相关函数 =====================
void Setting_Dialog::updateDeleteModeDisplay()
{
    QString stateText;
    switch(m_currentDeleteMode) {
    case DeleteFirst: stateText = "删除第一行"; break;
    case DeleteSelected: stateText = "删除选中行"; break;
    case DeleteAll: stateText = "删除所有行"; break;
    }

    if (m_currentRememberChoice) {
        stateText += " (已记住)";
    }

    ui->del_state_line->setText(stateText);
    qDebug() << "updateDeleteModeDisplay:" << stateText;
}

void Setting_Dialog::handleStateSettingButtonClicked()
{
    if (!m_mainWindow) return;

    // 确保对话框只显示一次
    if (m_deleteDialogShowing) return;
    m_deleteDialogShowing = true;

    // 使用当前设置创建对话框
    del_setting_dialog dialog(m_currentDeleteMode, this);
    dialog.setWindowTitle(tr("删除模式设置"));

    if (dialog.exec() == QDialog::Accepted) {
        // 更新本地设置
        m_currentDeleteMode = dialog.getDeleteMode();
        m_currentRememberChoice = dialog.rememberChoice();

        // 更新显示
        updateDeleteModeDisplay();

        // 设置已更改
        onSettingChanged();
    }

    m_deleteDialogShowing = false;
}

// ===================== 全选功能相关函数 =====================
void Setting_Dialog::onSelectAllStateChanged(int state)
{
    // 阻止递归调用
    static bool inProgress = false;
    if (inProgress) return;
    inProgress = true;

    // 获取所有列复选框（排除全选复选框自身）
    QList<QCheckBox*> columnCheckBoxes;
    for (QCheckBox* checkBox : ui->columnsContainer->findChildren<QCheckBox*>()) {
        if (checkBox != m_selectAllCheckBox && checkBox->property("column").isValid()) {
            columnCheckBoxes.append(checkBox);
        }
    }

    // 处理全选/全不选
    if (state == Qt::Checked) {
        for (QCheckBox* checkBox : columnCheckBoxes) {
            checkBox->setChecked(true);
        }
    } else if (state == Qt::Unchecked) {
        for (QCheckBox* checkBox : columnCheckBoxes) {
            checkBox->setChecked(false);
        }
    }

    inProgress = false;
}

void Setting_Dialog::onOptionCheckboxChanged()
{
    updateSelectAllState();
    onSettingChanged(); // 设置已更改
}

void Setting_Dialog::updateSelectAllState()
{
    // 获取所有列复选框（排除全选复选框自身）
    QList<QCheckBox*> columnCheckBoxes;
    for (QCheckBox* checkBox : ui->columnsContainer->findChildren<QCheckBox*>()) {
        if (checkBox != m_selectAllCheckBox && checkBox->property("column").isValid()) {
            columnCheckBoxes.append(checkBox);
        }
    }

    if (columnCheckBoxes.isEmpty()) return;

    // 计算选中状态
    int checkedCount = 0;
    for (QCheckBox* checkBox : columnCheckBoxes) {
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }

    // 设置全选复选框状态
    if (checkedCount == 0) {
        m_selectAllCheckBox->setCheckState(Qt::Unchecked);
    } else if (checkedCount == columnCheckBoxes.size()) {
        m_selectAllCheckBox->setCheckState(Qt::Checked);
    } else {
        m_selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    }
}

void Setting_Dialog::updateExportModeDisplay()
{
    QString stateText;
    switch(m_currentExportMode) {
    case ExportSingle: stateText = "导出第1项"; break;
    case ExportSelected: stateText = "导出选中项"; break;
    case ExportAll: stateText = "导出全部"; break;
    }

    if (m_currentExportRememberChoice) {
        stateText += " (已记住)";
    }

    ui->exp_stase_line->setText(stateText);
    qDebug() << "updateExportModeDisplay:" << stateText;
}

void Setting_Dialog::on_statesetting_Button_2_clicked()
{
    if (!m_mainWindow) return;

    // 确保对话框只显示一次
    if (m_exportDialogShowing) return;
    m_exportDialogShowing = true;

    // 使用当前设置创建对话框
    export_setting_dialog dialog(this, m_currentExportMode, m_currentExportRememberChoice);
    dialog.setWindowTitle(tr("导出模式设置"));

    int result = dialog.exec();

    // 确保在任何情况下都会重置标志
    m_exportDialogShowing = false;

    if (result == QDialog::Accepted) {
        ExportMode newMode = dialog.getExportMode();
        bool remember = dialog.rememberChoice();

        m_currentExportMode = newMode;
        m_currentExportRememberChoice = remember;

        // 更新显示
        updateExportModeDisplay();

        // 设置已更改
        onSettingChanged();
    }
}

// ===================== 按钮槽函数 =====================
void Setting_Dialog::on_OkButton_clicked()
{
    // 如果有未应用的更改，先应用它们
    if (m_settingsChanged) {
        on_ApplyButton_clicked();
    }
    accept(); // 关闭对话框
}

void Setting_Dialog::on_CancelButton_clicked()
{
    reject(); // 关闭对话框，不保存任何未应用的更改
}

void Setting_Dialog::on_ApplyButton_clicked()
{
    applySettings(); // 应用所有设置

    // 重置更改标志并禁用应用按钮
    m_settingsChanged = false;
    ui->ApplyButton->setEnabled(false);
}

// ===================== 设置更改处理 =====================
void Setting_Dialog::onSettingChanged()
{
    // 设置已更改，启用应用按钮
    m_settingsChanged = true;
    ui->ApplyButton->setEnabled(true);
}

