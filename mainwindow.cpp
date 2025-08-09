#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setting_dialog.h"  // 注意大小写
#include "singleline_import_dialog.h"
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Memoria V2.2.1");

    // 初始化表格
    initTableView();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initTableView()
{
    // 设置表格模型
    ui->MaintableView->setModel(m_tableModel);

    // 设置表头
    updateTableHeaders();

    // 设置表格属性
    ui->MaintableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MaintableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->MaintableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->MaintableView->verticalHeader()->setVisible(false);
}


void MainWindow::updateTableHeaders()
{
    // 清除原有表头
    m_tableModel->clear();

    // 获取可见列标题
    QStringList headers = m_columnManager.getVisibleHeaders();
    m_tableModel->setHorizontalHeaderLabels(headers);

    // 添加示例数据行
    QList<QStandardItem*> rowItems;
    for (int i = 0; i < headers.size(); ++i) {
        QStandardItem* item = new QStandardItem(QString("数据 %1").arg(i + 1));
        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);
}

//导入单个项目按钮槽函数，实现：
//点击弹出一个对话框，里面包含组件若干（输入地址、标题文件夹导入、标题栏等）
//点击确认以后，运行“解析函数”，将数据填写到表格中
void MainWindow::on_singleline_importButton_clicked()
{
    // 创建对话框实例（this 指定父对象，内存自动管理）
    QDialog *dialog = new singleline_import_dialog(this);

    // 设置对话框标题
    dialog->setWindowTitle(tr("导入单行数据"));

    // 1. 模态显示（阻塞主窗口）
    dialog->exec(); // 用户必须先关闭此对话框
}


//整个缓存ID源文件导入槽函数
//预计实现功能：1.点击后执行“生成表格行对象函数”、“遍历函数”、“解析函数”等
//           2.执行后填充一行“待混流视频行对象”，填写进之前写的“表格”里
void MainWindow::on_wholsoueflie_importButton_clicked()
{

}


//设置按钮的槽函数，点开以后进入“设置”对话框界面
void MainWindow::on_settingButton_clicked()
{
    // 修正：1. 类名改为 Setting_Dialog（注意大小写）
    //       2. 传入列管理器指针 &m_columnManager
    Setting_Dialog dialog(&m_columnManager, this);
    dialog.setWindowTitle(tr("设置"));

    // 显示对话框并在关闭后更新表格
    if (dialog.exec() == QDialog::Accepted) {
        updateTableHeaders();
    }

}

