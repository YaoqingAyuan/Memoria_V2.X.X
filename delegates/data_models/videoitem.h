#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include "tablecolumns.h"

class VideoItem : public QObject
{
    Q_OBJECT
public:
    explicit VideoItem(int index, QObject *parent = nullptr);

    // 获取/设置数据
    QVariant data(TableColumns column) const;
    void setData(TableColumns column, const QVariant &value);

    // 特殊属性处理
    int index() const { return m_index; }
    void setProgress(int progress);
    void setTitle(const QString &title);

    void setIndex(int index);

signals:
    void dataChanged();

private:
    int m_index; // 行序号
    QVector<QVariant> m_data; // 存储所有列的数据
};

#endif // VIDEOITEM_H
