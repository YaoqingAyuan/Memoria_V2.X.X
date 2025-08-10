#include "setting_dialog.h"
#include "ui_setting_dialog.h"
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>

Setting_Dialog::Setting_Dialog(ColumnManager* columnManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Setting_Dialog)
    , m_columnManager(columnManager)
    , m_selectAllCheckBox(nullptr)
{
    ui->setupUi(this);

    // 设置选项卡标题
    ui->tabWidget->setTabText(0, "列设置");
    ui->tabWidget->setTabText(1, "其他设置");

    // 确保 columnsContainer 有垂直布局
    QVBoxLayout* columnsLayout = new QVBoxLayout(ui->columnsContainer);
    ui->columnsContainer->setLayout(columnsLayout);

    // === 创建全选复选框 ===
    m_selectAllCheckBox = new QCheckBox(ui->columnsContainer);
    m_selectAllCheckBox->setObjectName("selectAllCheckBox");
    m_selectAllCheckBox->setText("全选/全不选");
    m_selectAllCheckBox->setTristate(true);
    m_selectAllCheckBox->setCheckState(Qt::PartiallyChecked);
    columnsLayout->addWidget(m_selectAllCheckBox);
    // =====================


    // 添加可选列复选框
    QList<TableColumns> optionalColumns = m_columnManager->getOptionalColumns();
    for (TableColumns column : optionalColumns) {
        QCheckBox* checkBox = new QCheckBox(ui->columnsContainer);
        checkBox->setText(m_columnManager->getColumnName(column));
        checkBox->setChecked(m_columnManager->isColumnVisible(column));
        checkBox->setProperty("column", static_cast<int>(column));
        columnsLayout->addWidget(checkBox);
    }

    // 添加弹簧使复选框顶部对齐
    columnsLayout->addStretch();

    // 连接信号槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &Setting_Dialog::onAccepted);

    // 连接全选复选框的信号
    connect(m_selectAllCheckBox, &QCheckBox::stateChanged,
            this, &Setting_Dialog::onSelectAllStateChanged);

    // 连接所有选项复选框的信号
    for (QCheckBox* checkbox : ui->columnsContainer->findChildren<QCheckBox*>()) {
        if (checkbox != m_selectAllCheckBox) {
            connect(checkbox, &QCheckBox::stateChanged,
                    this, &Setting_Dialog::onOptionCheckboxChanged);
        }
    }

    // 初始更新全选状态
    updateSelectAllState();
}

Setting_Dialog::~Setting_Dialog()
{
    delete ui;
}

void Setting_Dialog::onAccepted()
{
    // 应用列设置
    for (QCheckBox* checkBox : ui->columnsContainer->findChildren<QCheckBox*>()) {
        if (checkBox != m_selectAllCheckBox && checkBox->property("column").isValid()) {
            TableColumns column = static_cast<TableColumns>(checkBox->property("column").toInt());
            m_columnManager->setColumnVisibility(column, checkBox->isChecked());
        }
    }

    accept();  // 关闭对话框
}

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
