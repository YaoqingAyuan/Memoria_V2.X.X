<<<<<<< HEAD
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QProgressBar>
#include <QSettings>
#include <Qmessagebox>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include "delegates/progressbardelegate.h"
#include "dialogs/setting_dialog.h"
#include "dialogs/singleline_import_dialog.h"
#include "dialogs/del_setting_dialog.h"
#include "dialogs/export_setting_dialog.h" // 添加包含

// ===================== 构造函数/析构函数 =====================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Memoria V2.2.8");

    // 初始化路径记忆
    initPathMemory();

    // 初始化表格
    initTableView();

    // 设置上下文菜单
    setupContextMenu();

    // 设置进度条委托
    m_progressDelegate = new QProgressBar(this);
    m_progressDelegate->setRange(0, 100);
    m_progressDelegate->setTextVisible(false);

    // 读取保存的删除设置
    QSettings settings;
    m_rememberDeleteChoice = settings.value("delete/remember", false).toBool();
    m_deleteMode = static_cast<DeleteMode>(settings.value("delete/mode", DeleteFirst).toInt());

    // 读取保存的导出设置
    m_rememberExportChoice = settings.value("export/remember", false).toBool();
    m_exportMode = static_cast<ExportMode>(settings.value("export/mode", ExportSingle).toInt());

    // 更新状态显示
    updateExportStatusDisplay();


}

MainWindow::~MainWindow()
{
    qDeleteAll(m_videoItems);
    delete ui;
}


// ===================== 初始化函数 =====================
void MainWindow::initTableView()
{
    // 设置表格模型
    ui->MaintableView->setModel(m_tableModel);

    // 初始化表头
    updateTableHeaders();

    // 添加交替行颜色
    ui->MaintableView->setAlternatingRowColors(true);

    // 设置表格属性
    ui->MaintableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MaintableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->MaintableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->MaintableView->verticalHeader()->setVisible(false);
    // 修改后：仅允许双击编辑
    ui->MaintableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // 设置行选择样式
    QString style = "QTableView::item:selected {"
                    "    background-color: #FFFACD;"
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
}

void MainWindow::setupContextMenu()
{
    ui->MaintableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->MaintableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}

// ===================== 表格数据处理函数 =====================
void MainWindow::updateTableHeaders()
{
    // 保存当前行数和列数
    int rowCount = m_tableModel->rowCount();
    int oldColumnCount = m_tableModel->columnCount();

    // 获取可见列标题
    QStringList headers = m_columnManager.getVisibleHeaders();

    // 更新当前列的顺序列表
    m_currentColumnsOrder.clear();
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        TableColumns column = static_cast<TableColumns>(i);
        if (m_columnManager.isColumnVisible(column)) {
            m_currentColumnsOrder.append(column);
        }
    }

    // 设置新表头
    m_tableModel->setColumnCount(headers.size());
    m_tableModel->setHorizontalHeaderLabels(headers);

    // 添加新列（如果列数增加）
    for (int row = 0; row < rowCount; ++row) {
        for (int col = oldColumnCount; col < headers.size(); ++col) {
            QStandardItem* item = new QStandardItem();
            TableColumns colType = m_currentColumnsOrder[col];

            if (colType == COL_PROGRESS) {
                item->setData(0, Qt::DisplayRole);
            } else if (colType == COL_TITLE) {
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            } else {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }

            m_tableModel->setItem(row, col, item);
        }
        updateTableRow(row);
    }

    // 设置列宽策略
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_VIDEO_FILE || colType == COL_AUDIO_FILE) {
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::Interactive);
            ui->MaintableView->setColumnWidth(col, 150);
            ui->MaintableView->horizontalHeader()->setMinimumSectionSize(100);
            ui->MaintableView->horizontalHeader()->setMaximumSectionSize(300);
        } else {
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::ResizeToContents);
        }
    }

    // 设置进度条委托
    int progressCol = m_columnManager.getVisualIndex(COL_PROGRESS);
    if (progressCol >= 0) {
        // 删除旧的委托
        QAbstractItemDelegate* oldDelegate = ui->MaintableView->itemDelegateForColumn(progressCol);
        if (oldDelegate) {
            ui->MaintableView->setItemDelegateForColumn(progressCol, nullptr);
            delete oldDelegate;
        }

        // 设置新的委托
        ui->MaintableView->setItemDelegateForColumn(
            progressCol,
            new ProgressBarDelegate(this)
            );
    }
}

void MainWindow::updateTableRow(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= m_videoItems.size() || rowIndex >= m_tableModel->rowCount())
        return;

    VideoItem* item = m_videoItems[rowIndex];
    for (int col = 0; col < m_tableModel->columnCount(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        QStandardItem* tableItem = m_tableModel->item(rowIndex, col);

        if (colType == COL_PROGRESS) {
            int progress = item->data(colType).toInt();
            tableItem->setData(progress, Qt::DisplayRole);
        } else {
            tableItem->setText(item->data(colType).toString());
        }
    }
}

void MainWindow::updateRowNumbers()
{
    for (int i = 0; i < m_videoItems.size(); ++i) {
        m_videoItems[i]->setIndex(i + 1);
        updateTableRow(i);
    }
}

void MainWindow::clearModelData()
{
    m_tableModel->clear();
    qDeleteAll(m_videoItems);
    m_videoItems.clear();
    updateTableHeaders();
}

// ===================== 主按钮功能 =====================
void MainWindow::on_singleline_importButton_clicked()
{
    singleline_import_dialog *dialog = new singleline_import_dialog(this);
    dialog->setWindowTitle(tr("导入单个视频"));

    // 连接信号
    connect(dialog, &singleline_import_dialog::importDataReady,
            this, &MainWindow::handleImportData);

    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_wholsoueflie_importButton_clicked()
{
    // TODO: 实现整个缓存源文件导入功能
}

void MainWindow::on_settingButton_clicked()
{
    Setting_Dialog dialog(&m_columnManager, this);
    dialog.setWindowTitle(tr("设置"));

    if (dialog.exec() == QDialog::Accepted) {
        updateTableHeaders();
    }
}

void MainWindow::on_addlineButton_clicked()
{
    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    m_videoItems.append(newItem);

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        QStandardItem* item = new QStandardItem();
        item->setTextAlignment(Qt::AlignCenter);

        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            item->setData(0, Qt::DisplayRole);
        } else if (colType == COL_TITLE) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText("<新项目>");
        } else {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号
    connect(newItem, &VideoItem::dataChanged, this, [this, newItem](){
        int row = m_videoItems.indexOf(newItem);
        if (row >= 0) updateTableRow(row);
    });

    updateRowNumbers();
}

void MainWindow::on_delelineButton_clicked()
{
    if (m_rememberDeleteChoice) {
        performDeleteOperation(m_deleteMode);
    } else {
        del_setting_dialog dialog(m_deleteMode, this);
        dialog.setWindowTitle(tr("设置删除模式"));

        if (dialog.exec() == QDialog::Accepted) {
            setDeleteSettings(dialog.getDeleteMode(), dialog.rememberChoice());
            performDeleteOperation(m_deleteMode);
        }
    }
}

// ===================== 删除操作管理 =====================
void MainWindow::performDeleteOperation(DeleteMode mode)
{
    if (mode == DeleteFirst) {
        // 删除第一行
        if (m_videoItems.size() > 0) {
            delete m_videoItems.takeAt(0);
            m_tableModel->removeRow(0);
            updateRowNumbers();
        }
    } else if (mode == DeleteSelected) {
        // 删除选中行
        QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
        if (selected.isEmpty()) return;

        // 倒序删除避免索引问题
        std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
            return a.row() > b.row();
        });

        for (const QModelIndex &index : selected) {
            int row = index.row();
            if (row < m_videoItems.size()) {
                delete m_videoItems.takeAt(row);
                m_tableModel->removeRow(row);
            }
        }
        updateRowNumbers();
    } else if (mode == DeleteAll) {
        // 删除所有行
        clearModelData();
    }
}

void MainWindow::setDeleteSettings(DeleteMode mode, bool remember)
{
    m_deleteMode = mode;
    m_rememberDeleteChoice = remember;

    // 保存设置
    QSettings settings;
    settings.setValue("delete/remember", remember);
    settings.setValue("delete/mode", static_cast<int>(mode));
}

