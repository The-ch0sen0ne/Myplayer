#include "videoplayer.h"
#include<thread>

#define AUDIO_MAX_PKT_SIZE 1000
#define VIDEO_MAX_PKT_SIZE 1000


/*****************构造析构***********************/

VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent),
    audioPacketList(new std::list<AVPacket>),videoPacketList(new std::list<AVPacket>)
{
    //初始化SDL音频系统
    if(SDL_Init(SDL_INIT_AUDIO))
    {
        qDebug() << "SDL_Init Error" << SDL_GetError();
        emit playFailed(this);
        return;
    }

}

VideoPlayer::~VideoPlayer()
{
    stop();
    delete audioPacketList;
    delete videoPacketList;
    //清除所有SDL系统
    SDL_Quit();
}



/*****************Public***********************/
void VideoPlayer::play()
{
    if(state_ == Playing) return ;


    //解封装、解码、播放、音视频同步
    //子线程解码
    //解码后转换格式（重采样）
    //先在子线程中读取文件内容
    if(state_ == Stopped)
    {
        std::thread readthread([=]()
        {
            readFile();
        });
        readthread.detach();
    }

    setState(Playing);
}

void VideoPlayer::pause()
{
    if (state_ != Playing) return;
    // 状态可能是：正在播放

    // 改变状态
    setState(Pasued);
}

void VideoPlayer::stop()
{
    if(state_ == Stopped) return;

//    //延迟以确保子线程得知Stopped
//    std::thread([this](){
//        setState(Stopped);
//        free();
//    }).detach();
      setState(Stopped);
      free();

}

int64_t VideoPlayer::getDuration()
{

    return fmtCtx_?  round(fmtCtx_->duration * av_q2d(AV_TIME_BASE_Q) ): 0;
}





/*****************Private***********************/



void VideoPlayer::setState(State state)
{
    if(state == state_) return ;
    state_ = state;
    //状态改变时发送信号通知窗口
    emit stateChanged(this);
}

 void VideoPlayer::setFileName(QString& name)
 {
    const char * filename = name.toStdString().c_str();
    memcpy(filename_,filename,strlen(filename)+1);
 }


 void VideoPlayer::readFile()
 {

     //创建解封装上下文、打开文件
    int ret = avformat_open_input(&fmtCtx_,filename_,nullptr,nullptr);
//    qDebug() << "filename = " << filename_;
    END(avformat_open_input);

    //检索流信息
    ret = avformat_find_stream_info(fmtCtx_,nullptr);
    END(avformat_find_stream_info);

    //打印流信息到控制台
    av_dump_format(fmtCtx_,0,filename_,0);
    fflush(stderr);

    //初始化音频信息
    bool hasAudio = initAudioInfo()>= 0;

    //初始化视频信息
    bool hasVideo = ret = initVideoInfo() >= 0;
    if(!hasAudio&&!hasVideo)
    {
        fataError();
        return ;
    }


    //初始化完毕，获取视频信息
    emit initFinished(this);

    //开启SDL子线程解码并播放音频
    SDL_PauseAudio(0);

    //开启视频子线程解码并播放视频
    std::thread videothread([=](){decodeVideo();});
    videothread.detach();

    //从输入文件中读取数据,开始解码
    while(state_ != Stopped)
    {

        //用于seek
        if (seekTime_ >= 0) {
             int streamIdx;
             if (hasAudio) { // 优先使用音频流索引
                    streamIdx = audioStream_->index;
             } else {
                    streamIdx = videoStream_->index;
             }

              // 现实时间 -> 时间戳
              AVRational timeBase = fmtCtx_->streams[streamIdx]->time_base;
              int64_t ts = seekTime_ / av_q2d(timeBase);
              ret = av_seek_frame(fmtCtx_, streamIdx, ts, AVSEEK_FLAG_BACKWARD);

              if (ret < 0) { // seek失败
                   qDebug() << "seek失败" << seekTime_ << ts << streamIdx;
                   seekTime_ = -1;
              } else {
                  qDebug() << "seek成功" << seekTime_ << ts << streamIdx;
            videoSeekTime_ = seekTime_;
            audioSeekTime_ = seekTime_;
              seekTime_ = -1;
             // 恢复时钟
             audioClock_ = 0;
             videoClock_ = 0;
             // 清空之前读取的数据包
             clearAudioList();
             clearVideoList();
              }
     }
        int aSize = audioPacketList->size();
        int vSize = videoPacketList->size();

        if(aSize >= AUDIO_MAX_PKT_SIZE ||
           vSize >= VIDEO_MAX_PKT_SIZE)
        {

            continue;
        }


        AVPacket pkt;
        ret = av_read_frame(fmtCtx_,&pkt);
        if(ret == 0)
        {
            if(pkt.stream_index == audioStream_->index)
            {
                addAudioPkt(pkt);
            }
            else if(pkt.stream_index == videoStream_->index)
            {

                addVideoPkt(pkt);
            }
            else//非音频和视频流时直接释放
            {
                av_packet_unref(&pkt);
            }
        }
        else if(ret == AVERROR_EOF)
        {
//            qDebug() << "已经读取到文件尾部";
            if (vSize == 0 && aSize == 0) {
                // 说明文件正常播放完毕
                fmtCanFree_ = true;
                break;
            }
        }
        else
        {
            ERROR_BUF;
            qDebug() << "av_read_frame error" << errbuf;
            continue;
        }
    }

    if(fmtCanFree_)
    {
        stop();
    }
    else
    {
        fmtCanFree_ = true;
    }




 }





 int VideoPlayer::initDecoder(AVCodecContext* &decodeCtx,
                 AVStream* &stream,
                 AVMediaType type)
 {
    //先根据流的类型找到最适合的流信息
     int ret = av_find_best_stream(fmtCtx_,type,-1,-1,nullptr,0);
     RET(av_find_best_stream);

     stream = fmtCtx_->streams[ret];
     if(!stream)
     {
         qDebug() << "stream is empty";
         return -1;
     }

     //找到当前流最适合的解码器
    const AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if(!decoder)
    {
           qDebug() << "decoder not found" << stream->codecpar->codec_id;
           return -1;
    }

    //初始化解码上下文
    //先为解码器上下文分配空间
    decodeCtx = avcodec_alloc_context3(decoder);
    if(!decodeCtx)
    {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    //从流中拷贝参数到解码器上下文
    ret = avcodec_parameters_to_context(decodeCtx,stream->codecpar);
    RET(avcodec_parameters_to_context);

    //打开解码器
    ret =  avcodec_open2(decodeCtx,decoder,nullptr);
    RET(avcodec_open2);

    return 0;
 }


 void VideoPlayer::free()
 {
    while(audioStream_ && !audioCanFree_);
    while(videoStream_ && !videoCanFree_);
    while(!fmtCanFree_);

    avformat_close_input(&fmtCtx_);
    freeAudio();
    freeVideo();
    audioCanFree_ = 0;
    videoCanFree_ = 0;
    fmtCanFree_ = 0;
    seekTime_ = -1;
 }


 void VideoPlayer::setVolumn(int volumn)
 {
    volumn_ = volumn;
 }

 void VideoPlayer::fataError() {
     // 配合stop能够调用成功
     state_ = Playing;
     stop();
     emit playFailed(this);
 }

