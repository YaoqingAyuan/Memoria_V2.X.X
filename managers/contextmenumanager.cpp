#include "managers/contextmenumanager.h"
#include "mainwindow.h"
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include "data_models/tablemanager.h"

ContextMenuManager::ContextMenuManager(MainWindow* mainWindow, QTableView* tableView, QObject* parent)
    : QObject(parent), m_mainWindow(mainWindow), m_tableView(tableView),
    m_contextMenu(new QMenu(tableView))
{
    // 初始化路径设置
    loadPathSettings();

    if (!tableView) {
        qCritical() << "ContextMenuManager: tableView is null!";
        return;
    }

    // 创建菜单
    //m_contextMenu = new QMenu(tableView);
    createActions();
    connectActions();
}

void ContextMenuManager::setupContextMenu()
{
    qDebug() << "=== Entering setupContextMenu ===";

    // 双重空指针保护
    if (!m_tableView || !m_mainWindow) {
        qCritical() << "setupContextMenu: tableView or mainWindow is null!";
        return;
    }

    // 确保菜单已创建
    if(!m_contextMenu) {
        qCritical() << "Context menu not initialized!";
        return;
    }
    qDebug() << "Setting up context menu for table view:" << m_tableView;

    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    bool connected = connect(m_tableView, &QTableView::customContextMenuRequested,
                             this, &ContextMenuManager::showContextMenu);
    qDebug() << "CustomContextMenu connection:" << connected;
}

void ContextMenuManager::showContextMenu(const QPoint& pos)
{
    // 三重空指针保护
    if(!m_tableView || !m_mainWindow || !m_contextMenu) {
        qCritical() << "Cannot show context menu - missing dependencies";
        return;
    }

    QModelIndex index = m_tableView->indexAt(pos);
    if (!index.isValid()) return;

    // 设置"导入..."的可用性（仅在音视频列可用）
    bool isMediaColumn = (index.column() == COL_VIDEO_FILE || index.column() == COL_AUDIO_FILE);
    m_importFileAction->setEnabled(isMediaColumn);

    // 安全设置预览数据
    if(m_previewAction) {
        int row = index.row();
        m_previewAction->setData(row);
    }

    // 安全获取视口
    if(auto viewport = m_tableView->viewport()) {
        m_contextMenu->exec(viewport->mapToGlobal(pos));
    }
}

void ContextMenuManager::onCustomContextMenuAction(QAction* action)
{
    QString actionType = action->data().toString();
    if (actionType == "import_file") {
        emit importFileRequested();
    } else if (actionType == "import_title") {
        emit importTitleRequested();
    } else if (actionType == "import_source") {
        emit importSourceRequested();
    }
}

void ContextMenuManager::createActions()
{
    qDebug() << "Creating context menu actions";

    m_previewAction = new QAction("预览", this);
    m_deleteCurrentAction = new QAction("删除该行", this);
    m_deleteSelectedAction = new QAction("删除选中项", this);
    m_deleteAllAction = new QAction("删除所有行", this);

    m_importFileAction = new QAction("导入...", this);
    m_importFileAction->setData("import_file");

    m_importTitleAction = new QAction("导入标题文件夹", this);
    m_importTitleAction->setData("import_title");

    m_importSourceAction = new QAction("导入缓存源文件", this);
    m_importSourceAction->setData("import_source");

    m_exportSingleAction = new QAction("导出该项", this);
    m_exportSelectedAction = new QAction("导出选中项", this);
    m_exportAllAction = new QAction("导出全部", this);

    // 添加动作到菜单
    m_contextMenu->addAction(m_previewAction);
    m_contextMenu->addAction(m_deleteCurrentAction);
    m_contextMenu->addAction(m_deleteSelectedAction);
    m_contextMenu->addAction(m_deleteAllAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_importFileAction);
    m_contextMenu->addAction(m_importTitleAction);
    m_contextMenu->addAction(m_importSourceAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_exportSingleAction);
    m_contextMenu->addAction(m_exportSelectedAction);
    m_contextMenu->addAction(m_exportAllAction);
}

