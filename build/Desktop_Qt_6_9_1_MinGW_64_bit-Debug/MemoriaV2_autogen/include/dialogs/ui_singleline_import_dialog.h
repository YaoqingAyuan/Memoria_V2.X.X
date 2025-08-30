/********************************************************************************
** Form generated from reading UI file 'singleline_import_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SINGLELINE_IMPORT_DIALOG_H
#define UI_SINGLELINE_IMPORT_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_singleline_import_dialog
{
public:
    QLabel *label_2;
    QLineEdit *AudioAddline;
    QLineEdit *TitlelineEdit;
    QPushButton *AudioInputButton;
    QLabel *label_3;
    QPushButton *VideoInputButton;
    QLineEdit *VideoAddline;
    QLabel *label;
    QPushButton *TitlefolderButton;
    QPushButton *OkButton;
    QPushButton *CancelButton;

    void setupUi(QDialog *singleline_import_dialog)
    {
        if (singleline_import_dialog->objectName().isEmpty())
            singleline_import_dialog->setObjectName("singleline_import_dialog");
        singleline_import_dialog->resize(400, 278);
        label_2 = new QLabel(singleline_import_dialog);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(70, 120, 71, 20));
        label_2->setAlignment(Qt::AlignmentFlag::AlignCenter);
        AudioAddline = new QLineEdit(singleline_import_dialog);
        AudioAddline->setObjectName("AudioAddline");
        AudioAddline->setGeometry(QRect(140, 120, 141, 22));
        TitlelineEdit = new QLineEdit(singleline_import_dialog);
        TitlelineEdit->setObjectName("TitlelineEdit");
        TitlelineEdit->setGeometry(QRect(150, 150, 191, 22));
        AudioInputButton = new QPushButton(singleline_import_dialog);
        AudioInputButton->setObjectName("AudioInputButton");
        AudioInputButton->setGeometry(QRect(280, 120, 61, 21));
        label_3 = new QLabel(singleline_import_dialog);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(70, 150, 81, 21));
        VideoInputButton = new QPushButton(singleline_import_dialog);
        VideoInputButton->setObjectName("VideoInputButton");
        VideoInputButton->setGeometry(QRect(280, 100, 61, 21));
        VideoAddline = new QLineEdit(singleline_import_dialog);
        VideoAddline->setObjectName("VideoAddline");
        VideoAddline->setGeometry(QRect(140, 100, 141, 22));
        label = new QLabel(singleline_import_dialog);
        label->setObjectName("label");
        label->setGeometry(QRect(70, 100, 71, 20));
        label->setAlignment(Qt::AlignmentFlag::AlignCenter);
        TitlefolderButton = new QPushButton(singleline_import_dialog);
        TitlefolderButton->setObjectName("TitlefolderButton");
        TitlefolderButton->setGeometry(QRect(60, 60, 101, 21));
        OkButton = new QPushButton(singleline_import_dialog);
        OkButton->setObjectName("OkButton");
        OkButton->setGeometry(QRect(180, 230, 80, 21));
        CancelButton = new QPushButton(singleline_import_dialog);
        CancelButton->setObjectName("CancelButton");
        CancelButton->setGeometry(QRect(265, 230, 80, 21));

        retranslateUi(singleline_import_dialog);

        QMetaObject::connectSlotsByName(singleline_import_dialog);
    } // setupUi

    void retranslateUi(QDialog *singleline_import_dialog)
    {
        singleline_import_dialog->setWindowTitle(QCoreApplication::translate("singleline_import_dialog", "Dialog", nullptr));
        label_2->setText(QCoreApplication::translate("singleline_import_dialog", "\351\237\263\351\242\221Audio:", nullptr));
        AudioInputButton->setText(QCoreApplication::translate("singleline_import_dialog", "\346\265\217\350\247\210\342\200\246", nullptr));
        label_3->setText(QCoreApplication::translate("singleline_import_dialog", "\350\247\206\351\242\221\346\240\207\351\242\230Title:", nullptr));
        VideoInputButton->setText(QCoreApplication::translate("singleline_import_dialog", "\346\265\217\350\247\210\342\200\246", nullptr));
        label->setText(QCoreApplication::translate("singleline_import_dialog", "\350\247\206\351\242\221Video:", nullptr));
        TitlefolderButton->setText(QCoreApplication::translate("singleline_import_dialog", "\346\240\207\351\242\230\346\226\207\344\273\266\345\244\271\345\257\274\345\205\245", nullptr));
        OkButton->setText(QCoreApplication::translate("singleline_import_dialog", "\347\241\256\345\256\232(O)", nullptr));
        CancelButton->setText(QCoreApplication::translate("singleline_import_dialog", "\345\217\226\346\266\210(C)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class singleline_import_dialog: public Ui_singleline_import_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SINGLELINE_IMPORT_DIALOG_H
