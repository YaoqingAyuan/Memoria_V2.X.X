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
        exportSingleItem(row);
    } else {
        QMessageBox::information(this, "导出", "请先选择一个项目");
    }
}

void MainWindow::onExportSelected()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        exportSelectedItems();
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
    // 导出单行项目的实现
    if (row >= 0 && row < m_videoItems.size()) {
        VideoItem* item = m_videoItems[row];
        // 实现导出逻辑
    }
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
    // 导出所有项目的实现
    for (VideoItem* item : m_videoItems) {
        // 实现导出逻辑
    }
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
    // 使用成员变量而不是静态变量
    if (m_exportInProgress) return;

    m_exportInProgress = true;

    if (m_rememberExportChoice) {
        // 直接使用记住的导出模式
        performExportOperation(m_exportMode);
    } else {
        // 弹出导出设置对话框
        export_setting_dialog dialog(this, m_exportMode, m_rememberExportChoice);
        dialog.setWindowTitle(tr("生成模式设置"));

        if (dialog.exec() == QDialog::Accepted) {
            // 使用对话框返回的设置更新当前会话
            ExportMode newMode = dialog.getExportMode();
            bool remember = dialog.rememberChoice();

            setExportSettingsSessionOnly(newMode, remember);
            performExportOperation(newMode);
        }
    }

    m_exportInProgress = false;
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
    loadPathSettings();

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
