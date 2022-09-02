#include "videoplayer.h"
#include<QDebug>
extern "C"
{
    #include <libavutil/imgutils.h>
    #include <libavutil/time.h>
}

#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1

int VideoPlayer::initVideoInfo()
{
    //初始化视频解码器
    int ret = initDecoder(videoDecodeCtx_,videoStream_,AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);
    //初始化sws上下文
    ret = initSws();
    RET(initSws);


    //计算fps
    if(videoStream_)
        fps = (double) videoStream_->avg_frame_rate.num / (double) videoStream_->avg_frame_rate.den;
    return 0;
}


int VideoPlayer::initSws()
{
    videoSwsInSpec_.width = videoDecodeCtx_->width;
    videoSwsInSpec_.height = videoDecodeCtx_->height;
    videoSwsInSpec_.pixFmt = videoDecodeCtx_->pix_fmt;



    videoSwsOutSpec_.width = videoDecodeCtx_->width >> 4 << 4;
    videoSwsOutSpec_.height = videoDecodeCtx_->height >> 4 << 4;
    videoSwsOutSpec_.pixFmt = AV_PIX_FMT_RGB24;
    videoSwsOutSpec_.size = av_image_get_buffer_size(videoSwsOutSpec_.pixFmt,
                                                     videoSwsOutSpec_.width,
                                                     videoSwsOutSpec_.height,1);

    //初始化像素格式转换的上下文
    videoSwsCtx_ = sws_getContext(videoSwsInSpec_.width,
                                  videoSwsInSpec_.height,
                                  videoSwsInSpec_.pixFmt,
                                  videoSwsOutSpec_.width,
                                  videoSwsOutSpec_.height,
                                  videoSwsOutSpec_.pixFmt,
                                  SWS_BILINEAR,nullptr,nullptr,nullptr);

    if(!videoSwsCtx_)
    {
        qDebug() << "videoSwsCtx_ error";
        return -1;
    }
    //初始化视频像素格式转换的输入frame
    videoSwsInFrame_ = av_frame_alloc();
    if(!videoSwsInFrame_)
    {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    videoSwsOutFrame_ = av_frame_alloc();
    if(!videoSwsOutFrame_)
    {
        qDebug() << "av_frame_alloc error";
        return -1;
    }


    //初始化重采样的输出frame的data[0]空间
    int ret = av_image_alloc(videoSwsOutFrame_->data,
                             videoSwsOutFrame_->linesize,
                             videoSwsOutSpec_.width,
                             videoSwsOutSpec_.height,
                             videoSwsOutSpec_.pixFmt,
                             1);
    RET(av_image_alloc);
    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt)
{
    std::lock_guard<std::mutex> lock(videoMutex_);
    videoPacketList->push_back(pkt);
//    videoCon_.notify_all();
}





void VideoPlayer::clearVideoList()
{
    std::lock_guard<std::mutex> lock(videoMutex_);
    for(AVPacket &pkt:*videoPacketList)
    {
        av_packet_unref(&pkt);
    }
    videoPacketList->clear();
}


//解码部分
void VideoPlayer::decodeVideo()
{
    //帧延迟
    qDebug() << "fps = " << fps ;
    double frame_delay = 1.0/fps;
    while(1)
    {

//        if(state_ == Pasued && videoSeekTime_ == -1)
//        {
//            continue;
//        }

        if(state_ == Pasued)
        {
            continue;
        }

        if(state_ == Stopped)
        {
            videoCanFree_ = 1;
            break;
        }


        AVPacket pkt;
        {
            std::lock_guard<std::mutex> lock(videoMutex_);
            if(videoPacketList->empty())
                continue;
            pkt = videoPacketList->front();
            videoPacketList->pop_front();
        }
        //向解码器发送数据包
        int ret = avcodec_send_packet(videoDecodeCtx_,&pkt);

        //视频时钟
        if(pkt.pts != AV_NOPTS_VALUE)
        {
            videoClock_ = av_q2d(videoStream_->time_base)*pkt.pts;
//            qDebug() << "videoClock_ = " << videoClock_ ;
        }


        //释放pkt
        av_packet_unref(&pkt);
        CONTINUE(avcodec_send_packet);
        while(1)
        {
            //从解码器中读取解码后的帧
            ret = avcodec_receive_frame(videoDecodeCtx_,videoSwsInFrame_);
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else
              BREAK(avcodec_receive_frame);



            //用于解决seek后有几帧闪的特别快
            // 一定要在解码成功后，再进行下面的判断
            // 发现视频的时间是早于seekTime的，直接丢弃
            if (videoSeekTime_ >= 0) {
                if (videoClock_ < videoSeekTime_) {
                    continue;
                } else {
                    videoSeekTime_ = -1;
                }
            }



            //QT渲染指定需要RGB24格式
            //需要转换像素格式
            ret = sws_scale(videoSwsCtx_,videoSwsInFrame_->data,videoSwsInFrame_->linesize,
                      0,videoSwsInSpec_.height,videoSwsOutFrame_->data,videoSwsOutFrame_->linesize);

            //音视频同步
            //解码延迟
            double extra_delay = videoSwsOutFrame_->repeat_pict/(2*fps) ;
            //解码延迟+帧延迟
            double delay = extra_delay + frame_delay;

            double diff = videoClock_ - audioClock_;
            if(diff > 10)
            continue;
            //可接受误差
            double sync = FFMAX(AV_SYNC_THRESHOLD_MIN,FFMIN(AV_SYNC_THRESHOLD_MAX,delay));
            if(diff <= -sync)
            {
                //视频慢了
                delay = FFMAX(0,delay + diff);
            }
            else if(diff > sync)
            {
                delay = delay + diff;
            }
            qDebug() << "video clock = " << videoClock_ << "audio clock = " << audioClock_;
            av_usleep(delay * 1000000);

//            while (videoClock_ > audioClock_ && state_ == Playing) ;


            //拷贝data[0]避免读取时发生竞争条件
            int size = videoSwsOutSpec_.size;
            uint8_t *data = new uint8_t[size];
            memcpy(data,videoSwsOutFrame_->data[0],size);

            //通知ui线程
            emit frameDecoded(this,data,
                              videoSwsOutSpec_.width,
                              videoSwsOutSpec_.height,
                              videoSwsOutSpec_.pixFmt);
        }


    }
}



void VideoPlayer::freeVideo()
{
    videoClock_ = 0;
    videoStream_ = nullptr;
    clearVideoList();
    avcodec_free_context(&videoDecodeCtx_);
    sws_freeContext(videoSwsCtx_);
    videoSwsCtx_ = nullptr;
    av_frame_free(&videoSwsInFrame_);
    if(videoSwsOutFrame_)
    {
        //av_image_alloc创建的内容需要手动释放
        av_freep(&videoSwsOutFrame_->data[0]);
        av_frame_free(&videoSwsOutFrame_);
    }
}
