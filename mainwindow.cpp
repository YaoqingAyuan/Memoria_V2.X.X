#include "mainwindow.h"
#include "managers/contextmenumanager.h"
#include "ui_mainwindow.h"
#include <QHeaderView>
#include <QProgressBar>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>
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
    , m_mergeManager(nullptr)
{
    qDebug() << "------------------ MainWindow Initialization ------------------";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Platform:" << QSysInfo::prettyProductName();

    // ===================== UI初始化 =====================
    qDebug() << "UI setup start";
    ui->setupUi(this);
    qDebug() << "UI setup done. MaintableView:" << ui->MaintableView;
    this->setWindowTitle("Memoria V2.3.11");

    // 检查关键UI组件
    if (!ui->MaintableView) {
        qCritical() << "MaintableView is null!";
        return;
    }

    // ===================== 核心组件初始化 =====================
    // 初始化TableManager
    qDebug() << "Creating TableManager";
    m_tableManager = new TableManager(ui->MaintableView, this);
    qDebug() << "Before table view initialization";
    m_tableManager->initTableView();  // 初始化表格视图
    qDebug() << "Table view initialized";

    // 初始化路径设置
    m_tableManager->initPathMemory();
    ui->outputAdd_Edit->setText(m_tableManager->lastOutputPath());

    // 初始化进度条委托
    m_progressDelegate = new QProgressBar(this);
    m_progressDelegate->setRange(0, 100);
    m_progressDelegate->setTextVisible(false);

    // ===================== 设置加载 =====================
    QSettings settings;

    // 加载删除设置
    m_rememberDeleteChoice = settings.value("delete/remember", false).toBool();
    m_deleteMode = static_cast<DeleteMode>(settings.value("delete/mode", DeleteFirst).toInt());

    // 加载导出设置
    m_rememberExportChoice = settings.value("RememberExportChoice", false).toBool();
    m_exportMode = static_cast<ExportMode>(settings.value("ExportMode", ExportSingle).toInt());
    updateExportStatusDisplay();

    // ===================== 管理器初始化 =====================
    // 初始化上下文菜单管理器
    qDebug() << "Creating ContextMenuManager";
    m_contextMenuManager = new ContextMenuManager(this, ui->MaintableView, this);
    qDebug() << "ContextMenuManager created:" << (m_contextMenuManager != nullptr);

    // 初始化合并管理器
    qDebug() << "Creating MergeManager";
    m_mergeManager = new MergeManager(m_tableManager, this);
    qDebug() << "MergeManager created at" << m_mergeManager;

    // ===================== 上下文菜单设置 =====================
    qDebug() << "Setting up context menu";
    setupContextMenu();
    qDebug() << "Context menu setup complete";

    // ===================== 信号连接 =====================
    // 连接上下文菜单管理器的信号
    connect(m_contextMenuManager, &ContextMenuManager::importSourceRequested,
            this, &MainWindow::on_wholsoueflie_importButton_clicked);

    // 新增的信号连接（来自ContextMenuManager的预览和导出请求）
    connect(m_contextMenuManager, &ContextMenuManager::previewRequested,
            this, &MainWindow::previewItemAtRow);
    connect(m_contextMenuManager, &ContextMenuManager::exportSingleRequested,
            this, [this]() { performExportOperation(ExportSingle); });
    connect(m_contextMenuManager, &ContextMenuManager::exportSelectedRequested,
            this, [this]() { performExportOperation(ExportSelected); });
    connect(m_contextMenuManager, &ContextMenuManager::exportAllRequested,
            this, [this]() { performExportOperation(ExportAll); });
    // 连接上下文菜单的信号
    // 连接上下文菜单的信号
    if (m_contextMenuManager) {
        connect(m_contextMenuManager, &ContextMenuManager::importSourceRequested,
                this, &MainWindow::on_wholsoueflie_importButton_clicked);

        // 新增预览连接
        connect(m_contextMenuManager, &ContextMenuManager::previewRequested,
                this, &MainWindow::previewItemAtRow);
    }

    // 连接合并管理器的信号
    connect(m_mergeManager, &MergeManager::errorOccurred, this, [this](const QString& error) {
        QMessageBox::critical(this, "错误", error);
    });
    connect(m_mergeManager, &MergeManager::totalProgressChanged,
            ui->Total_progressBar, &QProgressBar::setValue);
    connect(m_mergeManager, &MergeManager::mergingFinished,
            this, &MainWindow::showMergeResultMessage);

    // ===================== 初始化状态检查 =====================
    qDebug() << "All connections established";
    qDebug() << "------------------ Initial State Check ------------------";
    qDebug() << "Video Items Count:" << m_tableManager->rowCount();
    qDebug() << "Merge Manager Status:" << (m_mergeManager ? "Initialized" : "Not Initialized");
    qDebug() << "MainWindow constructor completed";
}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow() start";
    qDebug() << "Deleting UI";
    delete ui;
    qDebug() << "UI deleted";
    qDebug() << "~MainWindow() end";  // 移除对videoItems和tableModel的操作
}


