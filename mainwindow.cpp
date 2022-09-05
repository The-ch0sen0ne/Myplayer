#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include<QFileDialog>
#include<QString>
#include<QMessageBox>
#include <videowidge.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    //注册信号的参数类型，保证能够发出信号
    qRegisterMetaType<AVPixelFormat>("AVPixelFormat");

    player_ = new VideoPlayer();
    ui->VolumnSlider->setRange(VideoPlayer::Volumn::Min,
                               VideoPlayer::Volumn::Max);
    ui->VolumnSlider->setValue(50);


    //监听时间滑块
    connect(ui->CurrentSlider,&MySlider::clicked,
            this,&MainWindow::onSliderClicked);

    connect(player_,&VideoPlayer::stateChanged,
            this,&MainWindow::onPlayerStateChanged);
    connect(player_,&VideoPlayer::initFinished,
            this,&MainWindow::onPlayerinitFinished);
    connect(player_,&VideoPlayer::playFailed,
            this,&MainWindow::onPlayerPlayFailed);

    //解码完成后渲染图片
    connect(player_,&VideoPlayer::frameDecoded,
            ui->videoWidge,&VideoWidge::onPlayerFrameDecoded);

    //播放器状态改变时，改变播放界面状态，主要是停止时释放图片
    connect(player_,&VideoPlayer::stateChanged,
            ui->videoWidge,&VideoWidge::onPlayerStateChanged);

    //时间播放器发生变化时改变滑块时间
    connect(player_,&VideoPlayer::timeChanged,
            this,&MainWindow::onPlayerTimeChanged);



}

MainWindow::~MainWindow()
{
    delete ui;
    delete player_;
}

void MainWindow::onPlayerPlayFailed(VideoPlayer* player)
{
    qDebug() << player->getState();
    QMessageBox::critical(nullptr,"提示","播放失败");
}
void MainWindow::onPlayerStateChanged(VideoPlayer* player)
{
    VideoPlayer::State state = player->getState();
    if(state == VideoPlayer::Playing)
    {
        ui->PlayBtn->setText("暂停");
    }
    else
    {
        ui->PlayBtn->setText("播放");
    }

    if(state == VideoPlayer::Stopped)
    {
        ui->PlayWidget->setCurrentIndex(0);
        ui->PlayBtn->setEnabled(false);
        ui->PlayBtn->setText("播放");
        ui->StopBtn->setEnabled(false);
        ui->CurrentSlider->setEnabled(false);
        ui->VolumnSlider->setEnabled(false);
        ui->SilenceBtn->setEnabled(false);

        ui->DurationLabel->setText("00:00:00");
        ui->CurrentSlider->setValue(0);

    }
    else
    {
        ui->PlayWidget->setCurrentIndex(1);
        ui->PlayBtn->setEnabled(true);
        ui->StopBtn->setEnabled(true);
        ui->CurrentSlider->setEnabled(true);
        ui->VolumnSlider->setEnabled(true);
        ui->SilenceBtn->setEnabled(true);
    }
}

void MainWindow::on_StopBtn_clicked()
{

    player_->stop();
}

void MainWindow::on_OpenFileBtn_clicked()
{
    QString filename =  QFileDialog::getOpenFileName(nullptr,
                                 "选择多媒体文件",
                                 "E:/",
                                 "所有文件 (*.*);;"
                                 "音频文件 (*.mp3 *.aac *.flac);;"
                                 "视频文件 (*.mp4 *.mkv *.avi)");
    if(filename.isEmpty())
        return;
    qDebug() << "filename = " << filename;
    player_->setFileName(filename);
    player_->play();
}


//声音条改变时，改变显示音量并改变显示字符
void MainWindow::on_VolumnSlider_valueChanged(int value)
{
    ui->VolumnLabel->setText(QString("%1").arg(value));
    player_->setVolumn(value);
}

//进度条改变时，改变显示字符
void MainWindow::on_CurrentSlider_valueChanged(int value)
{

    ui->Curretnlabel->setText(getDurationText(value));
}

void MainWindow::on_PlayBtn_clicked()
{
    VideoPlayer::State state = player_->getState();
    if(state == VideoPlayer::Playing)
    {
        player_->pause();
    }
    else
    {
        player_->play();
    }
}

//用于初始化视频的时长信息等操作
void MainWindow::onPlayerinitFinished(VideoPlayer* player)
{

    //需要将微秒转换为00:00:00的格式
    int64_t mircoseconds = player->getDuration();
//    qDebug() << "mircoseconds = " << mircoseconds;
    ui->CurrentSlider->setRange(0,mircoseconds);
//    ui->CurrentSlider->setValue(0);
    QString str = getDurationText(mircoseconds);
    ui->DurationLabel->setText(str);
}

QString MainWindow::getDurationText(int value)
{
//    int64_t seconds = value/1000000;

//    qDebug() << seconds;
    QString h = QString("0%1").arg(value / 3600).right(2);
    QString m = QString("0%1").arg((value % 3600)/60).right(2);
    QString s = QString("0%1").arg(value % 60).right(2);

//    qDebug() << "h = " << h << " m = " << m << " s = " << s;
    QString str = QString("%1:%2:%3").arg(h).arg(m).arg(s);
    return str;
}

void MainWindow::on_SilenceBtn_clicked()
{
    if(silence_ == 0)
    {
        oririnalVolumn_ = player_->getVolumn();
        ui->VolumnSlider->setValue(0);
        ui->SilenceBtn->setText("开启声音");
        ui->VolumnSlider->setEnabled(0);
        silence_ = 1;
    }
    else
    {
        ui->VolumnSlider->setValue(oririnalVolumn_);
        ui->SilenceBtn->setText("静音");
        ui->VolumnSlider->setEnabled(1);
        silence_ = 0;
    }
}

void MainWindow::onPlayerTimeChanged(VideoPlayer* player)
{
    ui->CurrentSlider->setValue(player->getTime());
}

//点击事件，就是seek
void MainWindow::onSliderClicked(MySlider* slider)
{
    //改变seek时间
    player_->setSeekTime(slider->value());
}


