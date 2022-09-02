#include "videowidge.h"
#include <QDebug>
#include <QPainter>
VideoWidge::VideoWidge(QWidget *parent) : QWidget(parent)
{
    //设置背景色
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
    qDebug() << "using VedioWideg";
}


VideoWidge::~VideoWidge()
{
    if(image_)
    {
        uchar * bits = image_->bits();
        delete [] bits;
        delete  image_;
        image_ = nullptr;

    }
}


void VideoWidge::paintEvent(QPaintEvent *event)
{

    if(!image_)
        return ;

    //将图片绘制到当前组件上
    QPainter(this).drawImage(rect_,*image_);
    if(image_)
    {
        uchar * bits = image_->bits();
        delete [] bits;
        delete  image_;
        image_ = nullptr;

    }
}


void VideoWidge::onPlayerFrameDecoded(VideoPlayer *player,
                          uint8_t *data,
                          int width_,int height_,
                          AVPixelFormat pixFmt)
{
    if(player->getState() == VideoPlayer::Stopped) return ;
    //释放之前的图片
    if(image_)
    {
        uchar * bits = image_->bits();
        delete [] bits;
        delete  image_;
        image_ = nullptr;
    }

    //创建新的图片
    if(data != nullptr)
    {
        image_ = new QImage(data,
                            width_,height_,QImage::Format_RGB888);

        //计算最终的尺寸
        //组件尺寸
        int w = this->width();
        int h = this->height();


        int dx = 0;
        int dy = 0;
        int dw = width_;
        int dh = height_;

        // 计算目标尺寸
         if (dw > w || dh > h) { // 缩放
              if (dw * h > w * dh) { // 视频的宽高比 > 播放器的宽高比
                    dh = w * dh / dw;
                    dw = w;
               } else {
                       dw = h * dw / dh;
                       dh = h;
               }
         }

          // 居中
          dx = (w - dw) >> 1;
          dy = (h - dh) >> 1;

          rect_ = QRect(dx, dy, dw, dh);



    }

    update();
}

void VideoWidge::onPlayerStateChanged(VideoPlayer* player)
{
    if(player->getState() != VideoPlayer::Stopped) return;
    if(image_)
    {
        delete[]  image_->bits();
        delete  image_;
        image_ = nullptr;

    }

    update();
}


