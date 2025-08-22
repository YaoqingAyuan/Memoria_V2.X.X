#ifndef PLAYBACK_WIDGE_H
#define PLAYBACK_WIDGE_H

#include <QWidget>

namespace Ui {
class Playback_Widge;
}

class Playback_Widge : public QWidget
{
    Q_OBJECT

public:
    explicit Playback_Widge(QWidget *parent = nullptr);
    ~Playback_Widge();

private:
    Ui::Playback_Widge *ui;
};

#endif // PLAYBACK_WIDGE_H