// ===================== 初始化函数组 =====================
void MainWindow::setupContextMenu()
{
    qDebug() << "=== Entering setupContextMenu ===";
    qDebug() << "ContextMenuManager valid:" << (m_contextMenuManager != nullptr);

    if (m_contextMenuManager) {
        m_contextMenuManager->setupContextMenu();
    }

    qDebug() << "=== Context menu setup complete ===";
}


// ===================== UI按钮槽函数组 =====================
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
    qDebug() << "Current table model:" << ui->MaintableView->model();
    qDebug() << "TableManager model:" << m_tableManager->tableModel();
    qDebug() << "Add button clicked. Current row count:" << m_tableManager->rowCount();

    // 创建新行对象
    int newIndex = m_tableManager->rowCount() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);

    qDebug() << "New VideoItem created:" << newItem;

    // 使用 TableManager 添加新行
    m_tableManager->addNewRow(newItem);
    qDebug() << "After addNewRow: Rows=" << m_tableManager->rowCount();
}

void MainWindow::on_delelineButton_clicked()
{
    if (m_rememberDeleteChoice) {
        performDeleteOperation(m_deleteMode);  // 使用统一接口
    } else {
        del_setting_dialog dialog(m_deleteMode, this);
        dialog.setWindowTitle(tr("设置删除模式"));

        if (dialog.exec() == QDialog::Accepted) {
            setDeleteSettings(dialog.getDeleteMode(), dialog.rememberChoice());
            performDeleteOperation(m_deleteMode);  // 使用统一接口
        }
    }
}

//打开文件浏览器，设置输出地址
void MainWindow::on_outputButton_clicked()
{
    // 使用TableManager中的路径作为初始目录
    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        tr("选择输出目录"),
        m_tableManager->lastOutputPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!outputDir.isEmpty()) {
        ui->outputAdd_Edit->setText(outputDir);

        // 更新TableManager中的路径并保存
        m_tableManager->setLastOutputPath(outputDir);
        m_tableManager->savePathSettings();
    }
}

void MainWindow::on_mergeStartBtn_clicked()
{
    qDebug() << "=== 开始混流操作 ===";
    qDebug() << "输出路径:" << ui->outputAdd_Edit->text();

    // 检查输出路径
    QString outputPath = ui->outputAdd_Edit->text();
    if (outputPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "输出路径不能为空");
        qCritical() << "错误：输出路径为空";
        return;
    }

    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        qWarning() << "输出路径不存在，尝试创建:" << outputPath;
        if (!outputDir.mkpath(".")) {
            QMessageBox::warning(this, "错误", "无法创建输出目录");
            qCritical() << "无法创建输出目录:" << outputPath;
            return;
        }
    }

    // 检查MergeManager是否正在处理中
    if (m_mergeManager->isProcessing()) {
        qDebug() << "混流进程已在运行，跳过本次操作";
        return;
    }

    // 根据导出模式执行操作
    if (m_rememberExportChoice) {
        qDebug() << "使用记忆的导出模式:" << m_exportMode;
        performExportOperation(m_exportMode);
    } else {
        qDebug() << "显示导出设置对话框";
        export_setting_dialog dialog(this, m_exportMode, m_rememberExportChoice);
        dialog.setWindowTitle(tr("生成模式设置"));

        if (dialog.exec() == QDialog::Accepted) {
            ExportMode newMode = dialog.getExportMode();
            bool remember = dialog.rememberChoice();

            setExportSettings(newMode, remember);
            qDebug() << "用户选择导出模式:" << newMode;
            performExportOperation(newMode);
        }
    }
}


