#include "tablemanager.h"
#include <QHeaderView>
#include <QStyle>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include "delegates/progressbardelegate.h"

// ===================== 构造函数/析构函数 =====================
TableManager::TableManager(QTableView* tableView, QObject *parent)
    : QObject(parent)
    , m_tableView(tableView)
    , m_tableModel(new QStandardItemModel(this))
    , m_progressDelegate(nullptr)
{

    if (m_tableView) {
        m_tableView->setModel(m_tableModel);
        qDebug() << "Model set to table view in constructor";
    }

    qDebug() << "TableManager initialized."; // 调试点9
    // 初始化代码
}

TableManager::~TableManager()
{

}

// ===================== 初始化函数 =====================
void TableManager::initTableView()
{
    qDebug() << "========== TABLE INITIALIZATION START ==========";
    qDebug() << "TableView address:" << m_tableView;
    qDebug() << "TableView model:" << m_tableView->model();
    qDebug() << "Our model:" << m_tableModel;

    if (!m_tableView) {
        qCritical() << "TableView is null in initTableView!";
        return;
    } else {
        qDebug() << "TableView is valid:" << m_tableView;
    }

    // 添加空指针检查
    if (!m_tableView || !m_tableModel) {
        qCritical() << "CRITICAL: TableView or TableModel is null!";
        return;
    }

    // 确保模型已创建
    if (!m_tableModel) {
        qDebug() << "Creating new table model";
        m_tableModel = new QStandardItemModel(this);
    } else {
        qDebug() << "Table model already exists";
    }

    qDebug() << "TableManager::initTableView start";
    qDebug() << "m_tableView:" << m_tableView;
    qDebug() << "m_tableModel:" << m_tableModel;
    qDebug() << "m_videoItems size:" << m_videoItems.size(); // 新增调试输出

    // 先清空原有数据，防止残留
    clearModelData();

    // 初始化表头
    qDebug() << "Updating table headers";
    updateTableHeaders(); // 这里可能是崩溃点
    qDebug() << "Table headers updated"; // 如果崩溃，可能不会打印这条

    qDebug() << "Setting model to table view";
    // 设置表格模型
    m_tableView->setModel(m_tableModel);
    qDebug() << "Model set to table view";

    // 确保设置模型到视图
    if (m_tableView && m_tableModel) {
        m_tableView->setModel(m_tableModel);
        qDebug() << "Model set to table view";
    }

    // 添加交替行颜色
    m_tableView->setAlternatingRowColors(true);

    // 设置表格属性
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->verticalHeader()->setVisible(false);
    // 修改后：仅允许双击编辑
    m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // 设置行选择样式
    QString style = "QTableView::item:selected {"
                    "    background-color: #FFFACD;"
                    "    color: black;"
                    "}";
    m_tableView->setStyleSheet(style);

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

    qDebug() << "========== TABLE INITIALIZATION COMPLETE ==========";
}

void TableManager::initPathMemory()
{
    qDebug() << "Initializing path memory...";
    loadPathSettings();

    qDebug() << "Path memory initialized.";
    qDebug() << "Last output path:" << m_lastOutputPath;
}

// ===================== 表视图更新函数 =====================
void TableManager::updateTableRow(int rowIndex)
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

