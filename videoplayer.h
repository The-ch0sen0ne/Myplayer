#ifndef VEDIOPLAYER_H
#define VEDIOPLAYER_H

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include <QObject>
#include <list>
#include <mutex>
#include <condition_variable>
#include <QDebug>
#include <SDL2/SDL.h>
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/avutil.h>
    #include <libswresample/swresample.h>
    #include<libswscale/swscale.h>
}


#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

#define CODE(func, code) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        code; \
    }



#define END(func) CODE(func, fataError(); return;)

#define RET(func) CODE(func, return ret;)
#define CONTINUE(func) CODE(func, continue;)
#define BREAK(func) CODE(func, break;)

//负责预处理视频
class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    //状态
    enum State
    {
        Stopped = 0,
        Playing,
        Pasued
    };
    enum Volumn
    {
        Min = 0,
        Max = 100
    };

    explicit VideoPlayer(QObject *parent = nullptr);
    ~VideoPlayer();
    void play();

    void pause();

    void stop();

    void setFileName(QString& name);

    //获取总时长,单位是微秒(1秒 = 1*10^6微秒)
    int64_t getDuration();
    State getState()
    {
        return state_;
    }

    void setVolumn(int volumn);
    int getVolumn()
    {
        return volumn_;
    }

    //单位是秒
    int getTime()
    {
        return round(audioClock_);
    }

    void setSeekTime(int seektime)
    {
        seekTime_ = seektime;
//        qDebug() << seekTime_;
    }
signals:
    //时间发生改变
    void timeChanged(VideoPlayer *player);
    //播放器状态改变信号
    void stateChanged(VideoPlayer *player);
    //视频信息读取完成信号
    void initFinished(VideoPlayer *player);
    //发送播放失败信号
    void playFailed(VideoPlayer *player);
    //视频解码成功
    void frameDecoded(VideoPlayer *player,
                      uint8_t *data,
                      int width,int height,
                      AVPixelFormat pixFmt);

private:
    /*********音频相关变量************/

    struct AudioSwrSpec
    {
        int sampleRate;
        AVSampleFormat sampleFmt;
        //声道布局
        int chLayout;
        //声道数
        int chs;
        int bytesPerSampleFrame;
    };
    //解码上下文
    AVCodecContext *audioDecodeCtx_ = nullptr;
    //流数据
    AVStream *audioStream_ = nullptr;
    //存放音频包的列表
    std::list<AVPacket> *audioPacketList = nullptr;
    //音频队列的锁
    std::mutex audioMutex_;
    //音频队列的条件变量
    std::condition_variable audioCon_;
    //重采样上下文
    SwrContext *audioSwrCtx_ = nullptr;
    //重采样输入参数
    AudioSwrSpec audioSwrInSpec_;
    //重采样输出参数
    AudioSwrSpec audioSwrOutSpec_;
    //解码后的帧数据(包含多个帧)
    AVFrame *audioSwrInFrame_ = nullptr;
    //重采样用于播放的帧数据
    AVFrame *audioSwrOutFrame_ = nullptr;
    //音频重采样后PCM缓冲区的下标索引
    int audioSwrOutIdx_ = 0;
    //音频重采样后PCM缓冲区的大小
    int audioSwrOutSize_ = 0;
    //当前音频包对应时间值
    double audioClock_ = 0;
    //音频资源是否可以释放
    bool audioCanFree_ = 0;
    //外面设置的当前音频播放时刻
    int audioSeekTime_ = -1;


    /*********视频相关变量************/
    struct VideoSwsSpec
    {
        int height;
        int width;
        int size;
        AVPixelFormat pixFmt;
    };

    //解码上下文
    AVCodecContext *videoDecodeCtx_ = nullptr;
    //流数据
    AVStream *videoStream_ = nullptr;
    //格式转换的上下文
    SwsContext *videoSwsCtx_ = nullptr;
    //解码后的帧数据(包含多个帧)
    AVFrame *videoSwsInFrame_ = nullptr;
    //格式转换后用于播放的帧数据
    AVFrame *videoSwsOutFrame_ = nullptr;
    //存放视频包的列表
    std::list<AVPacket> *videoPacketList = nullptr;
    //格式转换输入参数
    VideoSwsSpec videoSwsInSpec_;
    //格式转换输出参数
    VideoSwsSpec videoSwsOutSpec_;
    //视频队列的锁
    std::mutex videoMutex_;
    //视频队列的条件变量
    std::condition_variable videoCon_;
    int volumn_ = 50;
    //当前视频包对应时间值
    double videoClock_ = 0;
    //视频资源是否可以释放
    bool videoCanFree_ = 0;
    //外面设置的当前视频播放时刻
    int videoSeekTime_ = -1;
    //帧率
    double fps;

    /*********公共部分变量************/
    State state_ = Stopped;
    char filename_[512];
    //解封装上下文
     AVFormatContext *fmtCtx_ = nullptr;
     //fmtCtx是否可以释放
     bool fmtCanFree_ = 0;

     int seekTime_ = -1;

     /*********音频相关函数************/
     //初始化音频信息
     int initAudioInfo();
     //初始化音频重采样
     int initSwr();
     void addAudioPkt(AVPacket& pkt);
     //初始化SDL
     int initSDL();
     //实际用于填充SDL音频缓冲区的函数
     void sdlAudioCallback(Uint8 *stream,int len);
     //供SDL填充音频缓冲区时调用的回调函数
     static void sdlAudioCallbackFunc(void *userdata,Uint8 *stream,int len);
     //音频解码
     int decodeAudio();
     void freeAudio();


     /*********视频相关函数************/
     //初始化视频信息
     int initVideoInfo();
     void addVideoPkt(AVPacket& pkt);
     //清空音频包列表,停止播放器时调用
     void clearAudioList();
     void freeVideo();
     void decodeVideo();
     //初始化视频格式转换
     int initSws();





     /*********公共部分函数************/
     void setState(State state);
     void readFile();
     //清空视频包列表,停止播放器时调用
     void clearVideoList();




     //初始化解码器和解码上下文
     int initDecoder(AVCodecContext* &decodeCtx,
                    AVStream* &stream,
                    AVMediaType type);
    void fataError();
     void free();



};

#endif // VEDIOPLAYER_H
