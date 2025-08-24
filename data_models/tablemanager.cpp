#include "tablemanager.h"
#include <QHeaderView>
#include <QStyle>
#include "delegates/progressbardelegate.h"

TableManager::TableManager(QTableView* tableView, QObject *parent)
    : QObject(parent)
    , m_tableView(tableView)
    , m_tableModel(new QStandardItemModel(this))
    , m_progressDelegate(nullptr)
{
    qDebug() << "TableManager initialized."; // 调试点9
    // 初始化代码
}

TableManager::~TableManager()
{

}

void TableManager::initTableView()
{
    // 设置表格模型
    m_tableView->setModel(m_tableModel);

    // 初始化表头
    updateTableHeaders();

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
}

void TableManager::updateTableHeaders()
{
    // 保存当前行数和列数
    int rowCount = m_tableModel->rowCount();
    int oldColumnCount = m_tableModel->columnCount();

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

        // 确保委托对象存在
        if (!m_progressDelegate) {
            qDebug() << "Creating new progress delegate";
            m_progressDelegate = new ProgressBarDelegate(this);
        }

        qDebug() << "Setting progress delegate:" << m_progressDelegate;
        m_tableView->setItemDelegateForColumn(progressCol, m_progressDelegate);
    }
}

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

void TableManager::updateRowNumbers()
{
    for (int i = 0; i < m_videoItems.size(); ++i) {
        m_videoItems[i]->setIndex(i + 1);
        updateTableRow(i);
    }
}

void TableManager::clearModelData()
{
    m_tableModel->clear();
    qDeleteAll(m_videoItems);
    m_videoItems.clear();
    updateTableHeaders();
}

void TableManager::addNewRow(VideoItem* item)
{
    m_videoItems.append(item);

    // 添加新行到模型
    QList<QStandardItem*> rowItems;
    for (int col = 0; col < m_currentColumnsOrder.size(); ++col) {
        QStandardItem* tableItem = new QStandardItem();
        tableItem->setTextAlignment(Qt::AlignCenter);

        TableColumns colType = m_currentColumnsOrder[col];
        if (colType == COL_PROGRESS) {
            tableItem->setData(0, Qt::DisplayRole);
        } else if (colType == COL_TITLE) {
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
}

// 在tablemanager.cpp中实现这些方法
void TableManager::removeRow(int row)
{
    if (row < 0 || row >= m_videoItems.size())
        return;

    delete m_videoItems.takeAt(row);
    m_tableModel->removeRow(row);
    updateRowNumbers();
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
