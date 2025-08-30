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

    bool checkFilesExist() const;
    QString generateDefaultTitle() const;
    int progress() const;

    bool hasError() const { return m_hasError; }
    void setHasError(bool error) { m_hasError = error; }

    int duration() const { return m_duration; }
    void setDuration(int duration) { m_duration = duration; }

signals:
    void dataChanged();
    void progressChanged(int progress); // 添加进度改变信号

private:
    int m_duration = 0;
    int m_index; // 行序号
    QVector<QVariant> m_data; // 存储所有列的数据
    int m_progress = 0; // 添加进度成员变量

    bool m_hasError = false; // 添加错误状态跟踪
};

#endif // VIDEOITEM_H