void ContextMenuManager::connectActions()
{
    connect(m_deleteCurrentAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow && m_tableView) {
            if (auto index = m_tableView->currentIndex(); index.isValid()) {
                m_mainWindow->performDeleteOperation(DeleteFirst);  // 使用统一接口
            }
        }
    });

    connect(m_deleteSelectedAction, &QAction::triggered, this, [this]() {
        m_mainWindow->performDeleteOperation(DeleteSelected);  // 使用统一接口
    });

    connect(m_deleteAllAction, &QAction::triggered, this, [this]() {
        m_mainWindow->performDeleteOperation(DeleteAll);  // 使用统一接口
    });

    // 修复导入操作连接
    connect(m_importFileAction, &QAction::triggered, this, [this]() {
        onCustomContextMenuAction(m_importFileAction);
    });

    connect(m_importTitleAction, &QAction::triggered, this, [this]() {
        onCustomContextMenuAction(m_importTitleAction);
    });

    connect(m_importSourceAction, &QAction::triggered, this, [this]() {
        onCustomContextMenuAction(m_importSourceAction);
    });

    // ========== 导出操作重构 ==========
    // 预览动作：直接调用MainWindow的预览函数
    // 预览动作
    connect(m_previewAction, &QAction::triggered, this, [this]() {
        if (!m_tableView || !m_mainWindow) return;
        if (auto index = m_tableView->currentIndex(); index.isValid()) {
            emit previewRequested(index.row()); // 发射信号而不是直接调用
        }
    });

    // 导出操作
    connect(m_exportSingleAction, &QAction::triggered, this, [this]() {
        emit exportSingleRequested(); // 发射信号
    });

    connect(m_exportSelectedAction, &QAction::triggered, this, [this]() {
        emit exportSelectedRequested(); // 发射信号
    });

    connect(m_exportAllAction, &QAction::triggered, this, [this]() {
        emit exportAllRequested(); // 发射信号
    });

    // 连接导入文件动作
    connect(m_importFileAction, &QAction::triggered, this, [this]() {
        if (!m_tableView) return;
        QModelIndex index = m_tableView->currentIndex();
        if (index.isValid()) {
            importFile(index.row(), index.column());
        }
    });

    // 连接导入标题文件夹动作
    connect(m_importTitleAction, &QAction::triggered, this, [this]() {
        if (!m_tableView) return;
        QModelIndex index = m_tableView->currentIndex();
        if (index.isValid()) {
            importTitleFolder(index.row());
        }
    });
}

// ===================== 路径管理 =====================
void ContextMenuManager::loadPathSettings()
{
    QSettings settings;
    m_lastFileDialogPath = settings.value("Last/FileDialogPath", QDir::homePath()).toString();
    m_lastTitleFolderPath = settings.value("Last/TitleFolderPath", QDir::homePath()).toString();
}

void ContextMenuManager::savePathSettings()
{
    QSettings settings;
    settings.setValue("Last/FileDialogPath", m_lastFileDialogPath);
    settings.setValue("Last/TitleFolderPath", m_lastTitleFolderPath);
}

// ===================== 文件导入 =====================
void ContextMenuManager::importFile(int row, int col)
{
    if (!m_tableView || !m_mainWindow) return;

    QString filter = tr("M4S文件 (*.m4s);;所有文件 (*)");
    QString filePath = QFileDialog::getOpenFileName(
        m_mainWindow,
        tr("选择媒体文件"),
        m_lastFileDialogPath,
        filter
        );

    if (filePath.isEmpty()) return;

    // 更新模型数据
    QAbstractItemModel* model = m_tableView->model();
    if (model) {
        QModelIndex targetIndex = model->index(row, col);
        model->setData(targetIndex, filePath);

        // 新增：同步更新VideoItem数据模型
        if (m_mainWindow && m_mainWindow->tableManager()) {
            TableManager* tm = m_mainWindow->tableManager();
            TableColumns columnType = tm->columnTypeAt(col); // 获取真实列类型
            if (columnType != TOTAL_COLUMNS) {
                tm->updateVideoItem(row, columnType, filePath);
            }
        }
    }

    // 更新路径记忆
    m_lastFileDialogPath = QFileInfo(filePath).absolutePath();
    savePathSettings();
}

