#include "setting_dialog.h"
#include "ui_setting_dialog.h"
#include <QCheckBox>
#include <QVBoxLayout>

Setting_Dialog::Setting_Dialog(ColumnManager* columnManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Setting_Dialog)
    , m_columnManager(columnManager)
{
    ui->setupUi(this);

    // 设置选项卡标题
    ui->tabWidget->setTabText(0, "列设置");
    ui->tabWidget->setTabText(1, "其他设置");

    // 确保 columnsContainer 有垂直布局
    QVBoxLayout* columnsLayout = new QVBoxLayout(ui->columnsContainer);
    ui->columnsContainer->setLayout(columnsLayout);

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
}

Setting_Dialog::~Setting_Dialog()
{
    delete ui;
}

void Setting_Dialog::onAccepted()
{
    // 应用列设置
    QList<QCheckBox*> checkBoxes = ui->columnsContainer->findChildren<QCheckBox*>();
    for (QCheckBox* checkBox : checkBoxes) {
        TableColumns column = static_cast<TableColumns>(checkBox->property("column").toInt());
        m_columnManager->setColumnVisibility(column, checkBox->isChecked());
    }

    accept();  // 关闭对话框
}
