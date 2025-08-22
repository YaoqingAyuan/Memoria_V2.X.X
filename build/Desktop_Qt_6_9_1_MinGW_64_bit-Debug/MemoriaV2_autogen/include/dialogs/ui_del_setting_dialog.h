/********************************************************************************
** Form generated from reading UI file 'del_setting_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEL_SETTING_DIALOG_H
#define UI_DEL_SETTING_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_del_setting_dialog
{
public:
    QRadioButton *del_only_line;
    QRadioButton *del_selected_line;
    QRadioButton *del_Allline;
    QCheckBox *remember_cheBox;
    QPushButton *OkButton;
    QPushButton *CancelButton;
    QPushButton *ApplyButton;

    void setupUi(QDialog *del_setting_dialog)
    {
        if (del_setting_dialog->objectName().isEmpty())
            del_setting_dialog->setObjectName("del_setting_dialog");
        del_setting_dialog->resize(290, 203);
        del_only_line = new QRadioButton(del_setting_dialog);
        del_only_line->setObjectName("del_only_line");
        del_only_line->setGeometry(QRect(100, 40, 87, 19));
        del_selected_line = new QRadioButton(del_setting_dialog);
        del_selected_line->setObjectName("del_selected_line");
        del_selected_line->setGeometry(QRect(100, 70, 87, 19));
        del_Allline = new QRadioButton(del_setting_dialog);
        del_Allline->setObjectName("del_Allline");
        del_Allline->setGeometry(QRect(100, 100, 87, 19));
        remember_cheBox = new QCheckBox(del_setting_dialog);
        remember_cheBox->setObjectName("remember_cheBox");
        remember_cheBox->setGeometry(QRect(20, 170, 161, 19));
        OkButton = new QPushButton(del_setting_dialog);
        OkButton->setObjectName("OkButton");
        OkButton->setGeometry(QRect(21, 141, 80, 21));
        CancelButton = new QPushButton(del_setting_dialog);
        CancelButton->setObjectName("CancelButton");
        CancelButton->setGeometry(QRect(106, 141, 80, 21));
        ApplyButton = new QPushButton(del_setting_dialog);
        ApplyButton->setObjectName("ApplyButton");
        ApplyButton->setGeometry(QRect(191, 141, 80, 21));

        retranslateUi(del_setting_dialog);

        QMetaObject::connectSlotsByName(del_setting_dialog);
    } // setupUi

    void retranslateUi(QDialog *del_setting_dialog)
    {
        del_setting_dialog->setWindowTitle(QCoreApplication::translate("del_setting_dialog", "Dialog", nullptr));
        del_only_line->setText(QCoreApplication::translate("del_setting_dialog", "\345\210\240\351\231\2441\351\241\271", nullptr));
        del_selected_line->setText(QCoreApplication::translate("del_setting_dialog", "\345\210\240\351\231\244\351\200\211\344\270\255\351\241\271", nullptr));
        del_Allline->setText(QCoreApplication::translate("del_setting_dialog", "\345\210\240\351\231\244\346\211\200\346\234\211", nullptr));
        remember_cheBox->setText(QCoreApplication::translate("del_setting_dialog", "\350\256\260\344\275\217\346\210\221\347\232\204\351\200\211\346\213\251\357\274\214\344\270\215\345\206\215\346\217\220\351\206\222", nullptr));
        OkButton->setText(QCoreApplication::translate("del_setting_dialog", "\347\241\256\345\256\232(O)", nullptr));
        CancelButton->setText(QCoreApplication::translate("del_setting_dialog", "\345\217\226\346\266\210(C)", nullptr));
        ApplyButton->setText(QCoreApplication::translate("del_setting_dialog", "\345\272\224\347\224\250(A)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class del_setting_dialog: public Ui_del_setting_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEL_SETTING_DIALOG_H
