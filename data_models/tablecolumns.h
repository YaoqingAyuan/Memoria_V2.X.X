#ifndef TABLECOLUMNS_H
#define TABLECOLUMNS_H
#include <QVector>
#include <QStringList>
#include <QSet>

// 表格列枚举定义（按显示顺序）
enum TableColumns {
    // 强制显示列 (8列)
    COL_INDEX,          // 序号
    COL_VIDEO_TYPE,     // 视频类型 (可选，但位置固定)
    COL_TITLE,          // 视频标题
    COL_CREATE_TIME,    // 创建时间 (可选，但位置固定)
    COL_DURATION,       // 视频时长
    COL_TOTAL_SIZE,     // 文件总大小
    COL_QUALITY,        // 清晰度描述
    COL_PROGRESS,       // 混流进度条

    COL_VIDEO_FILE,     // 视频.m4s文件导入
    COL_AUDIO_FILE,     // 音频.m4s文件导入

    // 可选显示列 (8列)
    COL_UP_NAME,        // UP主昵称 (在进度条后)
    COL_UP_UID,         // UP主UID
    COL_SERIES,         // 所属系列
    COL_AV_NUMBER,      // 视频av号
    COL_DANMAKU_UPDATE, // 最近弹幕更新时间
    COL_DANMAKU_COUNT,  // 最近更新时弹幕数

    TOTAL_COLUMNS       // 总列数 (16列)
};

class ColumnManager
{
public:
    ColumnManager();

    // 设置列可见性（强制列不可修改）
    void setColumnVisibility(TableColumns column, bool visible);
    bool isColumnVisible(TableColumns column) const;

    // 获取所有可见列的标题列表
    QStringList getVisibleHeaders() const;

    // 获取列在实际表格中的位置索引
    int getVisualIndex(TableColumns column) const;

    // 获取所有可选列的类型
    QList<TableColumns> getOptionalColumns() const;

    // 获取列名
    QString getColumnName(TableColumns column) const;

private:
    bool isForcedColumn(TableColumns column) const;

private:
    QVector<bool> columnVisibility;
    QStringList columnNames;
};

#endif // TABLECOLUMNS_H
