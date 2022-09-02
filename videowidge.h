#ifndef VEDIOWIDGE_H
#define VEDIOWIDGE_H

#include <QWidget>
#include<QImage>
#include "videoplayer.h"

class VideoPlayer;
//负责渲染视频
class VideoWidge : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidge(QWidget *parent = nullptr);

    ~VideoWidge();
public slots:
    void onPlayerFrameDecoded(VideoPlayer *player,
                              uint8_t *data,
                              int width_,int height_,
                              AVPixelFormat pixFmt);
    void onPlayerStateChanged(VideoPlayer* player);

private:
    QImage *image_ = nullptr;
    QRect rect_;
    void paintEvent(QPaintEvent *event) override;
signals:

};

#endif // VEDIOWIDGE_H
