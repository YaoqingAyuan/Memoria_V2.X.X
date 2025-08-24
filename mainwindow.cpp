#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QProgressBar>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
#include "delegates/progressbardelegate.h"
#include "dialogs/setting_dialog.h"
#include "dialogs/singleline_import_dialog.h"
#include "dialogs/del_setting_dialog.h"
#include "dialogs/export_setting_dialog.h"
#include "data_models/tablemanager.h"
#include "managers/mergemanager.h" // 确保cpp文件也包含这个头文件

// ===================== 构造函数/析构函数 =====================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_tableManager(nullptr)
    , m_mergeManager(nullptr)
{
    qDebug() << "UI setup start";
    ui->setupUi(this);
    qDebug() << "UI setup done. MaintableView:" << ui->MaintableView;

    if (!ui->MaintableView) {
        qCritical() << "MaintableView is null!";
        return;
    }

    // 只创建一次 TableManager
    qDebug() << "Creating TableManager";
    m_tableManager = new TableManager(ui->MaintableView, this);

    this->setWindowTitle("Memoria V2.2.8");
    qDebug() << "Title set to 'Memoria V2.2.8'.";

    // 确保MaintableView存在
    if (!ui->MaintableView) {
        qCritical() << "MaintableView is null!";
        return;
    }

    // 初始化路径记忆
    initPathMemory();

    // 初始化表格 - 通过 TableManager
    m_tableManager->initTableView();

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

    // 在MainWindow构造函数中添加：
    qDebug() << "TableManager initialized with table view:" << ui->MaintableView;


    // 创建 MergeManager 实例
    m_mergeManager = new MergeManager(this);

    connect(m_mergeManager, &MergeManager::errorOccurred, this, [this](const QString& error)
            {
                QMessageBox::critical(this, "错误", error);
            });

    connect(m_mergeManager, &MergeManager::totalProgressChanged, ui->Total_progressBar, &QProgressBar::setValue);

    connect(m_mergeManager, &MergeManager::mergingFinished, this, &MainWindow::showMergeResultMessage);

}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow() start";
    qDebug() << "Deleting UI";
    delete ui;
    qDebug() << "UI deleted";
    qDebug() << "~MainWindow() end";  // 移除对videoItems和tableModel的操作
}


// ===================== 初始化函数 =====================
void MainWindow::setupContextMenu()
{
    ui->MaintableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->MaintableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}


void MainWindow::initPathMemory()
{
    qDebug() << "Initializing path memory..."; // 调试点7
    qDebug() << "Loading paths...";

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

    qDebug() << "Path memory initialized."; // 调试点8
    qDebug() << "Last output path:" << m_lastOutputPath;
}



// ===================== 路径记忆和设置 =====================
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
    qDebug() << "========== OPENING SETTING DIALOG ==========";
    qDebug() << "Current row count:" << m_tableManager->rowCount();
    qDebug() << "Current column count:" << m_tableManager->tableModel()->columnCount();

    Setting_Dialog dialog(m_tableManager, this); // 传入TableManager
    dialog.setWindowTitle(tr("设置"));

    if (dialog.exec() == QDialog::Accepted) {
        qDebug() << "Setting_Dialog accepted, updating table headers";
        m_tableManager->updateTableHeaders(); // 通过TableManager更新表头
    }
    qDebug() << "========== SETTING DIALOG CLOSED ==========";
}

void MainWindow::on_addlineButton_clicked()
{
    // 创建新行对象
    int newIndex = m_tableManager->videoItems().size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);

    // 使用 TableManager 添加新行
    m_tableManager->addNewRow(newItem);
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

