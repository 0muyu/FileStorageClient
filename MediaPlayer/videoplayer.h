#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include <QCoreApplication>
#include <QDebug>
#include <QImage>
#include <QThread>

#include "libavutil/time.h"
#include "packetqueue.h"

extern "C" {
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libavutil/time.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000  // 解码后最大的帧大小 1 second of 48khz 32bit audio(最大就是双声道16位  48*2*(32/16)=192)
#define SDL_AUDIO_BUFFER_SIZE 1024  //音频缓冲区大小 AAC->PCM的转换  一个采样周期是1024个点

class VideoPlayer;

typedef struct VideoState {
  AVFormatContext *pFormatCtx;  //相当于视频”文件指针”；AVFormatContext该结构体会在avformat_open_input 是初始化填充
  ///////////////音频///////////////////////////////////
  AVStream *        audio_st;        //音频流
  PacketQueue *     audioq;          //音频缓冲队列
  AVCodecContext *  pAudioCodecCtx;  //音频解码器信息指针
  int               audioStream;     //视频音频流索引
  double            audio_clock;     ///< pts of last decoded frame 音频时钟
  SDL_AudioDeviceID audioID;         //音频 ID
  AVFrame           out_frame;       //设置参数，供音频解码后的 swr_alloc_set_opts 使用。
  /// 音频回调函数使用的量
  uint8_t      audio_buf[ ( AVCODEC_MAX_AUDIO_FRAME_SIZE * 3 ) / 2 ];  //防止溢出——3/2
  unsigned int audio_buf_size  = 0;
  unsigned int audio_buf_index = 0;
  AVFrame *    audioFrame;
  //////////////////////////////////////////////////////
  ///////////////视频///////////////////////////////////
  AVStream *      video_st;     //视频流
  PacketQueue *   videoq;       //视频队列
  AVCodecContext *pCodecCtx;    //音频解码器信息指针
  int             videoStream;  //视频音频流索引
  double          video_clock;  ///< pts of last decoded frame 视频时钟
  SDL_Thread *    video_tid;    //视频线程 id
  /////////////////////////////////////////////////////

  //////////播放控制//////////////////////////////////////
  /// 播放控制的变量
  bool isPause             = false;  //暂停标志
  bool quit                = false;  //停止
  bool readFinished        = true;   //读线程文件读取完毕：读取网络中的文件时（网络数据不连续会<0 ）
  bool readThreadFinished  = true;   //读取线程是否结束：用于读取执行run的线程是否结束
  bool videoThreadFinished = true;   // 视频线程是否结束，用于回收解码器(音频是回调函数没有真正意义的结束)
  /////////////////////////////////////////////////////
  //// 跳转相关的变量
  //读取文件的跳转标志和跳转大小
  int     seek_req;  //跳转标志 -- 读线程
  int64_t seek_pos;  //跳转的位置 -- 微秒

  //解析音视频的跳转标志和大小
  int    seek_flag_audio;  //跳转标志 -- 用于音频线程中
  int    seek_flag_video;  //跳转标志 -- 用于视频线程中
  double seek_time;        //跳转的时间(秒) 值和 seek_pos 是一样的

  //////////////////////////////////////////////////////
  int64_t start_time;  //单位 微秒
  VideoState() { audio_clock = video_clock = start_time = 0; }
  VideoPlayer *m_player;  //用于调用函数
} VideoState;

// 回调函数的参数，时间和有无流的判断
typedef struct {
  time_t lasttime;
  bool   connected;
} Runner;
enum PlayerState { Playing = 0, Pause, Stop };  // pause
class VideoPlayer : public QThread {
  Q_OBJECT
 public:
  explicit VideoPlayer( QObject *parent = nullptr );
  ~VideoPlayer();

  void run();
  void setFileName( const QString &newFileName );

  ///播放控制
  void play();
  void pause();
  void stop( bool isWait );

  void        seek( int64_t pos );
  double      getCurrentTime();
  int64_t     getTotalTime();
  PlayerState playerState() const;
 signals:
  void sig_setImage( QImage image );  //使用拷贝不使用引用或指针，引用run在执行完后会回收资源
  void sig_PlayerStateChanged( int state );
  void sig_TotalTime( qint64 uSec );
 public slots:
  void slot_SendGetOneImage( QImage &img );

 private:
  QString     m_fileName;
  VideoState  m_videoState;
  PlayerState m_playerState;
};

#endif // VIDEOPLAYER_H
