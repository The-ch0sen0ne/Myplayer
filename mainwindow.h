#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "videowidge.h"
#include "videoplayer.h"
#include "myslider.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void onSliderClicked(MySlider *slider);

    void onPlayerTimeChanged(VideoPlayer* player);

    void onPlayerinitFinished(VideoPlayer* player);

    void onPlayerStateChanged(VideoPlayer* player);

    void onPlayerPlayFailed(VideoPlayer* player);

    void on_StopBtn_clicked();

    void on_OpenFileBtn_clicked();

    void on_VolumnSlider_valueChanged(int value);

    void on_CurrentSlider_valueChanged(int value);

    void on_PlayBtn_clicked();


    void on_SilenceBtn_clicked();

private:
    Ui::MainWindow *ui;
    VideoPlayer *player_;
    QString getDurationText(int value);
    bool silence_ = 0;
    int oririnalVolumn_ = 50;


};
#endif // MAINWINDOW_H