//打开文件浏览器，设置输出地址
void MainWindow::on_outputButton_clicked()
{
    // 打开文件夹选择对话框
    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择输出目录"),
        m_lastOutputPath,
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

void MainWindow::on_mergeStartBtn_clicked()
{
    // 如果导出正在进行，则直接返回
    if (m_exportInProgress) return;
    m_exportInProgress = true;

    // 创建待处理项目列表
    QList<VideoItem*> pendingItems;

    // 根据导出模式确定要处理的项
    if (m_rememberExportChoice) {
        // 使用记住的导出模式
        switch(m_exportMode) {
        case ExportSingle:
            if (!m_tableManager->videoItems().isEmpty()) {
                pendingItems.append(m_tableManager->videoItems().first());
            }
            break;
        case ExportSelected:
        {
            QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
            for (const QModelIndex &index : selected) {
                if (index.row() < m_tableManager->videoItems().size()) {
                    pendingItems.append(m_tableManager->videoItems()[index.row()]);
                }
            }
            break;
        }
        case ExportAll:
            pendingItems = m_tableManager->videoItems();
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
                if (!m_tableManager->videoItems().isEmpty()) {
                    pendingItems.append(m_tableManager->videoItems().first());
                }
                break;
            case ExportSelected:
            {
                QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
                for (const QModelIndex &index : selected) {
                    if (index.row() < m_tableManager->videoItems().size()) {
                        pendingItems.append(m_tableManager->videoItems()[index.row()]);
                    }
                }
                break;
            }
            case ExportAll:
                pendingItems = m_tableManager->videoItems();
                break;
            }
        } else {
            m_exportInProgress = false;
            return; // 用户取消
        }
    }

    // 如果没有待处理项目，显示提示信息并返回
    if (pendingItems.isEmpty()) {
        QMessageBox::information(this, "导出", "没有可导出的项目");
        m_exportInProgress = false;
        return;
    }

    // 开始处理 - 使用 MergeManager
    QString outputPath = ui->outputAdd_Edit->text();
    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, "输出错误", "请先设置输出目录");
        m_exportInProgress = false;
        return;
    }

    // 确保输出目录存在
    QDir outputDir(outputPath);
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        QMessageBox::critical(this, "输出错误", "无法创建输出目录：" + outputPath);
        m_exportInProgress = false;
        return;
    }

    // 调用 MergeManager 开始处理
    m_mergeManager->startMergingProcess(pendingItems, outputPath);
}

