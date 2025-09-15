#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QCloseEvent>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QMimeType>
#include <QProcess>
#include <QProgressBar>

#include "mytablewidgetitem.h"
#include "playerdialog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE
class CKernel;
class MainDialog : public QDialog
{
  Q_OBJECT

 public:
  MainDialog(QWidget *parent = nullptr);
  ~MainDialog();

  //重写父类的关闭事件
  void closeEvent( QCloseEvent *event );
 signals:
  void sig_close();
  // 1.给Kernel 发送上传文件的信号(什么绝对路径，上传到什么目录下)
  void sig_uploadFile( QString path, QString dir );
  // 2.给Kernel 发送下载文件的信号（什么目录下下载）
  void sig_downloadFile( int fileId, QString dir );
  // 3.给Kernel 发送下载文件夹的信号（什么目录下的文件夹下载）
  void sig_downloadFolder( int fileId, QString dir );
  // 4.给Kernel 发送新建文件夹的信号(什么名字什么目录下创建)
  void sig_addFolder( QString name, QString dir );
  // 5.给Kernel 发送跳转路径的信号(实际就是重新获取问价列表)
  void sig_changeDir( QString dir );
  // 6.给Kernel 发送上传文件夹的信号（什么路径 的文件夹，到什么目录下）
  void sig_uploadFolder( QString path, QString dir );
  // 7.给Kernel 发送分享文件的信号（什么目录下的什么文件列表）
  void sig_shareFile( QVector<int> shareArr, QString dir );
  // 8.给Kernel 发送获取分享文件的信号（获取什么分享码的文件，添加到什么目录下）
  void sig_getShare( int code, QString dir );
  // 9.给Kernel 发送删除文件的信号（什么目录下的什么文件列表）
  void sig_deleteFile( QVector<int> deleteArr, QString dir );

  // 10.给Kernel 发送设置上传暂停的信号  0开始  1暂停
  void sig_setUploadPause( int timestamp, int isPause );
  // 10.给Kernel 发送设置下载暂停的信号   0开始  1暂停
  void sig_setDownloadPause( int timestamp, int isPause );

  // 11.给Kernel 发送获取url的信号(获取什么文件的url)
  void sig_getUrl( int fileId, QString dir );

 private slots:
  void on_pb_file_clicked();

  void on_pb_transmit_clicked();

  void on_pb_share_clicked();
  // 4.点击添加文件按钮
  void on_pb_addfile_clicked();

  // 4.处理 点击 添加文件按钮弹出的菜单栏
  void slot_addFolder( bool flag );
  void slot_uploadFile( bool flag );
  void slot_uploadFolder( bool flag );
  // 5.插入上传文件信息 到MainDialog 的"上传中"  控件里
  void slot_insertUploadFile( FileInfo &info );

  // 6.插入完成上传的文件信息 到MainDialog 的"已完成"  控件里
  void slot_insertFinishFile( FileInfo &info );

  // 7.处理CKernel发送来的更新文件上传进度的信号
  void slot_updataUploadFileProcess( int timestamp, int pos );

  // 8.如果 进度条到满，删除在上传中对应的行
  void slot_deleteUploadFileByRow( int row );

  // 9.向文件列表插入用户文件列表项
  void slot_insertFileInfo( FileInfo &Info );
  // 10.勾选某一行（将最前面的小框打勾）
  void on_table_file_cellClicked( int row, int column );
  // 11.文件界面右键弹出菜单栏
  void on_table_file_customContextMenuRequested( const QPoint &pos );

  // 11.文件界面右键弹出菜单栏
  // 11.下载文件或下载文件夹
  void slot_downloadFile( bool flag );
  void slot_shareFile( bool flag );
  void slot_deleteFile( bool flag );
  void slot_getShare( bool flag );

  // 12.插入下载文件的信息  到MainDialog的"下载中"控件中
  void slot_insertDownloadFile( FileInfo &info );

  // 13.处理CKernel发送来的更新文件上传进度的信号
  void slot_updataDownloadFileProcess( int timestamp, int pos );

  // 14.插入完成下载的文件信息 到MainDialog 的"已完成"  控件里
  void slot_insertDownloadFinishFile( FileInfo &info );  //先添加再删除
  // 15.插如果 进度条到满，删除在下载中对应的行
  void slot_deleteDownloadFileByRow( int rpw );

  // 16.打开路径
  void slot_openPath( bool flag );

  // 17.删除文件列表
  void slot_deleteAllFileInfo();
  // 18.双击文件表，进入文件夹或文件,相当于">"
  void on_table_file_cellDoubleClicked( int row, int column );

  // 19.点击"<",回到上一级
  void on_pb_prev_clicked();

  // 20.插入f分享成功的文件信息 到MainDialog 的"分享"  控件里
  void slot_insertShareList( QString name, int sharelink, int size, QString time );
  // 21.删除分享文件列表
  void slot_deleteAllShareFileInfo();

  // 22.上传中界面弹出菜单栏  菜单项的处理槽函数
  void slot_uploadPause( bool flag );
  void slot_uploadResume( bool flag );
  // 23.下载中界面弹出菜单栏  菜单项的处理槽函数
  void slot_downloadPause( bool flag );
  void slot_downloadResume( bool flag );

  void on_table_upload_cellClicked( int row, int column );

  void on_table_download_cellClicked( int row, int column );

  // 25.设置登录信息
  void slot_setInfo( QString name );

  // 26.根据时间戳获取信息
  // 26.1根据时间戳获取下载文件信息
  bool slot_getDownloadFileInfoByTimestamp( int timestamp, FileInfo &info );
  // 26.2根据时间戳获取上传文件信息
  bool slot_getUploadFileInfoByTimestamp( int timestamp, FileInfo &info );

 private:
  bool isVideoFile( const QString &filePath );

 private:
  Ui::MainDialog *ui;
  // 1.上传文件的菜单栏
  QMenu           m_pMenuAddfile;
  // 2.文件页面选择文件后右键菜单栏
  QMenu           m_pMenuFileInfo;
  // 3.上传中 页面菜单栏
  QMenu m_pmenuUpload;
  // 4.下载中 页面菜单栏
  QMenu m_pmenuDownload;

  friend class CKernel;
};

#endif  // MAINDIALOG_H