// ===================== 上下文菜单处理 =====================
void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->MaintableView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);

    // ===================== 功能组 =====================
    QAction *properties = menu.addAction("属性");
    QAction *preview = menu.addAction("预览");
    QAction *openFolder = menu.addAction("打开所在文件夹");
    menu.addSeparator(); // 功能组后的分隔线

    // ===================== 删除组 =====================
    QAction *deleteCurrent = menu.addAction("删除该行");
    QAction *deleteSelected = menu.addAction("删除选中项");
    QAction *deleteAll = menu.addAction("删除所有行");
    menu.addSeparator(); // 删除组后的分隔线

    // ===================== 导入组 =====================
    // 修复列类型判断逻辑
    TableColumns column = COL_INDEX;
    if (index.column() >=0 && index.column() < m_currentColumnsOrder.size()) {
        column = m_currentColumnsOrder[index.column()];
    }

    QAction *importFile = menu.addAction("导入...");
    importFile->setData("import_file");
    importFile->setEnabled(column == COL_VIDEO_FILE || column == COL_AUDIO_FILE);

    QAction *importTitle = menu.addAction("导入标题文件夹");
    importTitle->setData("import_title");
    QAction *importSource = menu.addAction("导入缓存源文件");
    importSource->setData("import_source");
    menu.addSeparator(); // 导入组后的分隔线

    // ===================== 导出组 =====================
    QAction *exportSingle = menu.addAction("导出该项");
    QAction *exportSelected = menu.addAction("导出选中项");
    QAction *exportAll = menu.addAction("导出全部");
    menu.addSeparator(); // 导出组后的分隔线

    // 连接预览操作
    connect(preview, &QAction::triggered, this, [this, index]() {
        onPreviewAction(index.row());
    });

    // 连接删除操作（原代码）
    connect(deleteCurrent, &QAction::triggered, this, [this, index]() {
        int row = index.row();
        if (row >= 0 && row < m_videoItems.size()) {
            delete m_videoItems.takeAt(row);
            m_tableModel->removeRow(row);
            updateRowNumbers();
        }
    });

    connect(deleteSelected, &QAction::triggered, this, [this]() {
        performDeleteOperation(DeleteSelected);
    });

    connect(deleteAll, &QAction::triggered, this, [this]() {
        performDeleteOperation(DeleteAll);
    });

    // 连接导入操作（原代码）
    connect(&menu, &QMenu::triggered, this, &MainWindow::onCustomContextMenuAction);

    // 连接导出操作（新增）
    connect(exportSingle, &QAction::triggered, this, &MainWindow::onExportSingle);
    connect(exportSelected, &QAction::triggered, this, &MainWindow::onExportSelected);
    connect(exportAll, &QAction::triggered, this, &MainWindow::onExportAll);

    menu.exec(ui->MaintableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onPreviewAction(int row)
{
    if (row < 0 || row >= m_videoItems.size()) {
        QMessageBox::warning(this, "预览", "无效的行索引");
        return;
    }

    // 如果预览窗口已经存在，先删除
    if (m_playbackWidget) {
        delete m_playbackWidget;
        m_playbackWidget = nullptr;
    }

    // 创建新的预览窗口
    m_playbackWidget = new Playback_Widge(this);

    // 使用 data(COL_TITLE) 方法获取标题
    QString title = m_videoItems[row]->data(COL_TITLE).toString();
    m_playbackWidget->setWindowTitle("视频预览 - " + title);

    m_playbackWidget->setWindowFlags(Qt::Window);
    m_playbackWidget->show();

    // 这里可以添加代码将选中的视频项数据传递给预览窗口
    // 例如：m_playbackWidget->setVideoItem(m_videoItems[row]);
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


//打开文件浏览器，设置输出地址
void MainWindow::on_outputButton_clicked()
{
    // 打开文件夹选择对话框
    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择输出目录"),
        m_lastOutputPath,  // 使用上次的路径
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    // 检查是否选择了有效目录
    if (!outputDir.isEmpty()) {
        // 将选择的路径设置到输出地址编辑框
        ui->outputAdd_Edit->setText(outputDir);

        // 更新路径记忆
        m_lastOutputPath = outputDir;
        savePathSettings();
    }
}


// 在 mainwindow.cpp 中添加实现
void MainWindow::onExportSingle()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        if (row >= 0 && row < m_videoItems.size()) {
            // 清空现有队列，只添加当前项
            m_pendingItems.clear();
            m_processingItems.clear();
            m_pendingItems.append(m_videoItems[row]);

            // 开始处理
            m_exportInProgress = true;
            m_failedCount = 0;
            startMergingProcess();
        }
    } else {
        QMessageBox::information(this, "导出", "请先选择一个项目");
    }
}

void MainWindow::onExportSelected()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        // 清空现有队列，添加选中项
        m_pendingItems.clear();
        m_processingItems.clear();

        for (const QModelIndex &index : selected) {
            if (index.row() < m_videoItems.size()) {
                m_pendingItems.append(m_videoItems[index.row()]);
            }
        }

        // 开始处理
        m_exportInProgress = true;
        m_failedCount = 0;
        startMergingProcess();
    } else {
        QMessageBox::information(this, "导出", "请先选择要导出的项目");
    }
}

void MainWindow::onExportAll()
{
    exportAllItems();
}

void MainWindow::exportSingleItem(int row)
{
    // 清空现有队列，添加所有项
    m_pendingItems.clear();
    m_processingItems.clear();
    m_pendingItems = m_videoItems;

    // 开始处理
    m_exportInProgress = true;
    m_failedCount = 0;
    startMergingProcess();
}

void MainWindow::exportSelectedItems()
{
    // 导出选中项目的实现
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected) {
        int row = index.row();
        if (row >= 0 && row < m_videoItems.size()) {
            VideoItem* item = m_videoItems[row];
            // 实现导出逻辑
        }
    }
}

void MainWindow::exportAllItems()
{
    // 清空现有队列，添加所有项
    m_pendingItems.clear();
    m_processingItems.clear();
    m_pendingItems = m_videoItems;

    // 开始处理
    m_exportInProgress = true;
    m_failedCount = 0;
    startMergingProcess();
}

// 修改导出设置保存方法
void MainWindow::setExportSettings(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    // 保存设置
    QSettings settings;
    settings.setValue("export/remember", remember);
    settings.setValue("export/mode", static_cast<int>(mode));

    // 更新状态显示
    updateExportStatusDisplay();
}

// 修改状态显示更新方法
void MainWindow::updateExportStatusDisplay()
{
    QString status;
    switch(m_exportMode) {
    case ExportSingle:
        status = "当前模式: 导出第1项";
        break;
    case ExportSelected:
        status = "当前模式: 导出选中项";
        break;
    case ExportAll:
        status = "当前模式: 导出全部";
        break;
    }

    if (m_rememberExportChoice) {
        status += " (已记住选择)";
    }

    // 更新UI中的状态显示
    // 注意：需要在UI中添加一个QLabel来显示这个状态
    // ui->exportStatusLabel->setText(status);
}



// 添加执行导出操作的方法
void MainWindow::performExportOperation(ExportMode mode)
{
    switch(mode) {
    case ExportSingle:
        exportSingleItem(0); // 导出第一项
        break;
    case ExportSelected:
        exportSelectedItems();
        break;
    case ExportAll:
        exportAllItems();
        break;
    }
}

void MainWindow::on_mergeStartBtn_clicked()
{
    if (m_exportInProgress) return;
    m_exportInProgress = true;
    m_failedCount = 0;

    // 清空处理队列
    m_processingItems.clear();
    m_pendingItems.clear();

    // 根据导出模式确定要处理的项
    if (m_rememberExportChoice) {
        // 使用记住的导出模式
        switch(m_exportMode) {
        case ExportSingle:
            if (!m_videoItems.isEmpty()) {
                m_pendingItems.append(m_videoItems.first());
            }
            break;
        case ExportSelected:
        {
            QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
            for (const QModelIndex &index : selected) {
                if (index.row() < m_videoItems.size()) {
                    m_pendingItems.append(m_videoItems[index.row()]);
                }
            }
            break;
        }
        case ExportAll:
            m_pendingItems = m_videoItems;
            break;
        }
    } else {
        // 弹出导出设置对话框
        export_setting_dialog dialog(this, m_exportMode, m_rememberExportChoice);
        dialog.setWindowTitle(tr("生成模式设置"));

        if (dialog.exec() == QDialog::Accepted) {
            ExportMode newMode = dialog.getExportMode();
            bool remember = dialog.rememberChoice();

            setExportSettingsSessionOnly(newMode, remember);

            // 根据选择的模式添加项目到待处理列表
            switch(newMode) {
            case ExportSingle:
                if (!m_videoItems.isEmpty()) {
                    m_pendingItems.append(m_videoItems.first());
                }
                break;
            case ExportSelected:
            {
                QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
                for (const QModelIndex &index : selected) {
                    if (index.row() < m_videoItems.size()) {
                        m_pendingItems.append(m_videoItems[index.row()]);
                    }
                }
                break;
            }
            case ExportAll:
                m_pendingItems = m_videoItems;
                break;
            }
        } else {
            m_exportInProgress = false;
            return; // 用户取消
        }
    }

    // 开始处理
    startMergingProcess();
}

