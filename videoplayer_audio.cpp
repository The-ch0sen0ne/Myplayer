#include "videoplayer.h"


//初始化音频信息
int VideoPlayer::initAudioInfo()
{
    //先初始化解码上下文和解码器
   int ret = initDecoder(audioDecodeCtx_,audioStream_,AVMEDIA_TYPE_AUDIO);
   RET(initDecoder);
   //初始化音频重采样
   ret = initSwr();
   RET(initSwr);
   //初始化SDL
   ret = initSDL();
   RET(initSwr);

   return 0;

}


void VideoPlayer::freeAudio()
{
    audioClock_ = 0;
    audioSwrOutIdx_ = 0;
    audioSwrOutSize_ = 0;
    audioStream_ = nullptr;
    clearAudioList();
    avcodec_free_context(&audioDecodeCtx_);
    swr_free(&audioSwrCtx_);
    av_frame_free(&audioSwrInFrame_);
    if(audioSwrOutFrame_)
    {
        //只有OutFrame需要freedata[0]是因为解码调用的avcodec_receive_frame会在开始时自动调用av_frame_unref
        //OutFrame是av_samples_alloc开辟的需要手动释放
        av_freep(&audioSwrOutFrame_->data[0]);
        av_frame_free(&audioSwrOutFrame_);
    }

    //停止播放
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}

int VideoPlayer::initSwr()
{
    //重采样参数
    audioSwrInSpec_.sampleFmt = audioDecodeCtx_->sample_fmt;
    audioSwrInSpec_.sampleRate = audioDecodeCtx_->sample_rate;
    audioSwrInSpec_.chLayout = audioDecodeCtx_->channel_layout;
    audioSwrInSpec_.chs = audioDecodeCtx_->channels;

    audioSwrOutSpec_.sampleFmt = AV_SAMPLE_FMT_S16;
    audioSwrOutSpec_.sampleRate = 44100;
    audioSwrOutSpec_.chLayout = AV_CH_LAYOUT_STEREO;
    audioSwrOutSpec_.chs = 2;
    audioSwrOutSpec_.bytesPerSampleFrame = audioSwrOutSpec_.chs
            *av_get_bytes_per_sample(audioSwrOutSpec_.sampleFmt);
    //创建重采样上下文
    audioSwrCtx_ = swr_alloc_set_opts(nullptr,
                                      audioSwrOutSpec_.chLayout,audioSwrOutSpec_.sampleFmt,audioSwrOutSpec_.sampleRate,
                                      audioSwrInSpec_.chLayout,audioSwrInSpec_.sampleFmt,audioSwrInSpec_.sampleRate,
                                      0,nullptr);
    if(!audioSwrCtx_)
    {
        qDebug() << "audioSwrCtx_ error";
    }

    //初始化重采样上下文
    int ret = swr_init(audioSwrCtx_);
    RET(swr_init);

    //初始化输入Frame
    audioSwrInFrame_ = av_frame_alloc();
    if(!audioSwrInFrame_)
    {
            qDebug() << "av_frame_alloc error";
            return -1;
    }

    //初始化输出Frame,此时data[0]指向nullptr
    audioSwrOutFrame_ = av_frame_alloc();
    if(!audioSwrOutFrame_)
    {
            qDebug() << "av_frame_alloc error";
            return -1;
    }
//    qDebug() << "before data[0] = " << audioSwrOutFrame_->data[0];
//    qDebug() << "before linesize[0] = " << audioSwrOutFrame_->linesize[0];

    //初始化重采样的输出frame的data[0]空间，4096是随便写的一个一定不会超的数
    ret = av_samples_alloc(audioSwrOutFrame_->data,
                     audioSwrOutFrame_->linesize,
                     audioSwrOutSpec_.chs,
                     4096,audioSwrOutSpec_.sampleFmt,1);
    RET(av_samples_alloc);
//    qDebug() << "after data[0] = " << audioSwrOutFrame_->data[0];
//    qDebug() << "after linesize[0] = " << audioSwrOutFrame_->linesize[0];

    return 0;
}




void VideoPlayer::addAudioPkt(AVPacket &pkt)
{
    std::lock_guard<std::mutex> lock(audioMutex_);
    audioPacketList->push_back(pkt);
//    audioCon_.notify_all();
}


void VideoPlayer::clearAudioList()
{
    std::lock_guard<std::mutex> lock(audioMutex_);
    for(AVPacket &pkt:*audioPacketList)
    {
        av_packet_unref(&pkt);
    }
    audioPacketList->clear();
}


void VideoPlayer::sdlAudioCallbackFunc(void *userdata,Uint8 *stream,int len)
{
    VideoPlayer *player = (VideoPlayer *)userdata;
    player->sdlAudioCallback(stream,len);
}


