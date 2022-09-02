/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <myslider.h>
#include <videowidge.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QStackedWidget *PlayWidget;
    QWidget *page;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *OpenFileBtn;
    QWidget *page_2;
    QGridLayout *gridLayout;
    VideoWidge *videoWidge;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *PlayBtn;
    QPushButton *StopBtn;
    QHBoxLayout *horizontalLayout_3;
    MySlider *CurrentSlider;
    QLabel *Curretnlabel;
    QLabel *label_2;
    QLabel *DurationLabel;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer;
    QPushButton *SilenceBtn;
    MySlider *VolumnSlider;
    QLabel *VolumnLabel;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(873, 471);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        PlayWidget = new QStackedWidget(centralwidget);
        PlayWidget->setObjectName(QString::fromUtf8("PlayWidget"));
        PlayWidget->setMinimumSize(QSize(160, 90));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        horizontalLayout_5 = new QHBoxLayout(page);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        OpenFileBtn = new QPushButton(page);
        OpenFileBtn->setObjectName(QString::fromUtf8("OpenFileBtn"));
        OpenFileBtn->setMinimumSize(QSize(80, 0));
        OpenFileBtn->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_5->addWidget(OpenFileBtn);

        PlayWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        gridLayout = new QGridLayout(page_2);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        videoWidge = new VideoWidge(page_2);
        videoWidge->setObjectName(QString::fromUtf8("videoWidge"));

        gridLayout->addWidget(videoWidge, 0, 0, 1, 1);

        PlayWidget->addWidget(page_2);

        verticalLayout->addWidget(PlayWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        PlayBtn = new QPushButton(centralwidget);
        PlayBtn->setObjectName(QString::fromUtf8("PlayBtn"));
        PlayBtn->setEnabled(false);

        horizontalLayout_2->addWidget(PlayBtn);

        StopBtn = new QPushButton(centralwidget);
        StopBtn->setObjectName(QString::fromUtf8("StopBtn"));
        StopBtn->setEnabled(false);
        StopBtn->setMinimumSize(QSize(0, 0));

        horizontalLayout_2->addWidget(StopBtn);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        CurrentSlider = new MySlider(centralwidget);
        CurrentSlider->setObjectName(QString::fromUtf8("CurrentSlider"));
        CurrentSlider->setMinimumSize(QSize(200, 0));
        CurrentSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_3->addWidget(CurrentSlider);

        Curretnlabel = new QLabel(centralwidget);
        Curretnlabel->setObjectName(QString::fromUtf8("Curretnlabel"));
        Curretnlabel->setMinimumSize(QSize(100, 0));
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        Curretnlabel->setFont(font);
        Curretnlabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(Curretnlabel);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(5, 0));
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(label_2);

        DurationLabel = new QLabel(centralwidget);
        DurationLabel->setObjectName(QString::fromUtf8("DurationLabel"));
        DurationLabel->setMinimumSize(QSize(100, 0));
        DurationLabel->setFont(font);
        DurationLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(DurationLabel);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);

        SilenceBtn = new QPushButton(centralwidget);
        SilenceBtn->setObjectName(QString::fromUtf8("SilenceBtn"));
        SilenceBtn->setEnabled(false);

        horizontalLayout_4->addWidget(SilenceBtn);

        VolumnSlider = new MySlider(centralwidget);
        VolumnSlider->setObjectName(QString::fromUtf8("VolumnSlider"));
        VolumnSlider->setMinimumSize(QSize(100, 0));
        VolumnSlider->setMaximumSize(QSize(100, 16777215));
        VolumnSlider->setMaximum(100);
        VolumnSlider->setSliderPosition(50);
        VolumnSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(VolumnSlider);

        VolumnLabel = new QLabel(centralwidget);
        VolumnLabel->setObjectName(QString::fromUtf8("VolumnLabel"));
        VolumnLabel->setFont(font);

        horizontalLayout_4->addWidget(VolumnLabel);


        horizontalLayout_3->addLayout(horizontalLayout_4);


        horizontalLayout_2->addLayout(horizontalLayout_3);


        horizontalLayout->addLayout(horizontalLayout_2);


        verticalLayout->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 873, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        PlayWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MyPlayer", nullptr));
        OpenFileBtn->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
        PlayBtn->setText(QCoreApplication::translate("MainWindow", "\346\222\255\346\224\276", nullptr));
        StopBtn->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        Curretnlabel->setText(QCoreApplication::translate("MainWindow", "00\357\274\23200\357\274\23200", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        DurationLabel->setText(QCoreApplication::translate("MainWindow", "00\357\274\23200\357\274\23200", nullptr));
        SilenceBtn->setText(QCoreApplication::translate("MainWindow", "\351\235\231\351\237\263", nullptr));
        VolumnLabel->setText(QCoreApplication::translate("MainWindow", "50", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