void MainWindow::startMergingProcess()
{
    qDebug() << "开始混流过程，待处理项目数:" << m_pendingItems.size();

    // 重置进度条
    ui->Total_progressBar->setValue(0);

    // 开始处理项目
    for (int i = 0; i < m_maxConcurrentProcesses && !m_pendingItems.isEmpty(); i++) {
        qDebug() << "启动处理第" << i+1 << "个项目";
        processNextItem();
    }

}

void MainWindow::processNextItem()
{
    qDebug() << "处理下一个项目，待处理队列大小:" << m_pendingItems.size();

    if (m_pendingItems.isEmpty()) {
        qDebug() << "待处理队列为空，返回";
        return;
    }

    VideoItem* item = m_pendingItems.takeFirst();
    m_processingItems.append(item);

    qDebug() << "开始处理项目:" << item->data(COL_TITLE).toString();

    // 检查文件是否存在
    if (!item->checkFilesExist()) {
        qDebug() << "文件不存在或文件大小为0，标记为失败";
        // 只有在之前没有错误的情况下才处理
        if (!item->hasError()) {
            m_failedCount++;
            item->setProgress(-1); // 这会设置错误状态
            item->setHasError(true); // 明确设置错误状态
            qDebug() << "失败计数增加，当前失败数:" << m_failedCount;
        }
        m_processingItems.removeOne(item);
        qDebug() << "从处理队列中移除项目";

        // 继续处理下一个
        if (!m_pendingItems.isEmpty()) {
            qDebug() << "继续处理下一个项目";
            processNextItem();
        } else if (m_processingItems.isEmpty()) {
            qDebug() << "所有项目处理完成，调用完成函数";
            finishMergingProcess();
        }
        return;
    }

    // 检查标题是否为空，为空则生成默认标题
    if (item->data(COL_TITLE).toString().isEmpty()) {
        QString defaultTitle = item->generateDefaultTitle();
        item->setTitle(defaultTitle);
        qDebug() << "生成了默认标题:" << defaultTitle;

        // 更新表格显示
        int row = m_videoItems.indexOf(item);
        if (row >= 0) updateTableRow(row);
    }

    // 开始FFmpeg处理
    qDebug() << "开始FFmpeg处理";
    startFFmpegForItem(item);
}

void MainWindow::startFFmpegForItem(VideoItem* item)
{
    qDebug() << "为项目启动FFmpeg:" << item->data(COL_TITLE).toString();

    // 1. 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();

    // 2. FFmpeg路径 - 直接使用ffmpeg.exe
    QString ffmpegExe = appDir + "/ffmpeg.exe";

    // 3. 验证FFmpeg存在
    if (!QFile::exists(ffmpegExe)) {
        // 如果构建目录中没有，尝试原始位置（用于调试）
        QString originalFfmpeg = QCoreApplication::applicationDirPath() + "/../ffmpeg/bin/ffmpeg.exe";

        if (QFile::exists(originalFfmpeg)) {
            ffmpegExe = originalFfmpeg;
        } else {
            qWarning() << "FFmpeg not found at:" << ffmpegExe;
            qWarning() << "Also checked original location:" << originalFfmpeg;

            // 只有在之前没有错误的情况下才处理
            if (!item->hasError()) {
                item->setProgress(-1);
                item->setHasError(true);
                m_processingItems.removeOne(item);
                m_failedCount++;

                QMessageBox::critical(this, "FFmpeg错误",
                                      QString("找不到FFmpeg可执行文件:\n%1\n%2")
                                          .arg(ffmpegExe)
                                          .arg("请确保ffmpeg.exe已正确放置在ffmpeg/bin目录下"));
            }

            return;
        }
    }

    // 4. 获取视频项数据
    QString videoPath = item->data(COL_VIDEO_FILE).toString();
    QString audioPath = item->data(COL_AUDIO_FILE).toString();
    QString outputPath = ui->outputAdd_Edit->text();
    QString title = item->data(COL_TITLE).toString();

    if (outputPath.isEmpty()) {
        // 只有在之前没有错误的情况下才处理
        if (!item->hasError()) {
            QMessageBox::warning(this, "输出错误", "请先设置输出目录");
            item->setProgress(-1);
            item->setHasError(true);
            m_processingItems.removeOne(item);
        }
        return;
    }

    // 5. 处理文件名中的非法字符
    QString safeTitle = title;
    QRegularExpression illegalChars(R"([\\/:*?"<>|])");
    safeTitle.replace(illegalChars, "_");

    // 6. 确保输出目录存在
    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qWarning() << "Failed to create output directory:" << outputPath;
            // 只有在之前没有处理过错误的情况下才标记失败
            if (item->progress() != -1) {
                item->setProgress(-1);
                m_processingItems.removeOne(item);
                m_failedCount++;
                QMessageBox::critical(this, "输出错误", "无法创建输出目录：" + outputPath);
            }
            return;
        }
    }

    // 7. 获取输出格式
    QString format = "mp4"; // 默认MP4格式

    // 8. 构建安全的输出文件路径
    QString outputFile = outputDir.filePath(safeTitle + "." + format);

    // 9. 创建FFmpeg进程
    QProcess* ffmpegProcess = new QProcess(this);
    ffmpegProcess->setProperty("videoItem", QVariant::fromValue<VideoItem*>(item));

    // 10. 构建FFmpeg命令
    QStringList args;

    // 添加输入文件（直接使用路径）
    if (!videoPath.isEmpty()) {
        args << "-i" << videoPath;
    }
    if (!audioPath.isEmpty()) {
        args << "-i" << audioPath;
    }

    // 设置流复制参数
    args << "-c:v" << "copy" << "-c:a" << "copy";

    // 根据格式设置容器
    if (format == "mp4") {
        args << "-f" << "mp4";
    } else if (format == "mkv") {
        args << "-f" << "matroska";
    } else if (format == "webm") {
        args << "-f" << "webm";
    } else if (format == "avi") {
        args << "-f" << "avi";
    }

    // 添加输出文件参数
    args << "-y";
    args << outputFile; // 直接使用输出路径

    // 11. 连接信号处理
    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardOutput();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardError();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    // 在进程完成信号处理中添加调试输出
    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ffmpegProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                if (item) {
                    qDebug() << "FFmpeg进程完成，退出码:" << exitCode << "退出状态:" << exitStatus;

                    // 只有在之前没有错误的情况下才处理
                    if (!item->hasError()) {
                        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                            qDebug() << "FFmpeg处理成功";
                            item->setProgress(100);
                        } else {
                            qDebug() << "FFmpeg处理失败";
                            item->setProgress(-1);
                            item->setHasError(true);
                            m_failedCount++;
                            qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                            QString errorOutput = ffmpegProcess->readAllStandardError();
                            qDebug() << "FFmpeg错误输出:" << errorOutput;

                            // 将错误输出保存到文件
                            QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                            if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                errorLog.write(QString("Exit code: %1\n").arg(exitCode).toUtf8());
                                errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                                errorLog.write("Error output:\n" + errorOutput.toUtf8() + "\n\n");
                                errorLog.close();
                            }
                        }
                    }

                    m_processingItems.removeOne(item);
                    qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                    ffmpegProcess->deleteLater();
                    updateTotalProgress();

                    if (!m_pendingItems.isEmpty()) {
                        qDebug() << "有待处理项目，继续处理下一个";
                        processNextItem();
                    } else if (m_processingItems.isEmpty()) {
                        qDebug() << "所有项目处理完成，调用完成函数";
                        finishMergingProcess();
                    }
                }
            });

    // 在进程错误信号处理中添加调试输出
    connect(ffmpegProcess, &QProcess::errorOccurred,
            this, [this, ffmpegProcess](QProcess::ProcessError error) {
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                qDebug() << "FFmpeg进程错误:" << error;

                // 只有在之前没有错误的情况下才处理
                if (item && !item->hasError()) {
                    // 只处理启动失败的情况，其他错误由finished信号处理
                    if (error == QProcess::FailedToStart) {
                        qDebug() << "FFmpeg启动失败";
                        item->setProgress(-1);
                        item->setHasError(true);
                        m_failedCount++;
                        qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                        QString errorStr;
                        switch(error) {
                        case QProcess::FailedToStart:
                            errorStr = "无法启动FFmpeg进程";
                            break;
                        default:
                            return;  // 其他错误类型由finished信号处理
                        }

                        qDebug() << errorStr;
                        QMessageBox::critical(this, "FFmpeg错误", errorStr);

                        // 保存错误信息
                        QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                        if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                            errorLog.write(QString("Error: %1\n").arg(errorStr).toUtf8());
                            errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                            errorLog.close();
                        }

                        m_processingItems.removeOne(item);
                        qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                        ffmpegProcess->deleteLater();
                        updateTotalProgress();

                        if (!m_pendingItems.isEmpty()) {
                            qDebug() << "有待处理项目，继续处理下一个";
                            processNextItem();
                        } else if (m_processingItems.isEmpty()) {
                            qDebug() << "所有项目处理完成，调用完成函数";
                            finishMergingProcess();
                        }
                    }
                }
            });


    // 14. 启动进程
    qDebug() << "Executing FFmpeg command:" << ffmpegExe << args;
    ffmpegProcess->start(ffmpegExe, args);

    // 15. 添加超时处理
    QTimer::singleShot(5 * 60 * 1000, this, [ffmpegProcess, this]() {
        if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
            qDebug() << "FFmpeg process timed out, terminating";
            ffmpegProcess->terminate();

            // 等待5秒强制终止
            QTimer::singleShot(5000, this, [ffmpegProcess, this]() {
                if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
                    qDebug() << "FFmpeg process still running, killing";
                    ffmpegProcess->kill();
                }
            });
        }
    });



}

