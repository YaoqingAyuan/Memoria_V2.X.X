#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "singleline_import_dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Memoria V2.0.0"); // 修改此行

    // 通常不需要手动连接，除非自动连接失败
    //connect(ui->on_singleline_importButton_clicked(), &QPushButton::clicked, this, &MainWindow::on_singleline_importButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
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

    // 显示方式（二选一）：

    // 1. 模态显示（阻塞主窗口）
    dialog->exec(); // 用户必须先关闭此对话框
}

