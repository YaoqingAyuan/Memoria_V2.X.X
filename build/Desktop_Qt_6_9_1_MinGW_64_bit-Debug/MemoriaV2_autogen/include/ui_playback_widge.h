/********************************************************************************
** Form generated from reading UI file 'playback_widge.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLAYBACK_WIDGE_H
#define UI_PLAYBACK_WIDGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Playback_Widge
{
public:

    void setupUi(QWidget *Playback_Widge)
    {
        if (Playback_Widge->objectName().isEmpty())
            Playback_Widge->setObjectName("Playback_Widge");
        Playback_Widge->resize(400, 300);

        retranslateUi(Playback_Widge);

        QMetaObject::connectSlotsByName(Playback_Widge);
    } // setupUi

    void retranslateUi(QWidget *Playback_Widge)
    {
        Playback_Widge->setWindowTitle(QCoreApplication::translate("Playback_Widge", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Playback_Widge: public Ui_Playback_Widge {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLAYBACK_WIDGE_H
