#include "tablecolumns.h"
#include <qdebug.h>

// ===================== 构造函数 =====================
ColumnManager::ColumnManager() {
    // 初始化列标题（根据新的枚举顺序调整）

    // 调试：验证列名数量
    qDebug() << "初始化列名，预期数量:" << TOTAL_COLUMNS;

    columnNames = {
        "序号", "视频类型", "视频标题", "创建时间", "视频时长",
        "文件大小", "清晰度", "混流进度",
        "视频文件导入(m4s)", "音频文件导入(m4s)",
        "UP主", "UP主UID", "所属系列", "视频av/bv号",
        "弹幕更新时间(最近)", "最新弹幕数"
    };

    // 添加列名数量验证
    if(columnNames.size() != TOTAL_COLUMNS) {
        qCritical() << "列名数量错误! 实际:" << columnNames.size()
            << "预期:" << TOTAL_COLUMNS;  }

    // 初始化列显示状态
    columnVisibility = QVector<bool>(TOTAL_COLUMNS, false);

    // 设置强制列始终显示
    setColumnVisibility(COL_INDEX, true);
    setColumnVisibility(COL_VIDEO_TYPE, true);  // 新位置
    setColumnVisibility(COL_TITLE, true);
    setColumnVisibility(COL_CREATE_TIME, true); // 新位置
    setColumnVisibility(COL_DURATION, true);
    setColumnVisibility(COL_TOTAL_SIZE, true);  // 新位置
    setColumnVisibility(COL_QUALITY, true);
    setColumnVisibility(COL_PROGRESS, true);
    setColumnVisibility(COL_VIDEO_FILE, true);
    setColumnVisibility(COL_AUDIO_FILE, true);

    // 设置可选列默认状态
    setColumnVisibility(COL_VIDEO_TYPE, true);      // 默认显示视频类型
    setColumnVisibility(COL_CREATE_TIME, true);     // 默认显示创建时间
}

// ===================== 列可见性管理 =====================
void ColumnManager::setColumnVisibility(TableColumns column, bool visible) {
    // 如果是强制列且试图隐藏，则忽略
    if (isForcedColumn(column) && !visible) return;

    // 确保列索引在有效范围内
    if (column >= 0 && column < TOTAL_COLUMNS) {
        columnVisibility[column] = visible;
    }
}

bool ColumnManager::isColumnVisible(TableColumns column) const {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        return columnVisibility[column];
    }
    return false;
}

bool ColumnManager::isForcedColumn(TableColumns column) const {
    const QList<TableColumns> forcedColumns = {
        COL_INDEX, COL_TITLE, COL_DURATION,
        COL_QUALITY, COL_VIDEO_FILE, COL_AUDIO_FILE,
        COL_TOTAL_SIZE, COL_PROGRESS
    };

    // 添加调试输出
    if(forcedColumns.contains(column)) {
        qDebug() << "列" << column << "是强制列";
    }
    return forcedColumns.contains(column);
}


// ===================== 列信息获取 =====================
QStringList ColumnManager::getVisibleHeaders() const {
    QStringList headers;
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        if (columnVisibility[i]) {
            headers << columnNames[i];
        }
    }
    return headers;
}

int ColumnManager::getVisualIndex(TableColumns column) const {
    if (column < 0 || column >= TOTAL_COLUMNS || !columnVisibility[column]) {
        return -1;
    }

    int visualIndex = 0;
    for (int i = 0; i < column; ++i) {
        if (columnVisibility[i]) {
            visualIndex++;
        }
    }
    return visualIndex;
}

QList<TableColumns> ColumnManager::getOptionalColumns() const {
    return {
        COL_VIDEO_TYPE, COL_CREATE_TIME,
        COL_UP_NAME, COL_UP_UID, COL_SERIES,
        COL_AV_NUMBER, COL_DANMAKU_UPDATE, COL_DANMAKU_COUNT
    };
}

QString ColumnManager::getColumnName(TableColumns column) const {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        return columnNames[column];
    }
    return QString();
}