void MainWindow::finishMergingProcess()
{
    qDebug() << "完成混流过程，总项目数:" << m_videoItems.size() << "失败数:" << m_failedCount;

    m_exportInProgress = false;

    // 重置所有项目的错误状态
    for (VideoItem* item : m_videoItems) {
        item->setHasError(false);
    }

    // 显示完成消息
    QString message = QString("混流完成! 成功: %1, 失败: %2")
                          .arg(m_videoItems.size() - m_failedCount)
                          .arg(m_failedCount);

    qDebug() << "显示完成消息:" << message;
    QMessageBox::information(this, "混流完成", message);
}

void MainWindow::parseFFmpegOutput(VideoItem* item, const QString& output)
{
    // 解析FFmpeg输出获取进度
    // 这里需要根据FFmpeg的实际输出格式进行解析
    // 示例代码，实际需要根据FFmpeg输出调整

    QRegularExpression timeRegex("time=(\\d+):(\\d+):(\\d+).(\\d+)");
    QRegularExpressionMatch match = timeRegex.match(output);

    if (match.hasMatch()) {
        int hours = match.captured(1).toInt();
        int minutes = match.captured(2).toInt();
        int seconds = match.captured(3).toInt();
        int ms = match.captured(4).toInt();

        int totalMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + ms;

        // 假设总时长已知（实际可能需要从文件元数据获取）
        // 这里简化处理，实际应用中需要更精确的进度计算
        int progress = qMin(100, totalMs / 10000); // 假设10秒视频

        item->setProgress(progress);
        updateTotalProgress();
    }
}


void MainWindow::updateTotalProgress()
{
    if (m_videoItems.isEmpty()) {
        ui->Total_progressBar->setValue(0);
        return;
    }

    int totalProgress = 0;
    int validItems = 0;

    for (VideoItem* item : m_videoItems) {
        int progress = item->progress();
        if (progress >= 0 && !item->hasError()) { // 只计算有效进度且没有错误的项目
            totalProgress += progress;
            validItems++;
        }
    }

    if (validItems > 0) {
        totalProgress /= validItems;
    }

    ui->Total_progressBar->setValue(totalProgress);
}



void MainWindow::setExportSettingsSessionOnly(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    // 注意：这里不保存到QSettings，只更新当前会话
    updateExportStatusDisplay();
}

// ===================== 处理导入数据 =====================
void MainWindow::handleImportData(const QString& videoPath, const QString& audioPath, const QString& title)
{
    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    newItem->setHasError(false); // 确保新项目没有错误状态
    m_videoItems.append(newItem);

    // 设置数据 - 使用 setData 方法而不是 setVideoFile/setAudioFile
    newItem->setTitle(title);
    if (!videoPath.isEmpty()) {
        newItem->setData(COL_VIDEO_FILE, videoPath);
    }
    if (!audioPath.isEmpty()) {
        newItem->setData(COL_AUDIO_FILE, audioPath);
    }

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        QStandardItem* item = new QStandardItem();
        item->setTextAlignment(Qt::AlignCenter);

        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            item->setData(0, Qt::DisplayRole);
        } else if (colType == COL_TITLE) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText(title);
        } else if (colType == COL_VIDEO_FILE && !videoPath.isEmpty()) {
            item->setText(videoPath);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        } else if (colType == COL_AUDIO_FILE && !audioPath.isEmpty()) {
            item->setText(audioPath);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        } else {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号
    connect(newItem, &VideoItem::dataChanged, this, [this, newItem](){
        int row = m_videoItems.indexOf(newItem);
        if (row >= 0) updateTableRow(row);
    });

    updateRowNumbers();

    // 调用FFmpeg进行预加载操作
    if (!videoPath.isEmpty() || !audioPath.isEmpty()) {
        QProcess* ffmpegProcess = new QProcess(this);
        QStringList args;

        // 构建FFmpeg命令来获取媒体信息
        if (!videoPath.isEmpty()) {
            args << "-i" << videoPath;
        }
        if (!audioPath.isEmpty() && videoPath != audioPath) {
            if (args.isEmpty()) {
                args << "-i" << audioPath;
            } else {
                args << "-i" << audioPath;
            }
        }

        // 只获取信息，不进行实际处理
        args << "-f" << "null" << "-";

        connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [ffmpegProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    ffmpegProcess->deleteLater();
                });

        ffmpegProcess->start("ffmpeg", args);
    }
}

