#include "videoitem.h"

VideoItem::VideoItem(int index, QObject *parent)
    : QObject(parent), m_index(index)
{
    m_data.resize(TOTAL_COLUMNS);

    // 初始化默认值
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        m_data[i] = "<空>";
    }


    // 设置特殊列默认值
    m_data[COL_INDEX] = index;
    m_data[COL_PROGRESS] = 0; // 进度条初始0
}

QVariant VideoItem::data(TableColumns column) const {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        return m_data[column];
    }
    return QVariant();
}

void VideoItem::setData(TableColumns column, const QVariant &value) {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        m_data[column] = value;
    }
}

void VideoItem::setProgress(int progress) {
    if (progress < 0) progress = 0;
    if (progress > 100) progress = 100;
    m_data[COL_PROGRESS] = progress;
    emit dataChanged();
}

void VideoItem::setTitle(const QString &title) {
    m_data[COL_TITLE] = title;
    emit dataChanged();
}

void VideoItem::setIndex(int index)
{
    m_index = index;
    m_data[COL_INDEX] = index;
    emit dataChanged();
}