// ===================== 删除操作管理 =====================
void MainWindow::setDeleteSettings(DeleteMode mode, bool remember)
{
    m_deleteMode = mode;
    m_rememberDeleteChoice = remember;

    // 保存设置
    QSettings settings;
    settings.setValue("delete/remember", remember);
    settings.setValue("delete/mode", static_cast<int>(mode));
}

void MainWindow::performDeleteOperation(DeleteMode mode)
{
    if (!m_tableManager) {
        qCritical() << "TableManager is null!";
        return;
    }

    switch (mode) {
    case DeleteFirst:
        if (m_tableManager->rowCount() > 0) {
            m_tableManager->removeRow(0);
        }
        break;
    case DeleteSelected:
        if (ui->MaintableView->selectionModel()->hasSelection()) {
            m_tableManager->removeSelectedRows(ui->MaintableView->selectionModel()->selectedRows());
        }
        break;
    case DeleteAll:
        m_tableManager->removeAllRows();
        break;
    }
}


// ===================== 导出设置函数组 =====================
// 修改导出设置保存方法
void MainWindow::setExportSettings(ExportMode mode, bool remember)
{
    m_exportMode = mode;
    m_rememberExportChoice = remember;

    if (remember) {
        // 持久化存储到QSettings
        QSettings settings;
        settings.setValue("ExportMode", static_cast<int>(mode));
    }
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

    // 复用输出路径输入框作为临时状态显示
    //ui->outputAdd_Edit->setPlaceholderText(status);
}

//该函数为导出操作的分发中心，其核心功能是根据不同的导出模式
//（ExportSingle/ExportSelected/ExportAll）调用对应的导出执行函数
void MainWindow::performExportOperation(ExportMode mode)
{
    qDebug() << "=== 开始执行导出操作 ===";
    qDebug() << "导出模式:" << mode << (mode == ExportSingle ? "(单个)" :
                                            mode == ExportSelected ? "(选中)" : "(全部)");

    // 检查混流管理器状态
    if (m_mergeManager->isProcessing()) {
        qDebug() << "导出中止：混流进程已在运行中";
        return;
    }

    QList<VideoItem*> pendingItems;
    QString outputPath = ui->outputAdd_Edit->text();
    qDebug() << "输出路径:" << outputPath;

    // 空路径检查
    if (outputPath.isEmpty()) {
        qDebug() << "错误：输出路径为空";
        QMessageBox::warning(this, "错误", "请先设置输出路径");
        return;
    }

    // 验证输出目录
    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        qDebug() << "输出目录不存在，尝试创建:" << outputPath;
        if (!outputDir.mkpath(".")) {
            qDebug() << "创建输出目录失败";
            QMessageBox::warning(this, "错误", "无法创建输出目录");
            return;
        }
        qDebug() << "输出目录创建成功";
    }

    switch (mode) {
    case ExportSingle:
        qDebug() << "模式：导出单个项目";
        if (auto index = ui->MaintableView->currentIndex(); index.isValid()) {
            VideoItem* item = m_tableManager->videoItemAt(index.row());
            qDebug() << "选中的项目: 行" << index.row()
                     << "标题:" << item->data(COL_TITLE).toString()
                     << "视频:" << item->data(COL_VIDEO_FILE).toString()
                     << "音频:" << item->data(COL_AUDIO_FILE).toString();

            pendingItems.append(item);
        } else {
            qDebug() << "没有选中的项目";
            QMessageBox::information(this, "导出", "请先选择一个项目");
        }
        break;

    case ExportSelected: {
        qDebug() << "模式：导出选中项目";
        auto selectedRows = ui->MaintableView->selectionModel()->selectedRows();
        qDebug() << "选中的行数:" << selectedRows.count();

        for (const auto& index : selectedRows) {
            // ...
        }

        if (pendingItems.isEmpty()) {
            qDebug() << "没有选中的项目";
            QMessageBox::information(this, "导出", "没有选中的项目");
        }
        break;
    }

    case ExportAll:
        qDebug() << "模式：导出全部项目";
        pendingItems = m_tableManager->videoItems();
        qDebug() << "全部项目数量:" << pendingItems.size();

        for (VideoItem* item : pendingItems) {
            qDebug() << "  - 项目:"
                     << "标题:" << item->data(COL_TITLE).toString()
                     << "视频:" << item->data(COL_VIDEO_FILE).toString()
                     << "音频:" << item->data(COL_AUDIO_FILE).toString();
        }

        if (pendingItems.isEmpty()) {
            qDebug() << "没有可导出的项目";
            QMessageBox::information(this, "导出", "没有可导出的项目");
        }
        break;
    }

    qDebug() << "待导出项目总数:" << pendingItems.size();

    // 执行导出
    if (!pendingItems.isEmpty()) {
        switch (mode) {
        case ExportSingle:
            qDebug() << "调用 exportItem()";
            m_mergeManager->exportItem(pendingItems.first(), outputPath);
            break;
        case ExportSelected:
            qDebug() << "调用 exportSelectedItems()";
            m_mergeManager->exportSelectedItems(pendingItems, outputPath);
            break;
        case ExportAll:
            qDebug() << "调用 exportAllItems()";
            m_mergeManager->exportAllItems(pendingItems, outputPath);
            break;
        }
        qDebug() << "导出命令已发送";
    } else {
        qDebug() << "没有项目需要导出";
    }

    qDebug() << "=== 导出操作完成 ===";
}