void TableManager::updateTableHeaders()
{
    qDebug() << "updateTableHeaders start";

    // 保存当前行数和列数
    int rowCount = m_tableModel->rowCount();
    int oldColumnCount = m_tableModel->columnCount();
    qDebug() << "Current row count:" << rowCount << "Old column count:" << oldColumnCount;


    // 获取可见列标题
    QStringList headers = m_columnManager.getVisibleHeaders();
    qDebug() << "Visible headers count:" << headers.size();


    // 更新当前列的顺序列表
    m_currentColumnsOrder.clear();
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        TableColumns column = static_cast<TableColumns>(i);
        if (m_columnManager.isColumnVisible(column)) {
            m_currentColumnsOrder.append(column);
        }
    }
    qDebug() << "Current columns order size:" << m_currentColumnsOrder.size();

    // 设置新表头
    m_tableModel->setColumnCount(headers.size());
    m_tableModel->setHorizontalHeaderLabels(headers);
    qDebug() << "Set column count and header labels";

    // 添加新列（如果列数增加）
    for (int row = 0; row < rowCount; ++row) {
        for (int col = oldColumnCount; col < headers.size(); ++col) {
            QStandardItem* item = new QStandardItem();
            TableColumns colType = m_currentColumnsOrder[col];
            qDebug() << "Creating new item for row:" << row << "col:" << col << "type:" << colType;

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
    qDebug() << "Added new columns if necessary";

    // 设置列宽策略
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_VIDEO_FILE || colType == COL_AUDIO_FILE) {
            m_tableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::Interactive);
            m_tableView->setColumnWidth(col, 150);
            m_tableView->horizontalHeader()->setMinimumSectionSize(100);
            m_tableView->horizontalHeader()->setMaximumSectionSize(300);
        } else {
            m_tableView->horizontalHeader()->setSectionResizeMode(
                col, QHeaderView::ResizeToContents);
        }
    }

    // 设置进度条委托
    int progressCol = m_columnManager.getVisualIndex(COL_PROGRESS);
    qDebug() << "Setting delegate for progress column:" << progressCol;

    if (progressCol >= 0) {
        // 不再手动删除委托，由Qt自动管理生命周期
        QAbstractItemDelegate* oldDelegate = m_tableView->itemDelegateForColumn(progressCol);
        if (oldDelegate) {
            qDebug() << "Removing old delegate:" << oldDelegate;
            m_tableView->setItemDelegateForColumn(progressCol, nullptr);
            // 注意：不要删除oldDelegate！Qt会管理其生命周期
        }

        // 确保委托对象存在，仅在需要时创建委托
        if (!m_progressDelegate) {
            qDebug() << "Creating new progress delegate";
            m_progressDelegate = new ProgressBarDelegate(this);
        }

        qDebug() << "Setting progress delegate:" << m_progressDelegate;
        // 设置委托（无需删除旧委托）
        m_tableView->setItemDelegateForColumn(progressCol, m_progressDelegate);
    }
}

void TableManager::updateRowNumbers()
{
    for (int i = 0; i < m_videoItems.size(); ++i) {
        m_videoItems[i]->setIndex(i + 1);
        updateTableRow(i);
    }
}


// ===================== 数据操作函数 =====================
void TableManager::clearModelData()
{
    // 1. 先保存当前可见的表头标签
    QStringList savedHeaders = m_columnManager.getVisibleHeaders();
    const int oldColumnCount = savedHeaders.size();

    // 2. 清除数据
    m_tableModel->clear();
    qDeleteAll(m_videoItems);
    m_videoItems.clear();

    // 3. 恢复表头结构（列数和标签）
    m_tableModel->setColumnCount(oldColumnCount);
    m_tableModel->setHorizontalHeaderLabels(savedHeaders); // ✅ 关键修复：重新设置表头标签

    qDebug() << "Model cleared, headers restored";
}

void TableManager::addNewRow(VideoItem* item)
{
    qDebug() << "------------------ Adding New Table Row ------------------";
    qDebug() << "Item Index:" << item->index();
    qDebug() << "Current Rows:" << m_videoItems.size();

    m_videoItems.append(item);

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        TableColumns colType = m_currentColumnsOrder[col];
        QStandardItem* tableItem = new QStandardItem();
        tableItem->setTextAlignment(Qt::AlignCenter);

        // 仅初始化标题列，其他列保持空
        if (colType == COL_TITLE) {
            tableItem->setFlags(tableItem->flags() | Qt::ItemIsEditable);
            tableItem->setText("<新项目>");
        } else {
            tableItem->setFlags(tableItem->flags() & ~Qt::ItemIsEditable);
        }

        rowItems.append(tableItem);
    }
    m_tableModel->appendRow(rowItems);

    // 连接数据变化信号 - 使用唯一连接避免重复
    connect(item, &VideoItem::dataChanged, this, [this, item]()
    {
        int row = m_videoItems.indexOf(item);
        if (row >= 0) updateTableRow(row);
    }
    );

    updateRowNumbers();
    qDebug() << "Row Added Successfully";
}