int VideoPlayer::initSDL()
{
    SDL_AudioSpec spec;
    //采样率
    spec.freq = 44100;
    //采样格式
    spec.format = AUDIO_S16LSB;
    //声道数
    spec.channels = 2;
    //音频缓冲区的样本数(必须是2的幂)
    spec.samples = 1024;
    //回调函数(自动创建的子线程调用)
    spec.callback = &sdlAudioCallbackFunc;
    //传递给回调函数的参数
    spec.userdata = this;


    //打开音频设备
    if(SDL_OpenAudio(&spec,nullptr))
    {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        return -1;
    }



    return 0;
}




//当调用SDL_PauseAudio(0);后会自动调用回调函数并从缓冲区stream中拉取数据进行播放
void VideoPlayer::sdlAudioCallback(Uint8 *stream, int len)
{
//    qDebug() << "len = " << len;
    //先静音
    memset(stream,0,len);
    //stream指向存放pcm的音频缓冲区
    //需要先解码再存入缓冲区
    //len=缓冲区剩余大小
    while(len > 0)
    {
        if(state_ == Pasued&&audioSeekTime_ == -1) break;

        if(state_ == Stopped)
        {
            audioCanFree_ = 1;
            break;
        }
        //Frame已经全部读完的时候需要重写解码并填充Frame
        if(audioSwrOutIdx_ >= audioSwrOutSize_)
        {
            //解码并重采样，返回可用的PCM的数量,PCM存放在Frame的中data[0]中
            audioSwrOutSize_ = decodeAudio();
//            qDebug() << "audioSwrOutSize = " << audioSwrOutSize_;
            //索引清零
            audioSwrOutIdx_ = 0;
            //没有解码出PCM则静音处理
            if(audioSwrOutSize_ <=0)
            {
                memset(audioSwrOutFrame_->data[0],0,audioSwrOutSize_ = 1024);
            }

        }

        int srcLen = audioSwrOutSize_ - audioSwrOutIdx_;
        srcLen = std::min(srcLen,len);

        //设置音量
        int volumn = (volumn_ * 1.0 /Max)*SDL_MIX_MAXVOLUME;

        //将PCM数据填充到stream
        SDL_MixAudio(stream,audioSwrOutFrame_->data[0]+audioSwrOutIdx_,srcLen,volumn);

        len -= srcLen;
        stream += srcLen;
        audioSwrOutIdx_ += srcLen;

    }
}

//解码，返回PCM的总字节数
int VideoPlayer::decodeAudio()
{
    AVPacket pkt;
    {
        std::unique_lock<std::mutex> lock(audioMutex_);

    //    while(audioPacketList->empty())
    //        audioCon_.wait(lock);

        if(audioPacketList->empty() || state_ == Stopped)
        {
            return 0;
        }

        pkt = audioPacketList->front();
        audioPacketList->pop_front();
    }

    //保存音频时钟
    if(pkt.pts != AV_NOPTS_VALUE)
    {
        audioClock_ = av_q2d(audioStream_->time_base)*pkt.pts;
        emit timeChanged(this);
    }

       //注意播放视频时不能 在这个位置进行释放
    if (audioSeekTime_ >= 0) {
        if (audioClock_ < audioSeekTime_) {
            // 释放pkt
            av_packet_unref(&pkt);
            return 0;
        } else {
            audioSeekTime_ = -1;
        }
    }



    //发送数据到解码器
    int ret = avcodec_send_packet(audioDecodeCtx_,&pkt);
    //释放pkt
    av_packet_unref(&pkt);
    RET(avcodec_send_packet);




    //获取解码后的数据
    ret = avcodec_receive_frame(audioDecodeCtx_,audioSwrInFrame_);
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return 0;
    }
    else
        RET(avcodec_receive_frame);

    //查看解码后的数据
//    qDebug() << audioSwrInFrame_->sample_rate
//             << audioSwrInFrame_->channels
//             << av_get_sample_fmt_name((AVSampleFormat) audioSwrInFrame_->format)
//             <<audioSwrInFrame_->nb_samples;
    //重采样输出样本数
    int outSamples = av_rescale_rnd(audioSwrOutSpec_.sampleRate,
                                    audioSwrInFrame_->nb_samples,audioSwrInSpec_.sampleRate,
                                    AV_ROUND_UP);
//    qDebug() << "outSamples size = " << outSamples;
    //需要重采样
    ret = swr_convert(audioSwrCtx_,
                      //存放输出样本
                      audioSwrOutFrame_->data,
                      //输出样本数，通过输出采样率和输入信息计算获得
                      outSamples,
                      //输入样本的地址
                      (const uint8_t**)audioSwrInFrame_->data,
                      //输入样本数量
                      audioSwrInFrame_->nb_samples);
    RET(swr_convert);
//    qDebug() << "swr_convert size = " << ret;

    return ret*audioSwrOutSpec_.bytesPerSampleFrame;
}




