#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include <QThread>
#include <unordered_map>

#include "common.h"
#include "csqlite.h"
#include "logindialog.h"
#include "maindialog.h"
#include "packdef.h"
#include "playerdialog.h"
//定义类成员函数 指针

class CKernel;
typedef void ( CKernel::*PFUN )( unsigned int lSendIP, char* buf, int nlen );

class INetMediator;

//#define USE_SERVER 1

class CKernel : public QObject
{
  Q_OBJECT
  //为实现 核心处理类的单例将构造  拷贝构造私有化析构
 private:
  explicit CKernel( QObject* parent = nullptr );
  explicit CKernel( const CKernel& kernel );
  ~CKernel();

 public:
  //实现单例
  static CKernel* getInstance() {
    static CKernel kernel;
    return &kernel;
  }
  // 1.加载网络配置文件
  void loadIniFile();

 private slots:
  // 普通槽函数
  void slot_close();
  // 1.处理loginDialog发送的登录信息
  void slot_loginMessage( QString tel, QString pwd );
  // 2.处理loginDialog发送的注册信息
  void slot_registerMessage( QString tel, QString pwd, QString name );
  // 3.
  // 3.1处理MainDialog发送的上传文件信息  (什么绝对路径，上传到什么目录下)
  void slot_uploadFile( QString path, QString dir );

  // 4
  // 4.1处理MainDialog 发送下载文件的信号（什么目录下下载）
  void slot_downloadFile( int fileId, QString dir );
  // 4.2处理MainDialog 发送下载文件夹的信号（什么目录下的文件夹下载）
  void slot_downloadFolder( int fileId, QString dir );

  // 5.处理MainDialog 发送的新建文件夹的信号(什么名字什么目录下创建)
  void slot_addFolder( QString name, QString dir );

  // 6.处理MainDialog 发送跳转路径的信号(实际就是重新获取文件列表)
  void slot_changeDir( QString dir );

  // 7.处理MainDialog 发送上传文件夹的信号（什么路径的文件夹，到什么目录下）
  void slot_uploadFolder( QString path, QString dir );

  // 8.处理MainDialog 发送分享文件的信号（什么目录下的什么文件列表）
  void slot_shareFile( QVector<int> shareArr, QString dir );

  // 9.处理MainDialog 发送获取分享文件的信号（获取什么分享码的文件，添加到什么目录下）
  void slot_getShare( int code, QString dir );

  // 10.处理MainDialog 发送删除文件的信号（什么目录下的什么文件列表）
  void slot_deleteFile( QVector<int> shareArr, QString dir );

  // 11.处理MainDialog  发送设置上传暂停的信号  0开始  1暂停
  void slot_setUploadPause( int timestamp, int isPause );
  // 11.处理MainDialog  发送设置下载暂停的信号   0开始  1暂停
  void slot_setDownloadPause( int timestamp, int isPause );

  // 12.处理MainDialog  发送获取url的信号(获取什么文件的url)
  void slot_getUrl( int fileId, QString dir );

//网络槽函数

// 1.服务器的处理 槽函数
#ifdef USE_SERVER
  void slot_ServerReadyData( unsigned int lSendIP, char* buf, int nlen );
#endif

  //-------------------------------------------------------处理槽函数-----------------------------------
  // 1.客户端的处理槽函数
  void slot_dealClientData( unsigned int lSendIP, char* buf, int nlen );

  // 2. 处理登录回复
  void slot_dealLoginRs( unsigned int lSendIP, char* buf, int nlen );

  // 3.处理注册回复
  void slot_dealRegisterRs( unsigned int lSendIP, char* buf, int nlen );

  // 4.处理上传文件回复
  void slot_dealUploadFileRs( unsigned int lSendIP, char* buf, int nlen );

  // 5.处理文件内容回复
  void slot_dealFileContentRs( unsigned int lSendIP, char* buf, int nlen );