// 在tablemanager.cpp中实现这些方法
void TableManager::removeRow(int row)
{
    qDebug() << "------------------ Removing Table Row ------------------";
    qDebug() << "Removing Row:" << row;
    qDebug() << "Current Rows:" << m_videoItems.size();

    if (row < 0 || row >= m_videoItems.size())
        return;

    delete m_videoItems.takeAt(row);
    m_tableModel->removeRow(row);
    updateRowNumbers();
    qDebug() << "Row Removed Successfully";
}

void TableManager::removeSelectedRows(const QModelIndexList& selected)
{
    if (selected.isEmpty()) return;

    // 倒序删除避免索引问题
    QList<int> rows;
    for (const QModelIndex &index : selected) {
        rows.append(index.row());
    }

    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (int row : rows) {
        if (row < m_videoItems.size()) {
            delete m_videoItems.takeAt(row);
            m_tableModel->removeRow(row);
        }
    }
    updateRowNumbers();
}

void TableManager::removeAllRows()
{
    clearModelData();
}

void TableManager::addVideoItem(const QString& videoPath,const QString& audioPath,const QString& title)
{
    qDebug() << "Adding new video item via TableManager:";
    qDebug() << "  Video Path:" << videoPath;
    qDebug() << "  Audio Path:" << audioPath;
    qDebug() << "  Title:" << title;

    // 创建新行对象
    int newIndex = m_videoItems.size() + 1;
    VideoItem* newItem = new VideoItem(newIndex, this);
    newItem->setHasError(false);

    // 设置数据
    newItem->setTitle(title);
    if (!videoPath.isEmpty()) {
        newItem->setData(COL_VIDEO_FILE, videoPath);
    }
    if (!audioPath.isEmpty()) {
        newItem->setData(COL_AUDIO_FILE, audioPath);
    }

    // 使用 TableManager 添加新行
    addNewRow(newItem);
}


// ===================== 路径管理函数 =====================
void TableManager::loadPathSettings()
{
    QSettings settings;
    m_lastVideoPath = settings.value("Last/VideoPath", QDir::homePath()).toString();
    m_lastAudioPath = settings.value("Last/AudioPath", QDir::homePath()).toString();
    m_lastOutputPath = settings.value("Last/OutputPath",
                                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
    m_lastTitleFolderPath = settings.value("Last/TitleFolderPath", QDir::homePath()).toString();

    qDebug() << "Loaded path settings:";
    qDebug() << "  VideoPath:" << m_lastVideoPath;
    qDebug() << "  AudioPath:" << m_lastAudioPath;
    qDebug() << "  OutputPath:" << m_lastOutputPath;
    qDebug() << "  TitleFolderPath:" << m_lastTitleFolderPath;
}

void TableManager::savePathSettings()
{
    QSettings settings;
    settings.setValue("Last/VideoPath", m_lastVideoPath);
    settings.setValue("Last/AudioPath", m_lastAudioPath);
    settings.setValue("Last/OutputPath", m_lastOutputPath);
    settings.setValue("Last/TitleFolderPath", m_lastTitleFolderPath);

    qDebug() << "Saved path settings.";
}


// ===================== 工具函数 =====================
VideoItem* TableManager::videoItemAt(int row) const
{
    if (row >= 0 && row < m_videoItems.size())
        return m_videoItems[row];
    return nullptr;
}

int TableManager::rowCount() const
{
    return m_videoItems.size();
}

void TableManager::performDeleteOperation(DeleteMode mode)
{
    if (!m_tableView) {
        qCritical() << "TableView is null in performDeleteOperation!";
        return;
    }

    switch (mode) {
    case DeleteFirst:
        removeRow(0);
        break;
    case DeleteSelected:
        removeSelectedRows(m_tableView->selectionModel()->selectedRows());
        break;
    case DeleteAll:
        removeAllRows();
        break;
    default:
        qWarning() << "Unknown delete mode:" << mode;
    }
}

void TableManager::updateVideoItem(int row, TableColumns column, const QVariant& value) {
    if (row >=0 && row < m_videoItems.size()) {
        m_videoItems[row]->setData(column, value);
    }
}

TableColumns TableManager::columnTypeAt(int visualIndex) const
{
    if (visualIndex >= 0 && visualIndex < m_currentColumnsOrder.size()) {
        return m_currentColumnsOrder[visualIndex];
    }
    return TOTAL_COLUMNS;
}