// ===================== 其他功能函数组 =====================
void MainWindow::showMergeResultMessage(int successCount, int failedCount)
{
    qDebug() << "混流完成! 成功:" << successCount << "失败:" << failedCount;

    // 检查输出目录内容
    QString outputPath = ui->outputAdd_Edit->text();
    QDir outputDir(outputPath);
    if (outputDir.exists()) {
        qDebug() << "输出目录内容:";
        for (QFileInfo file : outputDir.entryInfoList(QDir::Files)) {
            qDebug() << "  - " << file.fileName() << "大小:" << file.size() << "字节";
        }
    } else {
        qWarning() << "输出目录不存在:" << outputPath;
    }

    QString message = QString("混流完成! 成功: %1, 失败: %2").arg(successCount).arg(failedCount);
    QMessageBox::information(this, "混流完成", message);
}


void MainWindow::handleImportData(const QString& videoPath, const QString& audioPath, const QString& title)
{
    // 委托给 TableManager 添加新行
    m_tableManager->addVideoItem(videoPath, audioPath, title);

    // 更新路径记忆
    if (!videoPath.isEmpty()) {
        m_tableManager->setLastVideoPath(QFileInfo(videoPath).path());
    }
    if (!audioPath.isEmpty()) {
        m_tableManager->setLastAudioPath(QFileInfo(audioPath).path());
    }
    m_tableManager->savePathSettings();

    // 保留原有的 FFmpeg 预加载操作
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

// ==================== 预览函数组 ====================
void MainWindow::previewItemAtRow(int row)
{
    VideoItem* item = m_tableManager->videoItemAt(row);
    if (!item) {
        QMessageBox::warning(this, "预览", "无效的行索引");
        return;
    }

    if (m_playbackWidget) {
        delete m_playbackWidget;
        m_playbackWidget = nullptr;
    }

    m_playbackWidget = new Playback_Widge(this);
    QString title = item->data(COL_TITLE).toString();
    m_playbackWidget->setWindowTitle("视频预览 - " + title);
    m_playbackWidget->setWindowFlags(Qt::Window);
    m_playbackWidget->show();
}