// ===================== 标题文件夹导入 =====================
void ContextMenuManager::importTitleFolder(int row)
{
    qDebug() << "=== 开始导入标题文件夹 ===";
    qDebug() << "当前行:" << row;

    if (!m_tableView || !m_mainWindow) {
        qCritical() << "导入失败：tableView或mainWindow为空";
        return;
    }

    QString folderPath = QFileDialog::getExistingDirectory(
        m_mainWindow,
        tr("选择标题文件夹"),
        m_lastTitleFolderPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (folderPath.isEmpty()) {
        qDebug() << "用户取消了文件夹选择";
        return;
    }

    qDebug() << "选择的文件夹路径:" << folderPath;

    QDir dir(folderPath);
    QString videoPath = findMediaFile(dir, "video");
    QString audioPath = findMediaFile(dir, "audio");
    QString title = QFileInfo(folderPath).fileName();

    qDebug() << "找到的视频文件:" << videoPath;
    qDebug() << "找到的音频文件:" << audioPath;
    qDebug() << "生成的标题:" << title;

    // 更新模型
    QAbstractItemModel* model = m_tableView->model();
    if (!model) {
        qCritical() << "模型为空，无法更新";
        return;
    }

    // 获取当前值用于调试
    QModelIndex titleIndex = model->index(row, COL_TITLE);
    QModelIndex videoIndex = model->index(row, COL_VIDEO_FILE);
    QModelIndex audioIndex = model->index(row, COL_AUDIO_FILE);

    qDebug() << "更新前值 - 标题:" << titleIndex.data().toString()
             << " 视频:" << videoIndex.data().toString()
             << " 音频:" << audioIndex.data().toString();

    // 设置新值
    model->setData(titleIndex, title);
    if (!videoPath.isEmpty()) {
        model->setData(videoIndex, videoPath);
    } else {
        qWarning() << "未找到视频文件：" << dir.path();
    }

    if (!audioPath.isEmpty()) {
        model->setData(audioIndex, audioPath);
    } else {
        qWarning() << "未找到音频文件：" << dir.path();
    }

    // 验证更新后的值
    qDebug() << "更新后值 - 标题:" << titleIndex.data().toString()
             << " 视频:" << videoIndex.data().toString()
             << " 音频:" << audioIndex.data().toString();

    // 保存路径
    m_lastTitleFolderPath = QFileInfo(folderPath).absolutePath();
    savePathSettings();
    qDebug() << "=== 标题文件夹导入完成 ===";

    if (model) {
        // 更新模型
        model->setData(titleIndex, title);
        model->setData(videoIndex, videoPath);
        model->setData(audioIndex, audioPath);

        // 修改后的访问方式：
        if (m_mainWindow && m_mainWindow->tableManager()) { // 使用公共getter
            TableManager* tm = m_mainWindow->tableManager();
            tm->updateVideoItem(row, COL_TITLE, title);
            tm->updateVideoItem(row, COL_VIDEO_FILE, videoPath);
            tm->updateVideoItem(row, COL_AUDIO_FILE, audioPath);
        }
    }
}

QString ContextMenuManager::findMediaFile(const QDir& dir, const QString& type)
{
    qDebug() << "在目录中查找媒体文件:" << dir.path() << "类型:" << type;

    // 直接查找固定文件名
    QString fixedFileName = type + ".m4s";
    QString fixedFilePath = dir.filePath(fixedFileName);

    if (QFile::exists(fixedFilePath)) {
        qDebug() << "使用固定文件名:" << fixedFilePath;
        return fixedFilePath;
    }

    qDebug() << "未找到匹配的" << type << "文件";
    return QString();
}