  // 6.处理获取文件列表的回复
  void slot_dealGetCurDirFileListRs( unsigned int lSendIP, char* buf, int nlen );

  // 7.处理文件头请求
  void slot_dealFileHeaderRq( unsigned int lSendIP, char* buf, int nlen );

  // 8.处理文件内容请求
  void slot_dealFileContentRq( unsigned int lSendIP, char* buf, int nlen );

  // 9.处理新建文件夹回复
  void slot_dealAddFolderRs( unsigned int lSendIP, char* buf, int nlen );

  // 10.处理秒传回复
  void slot_dealQuickUploadRs( unsigned int lSendIP, char* buf, int nlen );

  // 11.处理分享文件的回复
  void slot_dealShareFileRs( unsigned int lSendIP, char* buf, int nlen );

  // 12.处理 获取分享文件列表的回复
  void slot_dealGetShareFileListRs( unsigned int lSendIP, char* buf, int nlen );

  // 13.处理  获取分享文件请求的回复
  void slot_dealGetShareRs( unsigned int lSendIP, char* buf, int nlen );

  // 14.处理 文件夹头请求
  void slot_dealFolderHeaderRq( unsigned int lSendIP, char* buf, int nlen );

  // 15.处理 删除文件的回复
  void slot_dealDeleteFileRs( unsigned int lSendIP, char* buf, int nlen );

  // 16.处理 上传续传的回复
  void slot_dealContinueUploadRs( unsigned int lSendIP, char* buf, int nlen );

  // 17.处理  获取url的回复
  void slot_dealGetUrlRs( unsigned int lSendIP, char* buf, int nlen );

  //--------------------------------------------请求槽函数----------------------------------------------------
  //获取文件列表
  void slot_getCurDirFileListRq();
  //获取分享文件列表
  void slot_getShareFileListRq();

 signals:
  //给MainDialog 发送更新文件 上传进度的信号
  void sig_updataUploadFileProcess( int timestamp, int pos );
  //给MainDialog 发送更新文件 下载进度的信号
  void sig_updataDownloadFileProcess( int timestamp, int pos );

 private:
  // 2.私有的设置协议与函数的映射数组
  void initFunArr();

  // 3.私有的发送函数
  void sendData( char* buf, int len );

  // 4.设置系统路径
  void setSystemPath();

  // 5.初始化数据库
  void initDatabase( int id );

  // 1.缓存上传的任务
  void slot_writeUploadTask( FileInfo& info );  //在插入到上传控件中时，跟着更新数据库，相当于影子
  // 2.缓存下载的任务
  void slot_writeDownloadTask( FileInfo& info );
  // 3.完成任务，删除上传记录
  void slot_deleteUploadTask( FileInfo& info );  //在上传控件中删除对应上传项时（size>=capacity），删除在数据库中的记录
  // 4.完成任务，删除下载记录
  void slot_deleteDownloadTask( FileInfo& info );
  // 5.加载上传任务
  void slot_getUploadTask( QList<FileInfo>& infoLst );
  // 6.加载下载任务
  void slot_getDownloadTask( QList<FileInfo>& infoLst );

 private:
  MainDialog* m_pMainDialog;

  QString m_ip;
  QString m_port;

  QString m_name;
  int     m_id;

  PFUN m_funArr[ _DEF_PACK_COUNT ];

  QString m_curDir;  //存储网盘当前目录

  QString m_sysPath;  //默认系统保存路径(绝对路径)与exe同级

  bool m_quit;  //标志程序是否退出

  //服务器
#ifdef USE_SERVER
  INetMediator* m_pMediatorServer;
#endif

  //客户端
  INetMediator* m_pMediatorClient;

  //登录&注册窗口
  LoginDialog*                      m_pLoginDialog;
  std::unordered_map<int, FileInfo> m_mapTimestampToFileInfo;  //使用浅拷贝；

  //数据库
  CSqlite* m_sql;

  //播放器
  PlayerDialog* m_Player;
};

#endif  // CKERNEL_H