void MainWindow::initPathMemory()
{
    // 加载保存的路径
    QSettings settings;
    m_lastOutputPath = settings.value("LastOutputPath",
                                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();

    // 设置到UI
    ui->outputAdd_Edit->setText(m_lastOutputPath);

    // 如果没有保存的路径，使用默认路径
    if (m_lastVideoPath.isEmpty()) m_lastVideoPath = QDir::homePath();
    if (m_lastAudioPath.isEmpty()) m_lastAudioPath = QDir::homePath();
    if (m_lastOutputPath.isEmpty()) m_lastOutputPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (m_lastTitleFolderPath.isEmpty()) m_lastTitleFolderPath = QDir::homePath();
}

void MainWindow::loadPathSettings()
{
    QSettings settings;
    m_lastVideoPath = settings.value("Last/VideoPath", QDir::homePath()).toString();
    m_lastAudioPath = settings.value("Last/AudioPath", QDir::homePath()).toString();
    m_lastOutputPath = settings.value("Last/OutputPath",
                                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    m_lastTitleFolderPath = settings.value("Last/TitleFolderPath", QDir::homePath()).toString();
}

void MainWindow::savePathSettings()
{
    QSettings settings;
    settings.setValue("Last/VideoPath", m_lastVideoPath);
    settings.setValue("Last/AudioPath", m_lastAudioPath);
    settings.setValue("Last/OutputPath", m_lastOutputPath);
    settings.setValue("Last/TitleFolderPath", m_lastTitleFolderPath);
}
=======
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QProgressBar>
#include <QSettings>
#include <Qmessagebox>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include "delegates/progressbardelegate.h"
#include "dialogs/setting_dialog.h"
#include "dialogs/singleline_import_dialog.h"
#include "dialogs/del_setting_dialog.h"
#include "dialogs/export_setting_dialog.h" // 添加包含

// ===================== 构造函数/析构函数 =====================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Memoria V2.2.8");

    // 初始化路径记忆
    initPathMemory();

    // 初始化表格
    initTableView();

    // 设置上下文菜单
    setupContextMenu();

    // 设置进度条委托
    m_progressDelegate = new QProgressBar(this);
    m_progressDelegate->setRange(0, 100);
    m_progressDelegate->setTextVisible(false);

    // 读取保存的删除设置
    QSettings settings;
    m_rememberDeleteChoice = settings.value("delete/remember", false).toBool();
    m_deleteMode = static_cast<DeleteMode>(settings.value("delete/mode", DeleteFirst).toInt());

    // 读取保存的导出设置
    m_rememberExportChoice = settings.value("export/remember", false).toBool();
    m_exportMode = static_cast<ExportMode>(settings.value("export/mode", ExportSingle).toInt());

    // 更新状态显示
    updateExportStatusDisplay();


}

MainWindow::~MainWindow()
{
    qDeleteAll(m_videoItems);
    delete ui;
}


// ===================== 初始化函数 =====================
void MainWindow::initTableView()
{
    // 设置表格模型
    ui->MaintableView->setModel(m_tableModel);

    // 初始化表头
    updateTableHeaders();

    // 添加交替行颜色
    ui->MaintableView->setAlternatingRowColors(true);

    // 设置表格属性
    ui->MaintableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->MaintableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->MaintableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->MaintableView->verticalHeader()->setVisible(false);
    // 修改后：仅允许双击编辑
    ui->MaintableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // 设置行选择样式
    QString style = "QTableView::item:selected {"
                    "    background-color: #FFFACD;"
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
}

void MainWindow::setupContextMenu()
{
    ui->MaintableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->MaintableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}

// ===================== 表格数据处理函数 =====================
void MainWindow::updateTableHeaders()
{
    // 保存当前行数和列数
    int rowCount = m_tableModel->rowCount();
    int oldColumnCount = m_tableModel->columnCount();

    // 获取可见列标题
    QStringList headers = m_columnManager.getVisibleHeaders();

    // 更新当前列的顺序列表
    m_currentColumnsOrder.clear();
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        TableColumns column = static_cast<TableColumns>(i);
        if (m_columnManager.isColumnVisible(column)) {
            m_currentColumnsOrder.append(column);
        }
    }

    // 设置新表头
    m_tableModel->setColumnCount(headers.size());
    m_tableModel->setHorizontalHeaderLabels(headers);

    // 添加新列（如果列数增加）
    for (int row = 0; row < rowCount; ++row) {
        for (int col = oldColumnCount; col < headers.size(); ++col) {
            QStandardItem* item = new QStandardItem();
            TableColumns colType = m_currentColumnsOrder[col];

            if (colType == COL_PROGRESS) {
                item->setData(0, Qt::DisplayRole);
            } else if (colType == COL_TITLE) {
                item->setFlags(item->flags() | Qt::ItemIsEditable);
            } else {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }

            m_tableModel->setItem(row, col, item);
        }
        updateTableRow(row);
    }

    // 设置列宽策略
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_VIDEO_FILE || colType == COL_AUDIO_FILE) {
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::Interactive);
            ui->MaintableView->setColumnWidth(col, 150);
            ui->MaintableView->horizontalHeader()->setMinimumSectionSize(100);
            ui->MaintableView->horizontalHeader()->setMaximumSectionSize(300);
        } else {
            ui->MaintableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::ResizeToContents);
        }
    }

    // 设置进度条委托
    int progressCol = m_columnManager.getVisualIndex(COL_PROGRESS);
    if (progressCol >= 0) {
        // 删除旧的委托
        QAbstractItemDelegate* oldDelegate = ui->MaintableView->itemDelegateForColumn(progressCol);
        if (oldDelegate) {
            ui->MaintableView->setItemDelegateForColumn(progressCol, nullptr);
            delete oldDelegate;
        }

        // 设置新的委托
        ui->MaintableView->setItemDelegateForColumn(
            progressCol,
            new ProgressBarDelegate(this)
            );
    }
}

void MainWindow::updateTableRow(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= m_videoItems.size() || rowIndex >= m_tableModel->rowCount())
        return;

    VideoItem* item = m_videoItems[rowIndex];
    for (int col = 0; col < m_tableModel->columnCount(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        QStandardItem* tableItem = m_tableModel->item(rowIndex, col);

        if (colType == COL_PROGRESS) {
            int progress = item->data(colType).toInt();
            tableItem->setData(progress, Qt::DisplayRole);
        } else {
            tableItem->setText(item->data(colType).toString());
        }
    }
}

void MainWindow::updateRowNumbers()
{
    for (int i = 0; i < m_videoItems.size(); ++i) {
        m_videoItems[i]->setIndex(i + 1);
        updateTableRow(i);
    }
}

void MainWindow::clearModelData()
{
    m_tableModel->clear();
    qDeleteAll(m_videoItems);
    m_videoItems.clear();
    updateTableHeaders();
}

// ===================== 主按钮功能 =====================
void MainWindow::on_singleline_importButton_clicked()
{
    singleline_import_dialog *dialog = new singleline_import_dialog(this);
    dialog->setWindowTitle(tr("导入单个视频"));

    // 连接信号
    connect(dialog, &singleline_import_dialog::importDataReady,
            this, &MainWindow::handleImportData);

    dialog->exec();
    dialog->deleteLater();
}

void MainWindow::on_wholsoueflie_importButton_clicked()
{
    // TODO: 实现整个缓存源文件导入功能
}

void MainWindow::on_settingButton_clicked()
{
    Setting_Dialog dialog(&m_columnManager, this);
    dialog.setWindowTitle(tr("设置"));

    if (dialog.exec() == QDialog::Accepted) {
        updateTableHeaders();
    }
}

void MainWindow::on_addlineButton_clicked()
{
    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    m_videoItems.append(newItem);

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        QStandardItem* item = new QStandardItem();
        item->setTextAlignment(Qt::AlignCenter);

        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            item->setData(0, Qt::DisplayRole);
        } else if (colType == COL_TITLE) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText("<新项目>");
        } else {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号
    connect(newItem, &VideoItem::dataChanged, this, [this, newItem](){
        int row = m_videoItems.indexOf(newItem);
        if (row >= 0) updateTableRow(row);
    });

    updateRowNumbers();
}

void MainWindow::on_delelineButton_clicked()
{
    if (m_rememberDeleteChoice) {
        performDeleteOperation(m_deleteMode);
    } else {
        del_setting_dialog dialog(m_deleteMode, this);
        dialog.setWindowTitle(tr("设置删除模式"));

        if (dialog.exec() == QDialog::Accepted) {
            setDeleteSettings(dialog.getDeleteMode(), dialog.rememberChoice());
            performDeleteOperation(m_deleteMode);
        }
    }
}

// ===================== 删除操作管理 =====================
void MainWindow::performDeleteOperation(DeleteMode mode)
{
    if (mode == DeleteFirst) {
        // 删除第一行
        if (m_videoItems.size() > 0) {
            delete m_videoItems.takeAt(0);
            m_tableModel->removeRow(0);
            updateRowNumbers();
        }
    } else if (mode == DeleteSelected) {
        // 删除选中行
        QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
        if (selected.isEmpty()) return;

        // 倒序删除避免索引问题
        std::sort(selected.begin(), selected.end(), [](const QModelIndex &a, const QModelIndex &b) {
            return a.row() > b.row();
        });

        for (const QModelIndex &index : selected) {
            int row = index.row();
            if (row < m_videoItems.size()) {
                delete m_videoItems.takeAt(row);
                m_tableModel->removeRow(row);
            }
        }
        updateRowNumbers();
    } else if (mode == DeleteAll) {
        // 删除所有行
        clearModelData();
    }
}

void MainWindow::setDeleteSettings(DeleteMode mode, bool remember)
{
    m_deleteMode = mode;
    m_rememberDeleteChoice = remember;

    // 保存设置
    QSettings settings;
    settings.setValue("delete/remember", remember);
    settings.setValue("delete/mode", static_cast<int>(mode));
}

