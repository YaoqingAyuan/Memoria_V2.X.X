/********************************************************************************
** Form generated from reading UI file 'export_setting_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORT_SETTING_DIALOG_H
#define UI_EXPORT_SETTING_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>

QT_BEGIN_NAMESPACE

class Ui_export_setting_dialog
{
public:
    QRadioButton *exportSingle;
    QRadioButton *exportSelected;
    QRadioButton *exportAll;
    QCheckBox *remember_cheBox2;
    QPushButton *OkButton;
    QPushButton *CancelButton;
    QPushButton *ApplyButton;

    void setupUi(QDialog *export_setting_dialog)
    {
        if (export_setting_dialog->objectName().isEmpty())
            export_setting_dialog->setObjectName("export_setting_dialog");
        export_setting_dialog->resize(290, 203);
        exportSingle = new QRadioButton(export_setting_dialog);
        exportSingle->setObjectName("exportSingle");
        exportSingle->setGeometry(QRect(100, 30, 87, 19));
        exportSelected = new QRadioButton(export_setting_dialog);
        exportSelected->setObjectName("exportSelected");
        exportSelected->setGeometry(QRect(100, 60, 87, 19));
        exportAll = new QRadioButton(export_setting_dialog);
        exportAll->setObjectName("exportAll");
        exportAll->setGeometry(QRect(100, 90, 87, 19));
        remember_cheBox2 = new QCheckBox(export_setting_dialog);
        remember_cheBox2->setObjectName("remember_cheBox2");
        remember_cheBox2->setGeometry(QRect(20, 130, 151, 19));
        OkButton = new QPushButton(export_setting_dialog);
        OkButton->setObjectName("OkButton");
        OkButton->setGeometry(QRect(20, 160, 80, 21));
        CancelButton = new QPushButton(export_setting_dialog);
        CancelButton->setObjectName("CancelButton");
        CancelButton->setGeometry(QRect(105, 160, 80, 21));
        ApplyButton = new QPushButton(export_setting_dialog);
        ApplyButton->setObjectName("ApplyButton");
        ApplyButton->setGeometry(QRect(190, 160, 80, 21));

        retranslateUi(export_setting_dialog);

        QMetaObject::connectSlotsByName(export_setting_dialog);
    } // setupUi

    void retranslateUi(QDialog *export_setting_dialog)
    {
        export_setting_dialog->setWindowTitle(QCoreApplication::translate("export_setting_dialog", "Dialog", nullptr));
        exportSingle->setText(QCoreApplication::translate("export_setting_dialog", "\345\257\274\345\207\272\347\254\2541\351\241\271", nullptr));
        exportSelected->setText(QCoreApplication::translate("export_setting_dialog", "\345\257\274\345\207\272\351\200\211\344\270\255\351\241\271", nullptr));
        exportAll->setText(QCoreApplication::translate("export_setting_dialog", "\345\257\274\345\207\272\345\205\250\351\203\250", nullptr));
        remember_cheBox2->setText(QCoreApplication::translate("export_setting_dialog", "\350\256\260\344\275\217\346\210\221\347\232\204\351\200\211\346\213\251\357\274\214\344\270\215\345\206\215\346\217\220\351\206\222", nullptr));
        OkButton->setText(QCoreApplication::translate("export_setting_dialog", "\347\241\256\345\256\232(O)", nullptr));
        CancelButton->setText(QCoreApplication::translate("export_setting_dialog", "\345\217\226\346\266\210(C)", nullptr));
        ApplyButton->setText(QCoreApplication::translate("export_setting_dialog", "\345\272\224\347\224\250(A)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class export_setting_dialog: public Ui_export_setting_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORT_SETTING_DIALOG_H