// ===================== 删除操作管理 =====================
void MainWindow::performDeleteOperation(DeleteMode mode)
{
    if (mode == DeleteFirst) {
        // 删除第一行
        m_tableManager->removeRow(0);
    } else if (mode == DeleteSelected) {
        // 删除选中行
        QModelIndexList selected = ui->MaintableView->selectionModel()->selectedRows();
        m_tableManager->removeSelectedRows(selected);
    } else if (mode == DeleteAll) {
        // 删除所有行
        m_tableManager->removeAllRows();
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


// ===================== 导出操作管理 =====================
void MainWindow::startMergingProcess()
{
    qDebug() << "MainWindow::startMergingProcess - Starting merge process";

    if (!m_mergeManager) {
        qCritical() << "MergeManager is null!";
        return;
    }

    // 将待处理项目传递给 MergeManager
    m_mergeManager->startMergingProcess(m_pendingItems, ui->outputAdd_Edit->text());
}


void MainWindow::showMergeResultMessage(int successCount, int failedCount)
{
    m_exportInProgress = false;  // 重置导出状态

    QString message = QString("混流完成! 成功: %1, 失败: %2")
                          .arg(successCount)
                          .arg(failedCount);
    QMessageBox::information(this, "混流完成", message);
}


void MainWindow::onExportSingle()
{
    QModelIndexList selected = ui->MaintableView->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        if (row >= 0 && row < m_tableManager->videoItems().size()) {
            // 清空现有队列，只添加当前项
            m_pendingItems.clear();
            m_processingItems.clear();
            m_pendingItems.append(m_tableManager->videoItemAt(row));

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
            if (index.row() < m_tableManager->videoItems().size()) {
               m_pendingItems.append(m_tableManager->videoItemAt(index.row()));
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
    m_pendingItems = m_tableManager->videoItems();

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
        if (row >= 0 && row < m_tableManager->videoItems().size()) {
            VideoItem* item = m_tableManager->videoItems()[row];
            // 实现导出逻辑
        }
    }
}

void MainWindow::exportAllItems()
{
    // 清空现有队列，添加所有项
    m_pendingItems.clear();
    m_processingItems.clear();
    m_pendingItems.clear();
    for (int i = 0; i < m_tableManager->rowCount(); ++i) {
        m_pendingItems.append(m_tableManager->videoItemAt(i));
    }

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

void MainWindow::setExportSettingsSessionOnly(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    // 注意：这里不保存到QSettings，只更新当前会话
    updateExportStatusDisplay();
}


// ===================== 合并处理函数 =====================

// 修改updateTotalProgress函数
void MainWindow::updateTotalProgress()
{
    if (m_tableManager->rowCount() == 0) {
        ui->Total_progressBar->setValue(0);
        return;
    }

    int totalProgress = 0;
    int validItems = 0;

    for (int i = 0; i < m_tableManager->rowCount(); ++i) {
        VideoItem* item = m_tableManager->videoItemAt(i);
        if (item) {
            int progress = item->progress();
            if (progress >= 0 && !item->hasError()) {
                totalProgress += progress;
                validItems++;
            }
        }
    }

    if (validItems > 0) {
        totalProgress /= validItems;
    }

    ui->Total_progressBar->setValue(totalProgress);
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
    // 这里需要修改，因为 m_currentColumnsOrder 已经迁移到 TableManager 中
    // 您需要在 TableManager 中添加一个方法来获取当前列的顺序
    // TableColumns column = COL_INDEX;
    // if (index.column() >=0 && index.column() < m_tableManager->currentColumnsOrder().size()) {
    //     column = m_tableManager->currentColumnsOrder()[index.column()];
    // }

    QAction *importFile = menu.addAction("导入...");
    importFile->setData("import_file");
    // importFile->setEnabled(column == COL_VIDEO_FILE || column == COL_AUDIO_FILE);
    importFile->setEnabled(true); // 暂时设置为总是启用

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

    // // 连接删除操作
    // connect(deleteCurrent, &QAction::triggered, this, [this, index]() {
    //     int row = index.row();
    //     if (row >= 0 && row < m_tableManager->videoItems().size()) {
    //         delete m_tableManager->videoItems().takeAt(row);
    //         m_tableManager->tableModel()->removeRow(row);
    //         m_tableManager->updateRowNumbers();
    //     }
    // });

    // 在showContextMenu函数中，修改删除当前行的操作：
    connect(deleteCurrent, &QAction::triggered, this, [this, index]() {
        int row = index.row();
        m_tableManager->removeRow(row); // 使用TableManager的方法
    });

    connect(deleteAll, &QAction::triggered, this, [this]() {
        performDeleteOperation(DeleteAll);
    });

    // 连接导入操作
    connect(&menu, &QMenu::triggered, this, &MainWindow::onCustomContextMenuAction);

    // 连接导出操作
    connect(exportSingle, &QAction::triggered, this, &MainWindow::onExportSingle);
    connect(exportSelected, &QAction::triggered, this, &MainWindow::onExportSelected);
    connect(exportAll, &QAction::triggered, this, &MainWindow::onExportAll);

    menu.exec(ui->MaintableView->viewport()->mapToGlobal(pos));
}



// 修改onPreviewAction函数
void MainWindow::onPreviewAction(int row)
{
    // 修改后：使用 videoItemAt(row)
    VideoItem* item = m_tableManager->videoItemAt(row);
    if (!item) {
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
    QString title = item->data(COL_TITLE).toString();
    m_playbackWidget->setWindowTitle("视频预览 - " + title);

    m_playbackWidget->setWindowFlags(Qt::Window);
    m_playbackWidget->show();

    // 这里可以添加代码将选中的视频项数据传递给预览窗口
    // 例如：m_playbackWidget->setVideoItem(item);
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


// ===================== 处理导入数据 =====================
void MainWindow::handleImportData(const QString& videoPath, const QString& audioPath, const QString& title)
{
    // 创建新行对象
    int newIndex = m_tableManager->rowCount() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    newItem->setHasError(false); // 确保新项目没有错误状态

    // 设置数据
    newItem->setTitle(title);
    if (!videoPath.isEmpty()) {
        newItem->setData(COL_VIDEO_FILE, videoPath);
    }
    if (!audioPath.isEmpty()) {
        newItem->setData(COL_AUDIO_FILE, audioPath);
    }

    // 使用 TableManager 添加新行
    m_tableManager->addNewRow(newItem);

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