// ===================== 上下文菜单处理 =====================
void MainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->MaintableView->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);

    // ===================== 功能组 =====================
    QAction *properties = menu.addAction("属性");
    QAction *preview = menu.addAction("预览");
    QAction *openFolder = menu.addAction("打开所在文件夹");
    menu.addSeparator(); // 功能组后的分隔线

    // ===================== 删除组 =====================
    QAction *deleteCurrent = menu.addAction("删除该行");
    QAction *deleteSelected = menu.addAction("删除选中项");
    QAction *deleteAll = menu.addAction("删除所有行");
    menu.addSeparator(); // 删除组后的分隔线

    // ===================== 导入组 =====================
    // 修复列类型判断逻辑
    TableColumns column = COL_INDEX;
    if (index.column() >=0 && index.column() < m_currentColumnsOrder.size()) {
        column = m_currentColumnsOrder[index.column()];
    }

    QAction *importFile = menu.addAction("导入...");
    importFile->setData("import_file");
    importFile->setEnabled(column == COL_VIDEO_FILE || column == COL_AUDIO_FILE);

    QAction *importTitle = menu.addAction("导入标题文件夹");
    importTitle->setData("import_title");
    QAction *importSource = menu.addAction("导入缓存源文件");
    importSource->setData("import_source");
    menu.addSeparator(); // 导入组后的分隔线

    // ===================== 导出组 =====================
    QAction *exportSingle = menu.addAction("导出该项");
    QAction *exportSelected = menu.addAction("导出选中项");
    QAction *exportAll = menu.addAction("导出全部");
    menu.addSeparator(); // 导出组后的分隔线

    // 连接预览操作
    connect(preview, &QAction::triggered, this, [this, index]() {
        onPreviewAction(index.row());
    });

    // 连接删除操作（原代码）
    connect(deleteCurrent, &QAction::triggered, this, [this, index]() {
        int row = index.row();
        if (row >= 0 && row < m_videoItems.size()) {
            delete m_videoItems.takeAt(row);
            m_tableModel->removeRow(row);
            updateRowNumbers();
        }
    });

    connect(deleteSelected, &QAction::triggered, this, [this]() {
        performDeleteOperation(DeleteSelected);
    });

    connect(deleteAll, &QAction::triggered, this, [this]() {
        performDeleteOperation(DeleteAll);
    });

    // 连接导入操作（原代码）
    connect(&menu, &QMenu::triggered, this, &MainWindow::onCustomContextMenuAction);

    // 连接导出操作（新增）
    connect(exportSingle, &QAction::triggered, this, &MainWindow::onExportSingle);
    connect(exportSelected, &QAction::triggered, this, &MainWindow::onExportSelected);
    connect(exportAll, &QAction::triggered, this, &MainWindow::onExportAll);

    menu.exec(ui->MaintableView->viewport()->mapToGlobal(pos));
}

void MainWindow::onPreviewAction(int row)
{
    if (row < 0 || row >= m_videoItems.size()) {
        QMessageBox::warning(this, "预览", "无效的行索引");
        return;
    }

    // 如果预览窗口已经存在，先删除
    if (m_playbackWidget) {
        delete m_playbackWidget;
        m_playbackWidget = nullptr;
    }

    // 创建新的预览窗口
    m_playbackWidget = new Playback_Widge(this);

    // 使用 data(COL_TITLE) 方法获取标题
    QString title = m_videoItems[row]->data(COL_TITLE).toString();
    m_playbackWidget->setWindowTitle("视频预览 - " + title);

    m_playbackWidget->setWindowFlags(Qt::Window);
    m_playbackWidget->show();

    // 这里可以添加代码将选中的视频项数据传递给预览窗口
    // 例如：m_playbackWidget->setVideoItem(m_videoItems[row]);
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


//打开文件浏览器，设置输出地址
void MainWindow::on_outputButton_clicked()
{
    // 打开文件夹选择对话框
    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择输出目录"),
        m_lastOutputPath,  // 使用上次的路径
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    // 检查是否选择了有效目录
    if (!outputDir.isEmpty()) {
        // 将选择的路径设置到输出地址编辑框
        ui->outputAdd_Edit->setText(outputDir);

        // 更新路径记忆
        m_lastOutputPath = outputDir;
        savePathSettings();
    }
}


// 在 mainwindow.cpp 中添加实现
void MainWindow::onExportSingle()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        if (row >= 0 && row < m_videoItems.size()) {
            // 清空现有队列，只添加当前项
            m_pendingItems.clear();
            m_processingItems.clear();
            m_pendingItems.append(m_videoItems[row]);

            // 开始处理
            m_exportInProgress = true;
            m_failedCount = 0;
            startMergingProcess();
        }
    } else {
        QMessageBox::information(this, "导出", "请先选择一个项目");
    }
}

void MainWindow::onExportSelected()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        // 清空现有队列，添加选中项
        m_pendingItems.clear();
        m_processingItems.clear();

        for (const QModelIndex &index : selected) {
            if (index.row() < m_videoItems.size()) {
                m_pendingItems.append(m_videoItems[index.row()]);
            }
        }

        // 开始处理
        m_exportInProgress = true;
        m_failedCount = 0;
        startMergingProcess();
    } else {
        QMessageBox::information(this, "导出", "请先选择要导出的项目");
    }
}

void MainWindow::onExportAll()
{
    exportAllItems();
}

void MainWindow::exportSingleItem(int row)
{
    // 清空现有队列，添加所有项
    m_pendingItems.clear();
    m_processingItems.clear();
    m_pendingItems = m_videoItems;

    // 开始处理
    m_exportInProgress = true;
    m_failedCount = 0;
    startMergingProcess();
}

void MainWindow::exportSelectedItems()
{
    // 导出选中项目的实现
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    for (const QModelIndex &index : selected) {
        int row = index.row();
        if (row >= 0 && row < m_videoItems.size()) {
            VideoItem* item = m_videoItems[row];
            // 实现导出逻辑
        }
    }
}

void MainWindow::exportAllItems()
{
    // 清空现有队列，添加所有项
    m_pendingItems.clear();
    m_processingItems.clear();
    m_pendingItems = m_videoItems;

    // 开始处理
    m_exportInProgress = true;
    m_failedCount = 0;
    startMergingProcess();
}

// 修改导出设置保存方法
void MainWindow::setExportSettings(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    // 保存设置
    QSettings settings;
    settings.setValue("export/remember", remember);
    settings.setValue("export/mode", static_cast<int>(mode));

    // 更新状态显示
    updateExportStatusDisplay();
}

// 修改状态显示更新方法
void MainWindow::updateExportStatusDisplay()
{
    QString status;
    switch(m_exportMode) {
    case ExportSingle:
        status = "当前模式: 导出第1项";
        break;
    case ExportSelected:
        status = "当前模式: 导出选中项";
        break;
    case ExportAll:
        status = "当前模式: 导出全部";
        break;
    }

    if (m_rememberExportChoice) {
        status += " (已记住选择)";
    }

    // 更新UI中的状态显示
    // 注意：需要在UI中添加一个QLabel来显示这个状态
    // ui->exportStatusLabel->setText(status);
}



// 添加执行导出操作的方法
void MainWindow::performExportOperation(ExportMode mode)
{
    switch(mode) {
    case ExportSingle:
        exportSingleItem(0); // 导出第一项
        break;
    case ExportSelected:
        exportSelectedItems();
        break;
    case ExportAll:
        exportAllItems();
        break;
    }
}

void MainWindow::on_mergeStartBtn_clicked()
{
    if (m_exportInProgress) return;
    m_exportInProgress = true;
    m_failedCount = 0;

    // 清空处理队列
    m_processingItems.clear();
    m_pendingItems.clear();

    // 根据导出模式确定要处理的项
    if (m_rememberExportChoice) {
        // 使用记住的导出模式
        switch(m_exportMode) {
        case ExportSingle:
            if (!m_videoItems.isEmpty()) {
                m_pendingItems.append(m_videoItems.first());
            }
            break;
        case ExportSelected:
        {
            QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
            for (const QModelIndex &index : selected) {
                if (index.row() < m_videoItems.size()) {
                    m_pendingItems.append(m_videoItems[index.row()]);
                }
            }
            break;
        }
        case ExportAll:
            m_pendingItems = m_videoItems;
            break;
        }
    } else {
        // 弹出导出设置对话框
        export_setting_dialog dialog(this, m_exportMode, m_rememberExportChoice);
        dialog.setWindowTitle(tr("生成模式设置"));

        if (dialog.exec() == QDialog::Accepted) {
            ExportMode newMode = dialog.getExportMode();
            bool remember = dialog.rememberChoice();

            setExportSettingsSessionOnly(newMode, remember);

            // 根据选择的模式添加项目到待处理列表
            switch(newMode) {
            case ExportSingle:
                if (!m_videoItems.isEmpty()) {
                    m_pendingItems.append(m_videoItems.first());
                }
                break;
            case ExportSelected:
            {
                QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
                for (const QModelIndex &index : selected) {
                    if (index.row() < m_videoItems.size()) {
                        m_pendingItems.append(m_videoItems[index.row()]);
                    }
                }
                break;
            }
            case ExportAll:
                m_pendingItems = m_videoItems;
                break;
            }
        } else {
            m_exportInProgress = false;
            return; // 用户取消
        }
    }

    // 开始处理
    startMergingProcess();
}

