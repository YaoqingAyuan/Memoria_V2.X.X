#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QProgressBar>
#include "progressbardelegate.h"  // 添加委托类头文件
#include "setting_dialog.h"
#include "singleline_import_dialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Memoria V2.2.2");

    // 初始化表格
    initTableView();

    // 设置上下文菜单
    setupContextMenu();

    // 设置进度条委托
    m_progressDelegate = new QProgressBar(this);
    m_progressDelegate->setRange(0, 100);
    m_progressDelegate->setTextVisible(false);
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

    // 添加交替行颜色
    ui->MaintableView->setAlternatingRowColors(true);

    // 设置表格属性
    ui->MaintableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MaintableView->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 改为ExtendedSelection
    ui->MaintableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->MaintableView->verticalHeader()->setVisible(false);

    // 设置行选择样式
    QString style = "QTableView::item:selected {"
                    "    background-color: #FFFACD;" // 浅黄色
                    "    color: black;"
                    "}";
    ui->MaintableView->setStyleSheet(style);

    // 启用标题编辑
    connect(m_tableModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem *item) {
        int row = item->row();
        if (row < m_videoItems.size()) {
            TableColumns colType = m_currentColumnsOrder[item->column()];
            if (colType == COL_TITLE) {
                m_videoItems[row]->setTitle(item->text());
            }
        }
    });

    // 设置进度条委托
    int progressCol = m_columnManager.getVisualIndex(COL_PROGRESS);
    if (progressCol >= 0) {
        ui->MaintableView->setItemDelegateForColumn(
            progressCol,
            new ProgressBarDelegate(this)
            );
    }
}


void MainWindow::updateTableHeaders()
{
    // 清除原有表头
    m_tableModel->clear();

    // 获取可见列标题
    QStringList headers = m_columnManager.getVisibleHeaders();
    m_tableModel->setHorizontalHeaderLabels(headers);

    // 更新当前列的顺序列表
    m_currentColumnsOrder.clear();
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        TableColumns column = static_cast<TableColumns>(i);
        if (m_columnManager.isColumnVisible(column)) {
            m_currentColumnsOrder.append(column);
        }
    }

    // 设置列宽策略
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_VIDEO_FILE || colType == COL_AUDIO_FILE) {
            // 视频/音频列：可调整大小
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::Interactive);
            ui->MaintableView->setColumnWidth(col, 150); // 初始宽度
            // 设置最小和最大宽度
            ui->MaintableView->horizontalHeader()->setMinimumSectionSize(100);
            ui->MaintableView->horizontalHeader()->setMaximumSectionSize(300);
        } else {
            // 其他列：根据内容调整
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::ResizeToContents);
        }
    }

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

void MainWindow::setupContextMenu()
{
    ui->MaintableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->MaintableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->MaintableView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);

    // 常规菜单项
    QAction *properties = menu.addAction("属性");
    QAction *preview = menu.addAction("预览");
    QAction *openFolder = menu.addAction("打开所在文件夹");
    menu.addSeparator();

    // 导入相关菜单项
    TableColumns column = static_cast<TableColumns>(index.column());
    if (column == COL_VIDEO_FILE || column == COL_AUDIO_FILE) {
        QAction *importFile = menu.addAction("导入单独音(视)文件");
        importFile->setData("import_file");
    }

    QAction *importTitle = menu.addAction("导入标题文件夹");
    importTitle->setData("import_title");
    QAction *importSource = menu.addAction("导入缓存源文件");
    importSource->setData("import_source");

    // 连接信号
    connect(&menu, &QMenu::triggered, this, &MainWindow::onCustomContextMenuAction);

    menu.exec(ui->MaintableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onCustomContextMenuAction(QAction* action)
{
    QString actionType = action->data().toString();
    if (actionType == "import_file") {
        // 处理导入单独文件
    } else if (actionType == "import_title") {
        // 处理导入标题文件夹
    } else if (actionType == "import_source") {
        on_wholsoueflie_importButton_clicked();
    }
}

//添加按钮，添加一行数据
void MainWindow::on_addlineButton_clicked()
{
    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    m_videoItems.append(newItem);

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_tableModel->columnCount(); ++col) {
        QStandardItem* item = new QStandardItem();
        item->setTextAlignment(Qt::AlignCenter);

        // 设置特殊列属性
        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            item->setData(0, Qt::DisplayRole); // 进度条数据
        }

        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号
    connect(newItem, &VideoItem::dataChanged, this, [this, newIndex](){
        updateTableRow(newIndex - 1);
    });
}


//删除按钮，删除一行数据
void MainWindow::on_delelineButton_clicked()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    // 倒序删除避免索引问题
    std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for (const QModelIndex &index : selected) {
        int row = index.row();
        m_tableModel->removeRow(row);
        delete m_videoItems.takeAt(row);
    }

    updateRowNumbers();
}

void MainWindow::updateRowNumbers()
{
    for (int i = 0; i < m_videoItems.size(); ++i) {
        m_videoItems[i]->setIndex(i + 1);
        updateTableRow(i);
    }
}

void MainWindow::updateTableRow(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= m_videoItems.size()) return;

    VideoItem* item = m_videoItems[rowIndex];
    for (int col = 0; col < m_tableModel->columnCount(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        QStandardItem* tableItem = m_tableModel->item(rowIndex, col);

        if (colType == COL_PROGRESS) {
            // 进度条特殊处理
            int progress = item->data(colType).toInt();
            tableItem->setData(progress, Qt::DisplayRole);
        } else {
            tableItem->setText(item->data(colType).toString());
        }
    }
}