void MainWindow::startMergingProcess()
{
    qDebug() << "开始混流过程，待处理项目数:" << m_pendingItems.size();

    // 重置进度条
    ui->Total_progressBar->setValue(0);

    // 开始处理项目
    for (int i = 0; i < m_maxConcurrentProcesses && !m_pendingItems.isEmpty(); i++) {
        qDebug() << "启动处理第" << i+1 << "个项目";
        processNextItem();
    }

}

void MainWindow::processNextItem()
{
    qDebug() << "处理下一个项目，待处理队列大小:" << m_pendingItems.size();

    if (m_pendingItems.isEmpty()) {
        qDebug() << "待处理队列为空，返回";
        return;
    }

    VideoItem* item = m_pendingItems.takeFirst();
    m_processingItems.append(item);

    qDebug() << "开始处理项目:" << item->data(COL_TITLE).toString();

    // 检查文件是否存在
    if (!item->checkFilesExist()) {
        qDebug() << "文件不存在或文件大小为0，标记为失败";
        // 只有在之前没有错误的情况下才处理
        if (!item->hasError()) {
            m_failedCount++;
            item->setProgress(-1); // 这会设置错误状态
            item->setHasError(true); // 明确设置错误状态
            qDebug() << "失败计数增加，当前失败数:" << m_failedCount;
        }
        m_processingItems.removeOne(item);
        qDebug() << "从处理队列中移除项目";

        // 继续处理下一个
        if (!m_pendingItems.isEmpty()) {
            qDebug() << "继续处理下一个项目";
            processNextItem();
        } else if (m_processingItems.isEmpty()) {
            qDebug() << "所有项目处理完成，调用完成函数";
            finishMergingProcess();
        }
        return;
    }

    // 检查标题是否为空，为空则生成默认标题
    if (item->data(COL_TITLE).toString().isEmpty()) {
        QString defaultTitle = item->generateDefaultTitle();
        item->setTitle(defaultTitle);
        qDebug() << "生成了默认标题:" << defaultTitle;

        // 更新表格显示
        int row = m_videoItems.indexOf(item);
        if (row >= 0) updateTableRow(row);
    }

    // 开始FFmpeg处理
    qDebug() << "开始FFmpeg处理";
    startFFmpegForItem(item);
}

void MainWindow::startFFmpegForItem(VideoItem* item)
{
    qDebug() << "为项目启动FFmpeg:" << item->data(COL_TITLE).toString();

    // 1. 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();

    // 2. FFmpeg路径 - 直接使用ffmpeg.exe
    QString ffmpegExe = appDir + "/ffmpeg.exe";

    // 3. 验证FFmpeg存在
    if (!QFile::exists(ffmpegExe)) {
        // 如果构建目录中没有，尝试原始位置（用于调试）
        QString originalFfmpeg = QCoreApplication::applicationDirPath() + "/../ffmpeg/bin/ffmpeg.exe";

        if (QFile::exists(originalFfmpeg)) {
            ffmpegExe = originalFfmpeg;
        } else {
            qWarning() << "FFmpeg not found at:" << ffmpegExe;
            qWarning() << "Also checked original location:" << originalFfmpeg;

            // 只有在之前没有错误的情况下才处理
            if (!item->hasError()) {
                item->setProgress(-1);
                item->setHasError(true);
                m_processingItems.removeOne(item);
                m_failedCount++;

                QMessageBox::critical(this, "FFmpeg错误",
                                      QString("找不到FFmpeg可执行文件:\n%1\n%2")
                                          .arg(ffmpegExe)
                                          .arg("请确保ffmpeg.exe已正确放置在ffmpeg/bin目录下"));
            }

            return;
        }
    }

    // 4. 获取视频项数据
    QString videoPath = item->data(COL_VIDEO_FILE).toString();
    QString audioPath = item->data(COL_AUDIO_FILE).toString();
    QString outputPath = ui->outputAdd_Edit->text();
    QString title = item->data(COL_TITLE).toString();

    if (outputPath.isEmpty()) {
        // 只有在之前没有错误的情况下才处理
        if (!item->hasError()) {
            QMessageBox::warning(this, "输出错误", "请先设置输出目录");
            item->setProgress(-1);
            item->setHasError(true);
            m_processingItems.removeOne(item);
        }
        return;
    }

    // 5. 处理文件名中的非法字符
    QString safeTitle = title;
    QRegularExpression illegalChars(R"([\\/:*?"<>|])");
    safeTitle.replace(illegalChars, "_");

    // 6. 确保输出目录存在
    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qWarning() << "Failed to create output directory:" << outputPath;
            // 只有在之前没有处理过错误的情况下才标记失败
            if (item->progress() != -1) {
                item->setProgress(-1);
                m_processingItems.removeOne(item);
                m_failedCount++;
                QMessageBox::critical(this, "输出错误", "无法创建输出目录：" + outputPath);
            }
            return;
        }
    }

    // 7. 获取输出格式
    QString format = "mp4"; // 默认MP4格式

    // 8. 构建安全的输出文件路径
    QString outputFile = outputDir.filePath(safeTitle + "." + format);

    // 9. 创建FFmpeg进程
    QProcess* ffmpegProcess = new QProcess(this);
    ffmpegProcess->setProperty("videoItem", QVariant::fromValue<VideoItem*>(item));

    // 10. 构建FFmpeg命令
    QStringList args;

    // 添加输入文件（直接使用路径）
    if (!videoPath.isEmpty()) {
        args << "-i" << videoPath;
    }
    if (!audioPath.isEmpty()) {
        args << "-i" << audioPath;
    }

    // 设置流复制参数
    args << "-c:v" << "copy" << "-c:a" << "copy";

    // 根据格式设置容器
    if (format == "mp4") {
        args << "-f" << "mp4";
    } else if (format == "mkv") {
        args << "-f" << "matroska";
    } else if (format == "webm") {
        args << "-f" << "webm";
    } else if (format == "avi") {
        args << "-f" << "avi";
    }

    // 添加输出文件参数
    args << "-y";
    args << outputFile; // 直接使用输出路径

    // 11. 连接信号处理
    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardOutput();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardError();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    // 在进程完成信号处理中添加调试输出
    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ffmpegProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                if (item) {
                    qDebug() << "FFmpeg进程完成，退出码:" << exitCode << "退出状态:" << exitStatus;

                    // 只有在之前没有错误的情况下才处理
                    if (!item->hasError()) {
                        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                            qDebug() << "FFmpeg处理成功";
                            item->setProgress(100);
                        } else {
                            qDebug() << "FFmpeg处理失败";
                            item->setProgress(-1);
                            item->setHasError(true);
                            m_failedCount++;
                            qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                            QString errorOutput = ffmpegProcess->readAllStandardError();
                            qDebug() << "FFmpeg错误输出:" << errorOutput;

                            // 将错误输出保存到文件
                            QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                            if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                errorLog.write(QString("Exit code: %1\n").arg(exitCode).toUtf8());
                                errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                                errorLog.write("Error output:\n" + errorOutput.toUtf8() + "\n\n");
                                errorLog.close();
                            }
                        }
                    }

                    m_processingItems.removeOne(item);
                    qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                    ffmpegProcess->deleteLater();
                    updateTotalProgress();

                    if (!m_pendingItems.isEmpty()) {
                        qDebug() << "有待处理项目，继续处理下一个";
                        processNextItem();
                    } else if (m_processingItems.isEmpty()) {
                        qDebug() << "所有项目处理完成，调用完成函数";
                        finishMergingProcess();
                    }
                }
            });

    // 在进程错误信号处理中添加调试输出
    connect(ffmpegProcess, &QProcess::errorOccurred,
            this, [this, ffmpegProcess](QProcess::ProcessError error) {
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                qDebug() << "FFmpeg进程错误:" << error;

                // 只有在之前没有错误的情况下才处理
                if (item && !item->hasError()) {
                    // 只处理启动失败的情况，其他错误由finished信号处理
                    if (error == QProcess::FailedToStart) {
                        qDebug() << "FFmpeg启动失败";
                        item->setProgress(-1);
                        item->setHasError(true);
                        m_failedCount++;
                        qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                        QString errorStr;
                        switch(error) {
                        case QProcess::FailedToStart:
                            errorStr = "无法启动FFmpeg进程";
                            break;
                        default:
                            return;  // 其他错误类型由finished信号处理
                        }

                        qDebug() << errorStr;
                        QMessageBox::critical(this, "FFmpeg错误", errorStr);

                        // 保存错误信息
                        QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                        if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                            errorLog.write(QString("Error: %1\n").arg(errorStr).toUtf8());
                            errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                            errorLog.close();
                        }

                        m_processingItems.removeOne(item);
                        qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                        ffmpegProcess->deleteLater();
                        updateTotalProgress();

                        if (!m_pendingItems.isEmpty()) {
                            qDebug() << "有待处理项目，继续处理下一个";
                            processNextItem();
                        } else if (m_processingItems.isEmpty()) {
                            qDebug() << "所有项目处理完成，调用完成函数";
                            finishMergingProcess();
                        }
                    }
                }
            });


    // 14. 启动进程
    qDebug() << "Executing FFmpeg command:" << ffmpegExe << args;
    ffmpegProcess->start(ffmpegExe, args);

    // 15. 添加超时处理
    QTimer::singleShot(5 * 60 * 1000, this, [ffmpegProcess, this]() {
        if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
            qDebug() << "FFmpeg process timed out, terminating";
            ffmpegProcess->terminate();

            // 等待5秒强制终止
            QTimer::singleShot(5000, this, [ffmpegProcess, this]() {
                if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
                    qDebug() << "FFmpeg process still running, killing";
                    ffmpegProcess->kill();
                }
            });
        }
    });



}

void MainWindow::finishMergingProcess()
{
    qDebug() << "完成混流过程，总项目数:" << m_videoItems.size() << "失败数:" << m_failedCount;

    m_exportInProgress = false;

    // 重置所有项目的错误状态
    for (VideoItem* item : m_videoItems) {
        item->setHasError(false);
    }

    // 显示完成消息
    QString message = QString("混流完成! 成功: %1, 失败: %2")
                          .arg(m_videoItems.size() - m_failedCount)
                          .arg(m_failedCount);

    qDebug() << "显示完成消息:" << message;
    QMessageBox::information(this, "混流完成", message);
}

void MainWindow::parseFFmpegOutput(VideoItem* item, const QString& output)
{
    // 解析FFmpeg输出获取进度
    // 这里需要根据FFmpeg的实际输出格式进行解析
    // 示例代码，实际需要根据FFmpeg输出调整

    QRegularExpression timeRegex("time=(\\d+):(\\d+):(\\d+).(\\d+)");
    QRegularExpressionMatch match = timeRegex.match(output);

    if (match.hasMatch()) {
        int hours = match.captured(1).toInt();
        int minutes = match.captured(2).toInt();
        int seconds = match.captured(3).toInt();
        int ms = match.captured(4).toInt();

        int totalMs = (hours * 3600 + minutes * 60 + seconds) * 1000 + ms;

        // 假设总时长已知（实际可能需要从文件元数据获取）
        // 这里简化处理，实际应用中需要更精确的进度计算
        int progress = qMin(100, totalMs / 10000); // 假设10秒视频

        item->setProgress(progress);
        updateTotalProgress();
    }
}


void MainWindow::updateTotalProgress()
{
    if (m_videoItems.isEmpty()) {
        ui->Total_progressBar->setValue(0);
        return;
    }

    int totalProgress = 0;
    int validItems = 0;

    for (VideoItem* item : m_videoItems) {
        int progress = item->progress();
        if (progress >= 0 && !item->hasError()) { // 只计算有效进度且没有错误的项目
            totalProgress += progress;
            validItems++;
        }
    }

    if (validItems > 0) {
        totalProgress /= validItems;
    }

    ui->Total_progressBar->setValue(totalProgress);
}



void MainWindow::setExportSettingsSessionOnly(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    // 注意：这里不保存到QSettings，只更新当前会话
    updateExportStatusDisplay();
}

// ===================== 处理导入数据 =====================
void MainWindow::handleImportData(const QString& videoPath, const QString& audioPath, const QString& title)
{
    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    newItem->setHasError(false); // 确保新项目没有错误状态
    m_videoItems.append(newItem);

    // 设置数据 - 使用 setData 方法而不是 setVideoFile/setAudioFile
    newItem->setTitle(title);
    if (!videoPath.isEmpty()) {
        newItem->setData(COL_VIDEO_FILE, videoPath);
    }
    if (!audioPath.isEmpty()) {
        newItem->setData(COL_AUDIO_FILE, audioPath);
    }

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        QStandardItem* item = new QStandardItem();
        item->setTextAlignment(Qt::AlignCenter);

        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            item->setData(0, Qt::DisplayRole);
        } else if (colType == COL_TITLE) {
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            item->setText(title);
        } else if (colType == COL_VIDEO_FILE && !videoPath.isEmpty()) {
            item->setText(videoPath);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        } else if (colType == COL_AUDIO_FILE && !audioPath.isEmpty()) {
            item->setText(audioPath);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        } else {
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }

        rowItems.append(item);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号
    connect(newItem, &VideoItem::dataChanged, this, [this, newItem](){
        int row = m_videoItems.indexOf(newItem);
        if (row >= 0) updateTableRow(row);
    });

    updateRowNumbers();

    // 调用FFmpeg进行预加载操作
    if (!videoPath.isEmpty() || !audioPath.isEmpty()) {
        QProcess* ffmpegProcess = new QProcess(this);
        QStringList args;

        // 构建FFmpeg命令来获取媒体信息
        if (!videoPath.isEmpty()) {
            args << "-i" << videoPath;
        }
        if (!audioPath.isEmpty() && videoPath != audioPath) {
            if (args.isEmpty()) {
                args << "-i" << audioPath;
            } else {
                args << "-i" << audioPath;
            }
        }

        // 只获取信息，不进行实际处理
        args << "-f" << "null" << "-";

        connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [ffmpegProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                    ffmpegProcess->deleteLater();
                });

        ffmpegProcess->start("ffmpeg", args);
    }
}

void MainWindow::initPathMemory()
{
    // 加载保存的路径
    QSettings settings;
    m_lastOutputPath = settings.value("LastOutputPath",
                                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();

    // 设置到UI
    ui->outputAdd_Edit->setText(m_lastOutputPath);

    // 如果没有保存的路径，使用默认路径
    if (m_lastVideoPath.isEmpty()) m_lastVideoPath = QDir::homePath();
    if (m_lastAudioPath.isEmpty()) m_lastAudioPath = QDir::homePath();
    if (m_lastOutputPath.isEmpty()) m_lastOutputPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (m_lastTitleFolderPath.isEmpty()) m_lastTitleFolderPath = QDir::homePath();
}

void MainWindow::loadPathSettings()
{
    QSettings settings;
    m_lastVideoPath = settings.value("Last/VideoPath", QDir::homePath()).toString();
    m_lastAudioPath = settings.value("Last/AudioPath", QDir::homePath()).toString();
    m_lastOutputPath = settings.value("Last/OutputPath",
                                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    m_lastTitleFolderPath = settings.value("Last/TitleFolderPath", QDir::homePath()).toString();
}

void MainWindow::savePathSettings()
{
    QSettings settings;
    settings.setValue("Last/VideoPath", m_lastVideoPath);
    settings.setValue("Last/AudioPath", m_lastAudioPath);
    settings.setValue("Last/OutputPath", m_lastOutputPath);
    settings.setValue("Last/TitleFolderPath", m_lastTitleFolderPath);
}
>>>>>>> 3d4335f4f49c6d54dd858b9728791ca344b55e42
