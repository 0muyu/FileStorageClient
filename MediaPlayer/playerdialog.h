#ifndef PLAYERDIALOG_H
#define PLAYERDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPixmap>
#include <QStyle>
#include <QTimer>

#include "videoplayer.h"
QT_BEGIN_NAMESPACE
namespace Ui { class PlayerDialog; }
QT_END_NAMESPACE

class PlayerDialog : public QDialog
{
  Q_OBJECT

 public:
  PlayerDialog(QWidget *parent = nullptr);
  ~PlayerDialog();

  VideoPlayer *pVediaPlayer() const;

 private slots:
  void on_pb_start_clicked();

  void on_pb_resume_clicked();

  void on_pb_pause_clicked();
  //点击停止
  void on_pb_stop_clicked();

  //设置显示图片
  void slot_setImage( QImage image );

  void slot_TotalTime( qint64 uSec );

  void slot_TimerTimeOut();

  //时间过滤函数
  bool eventFilter( QObject *obj, QEvent *evnt );
 public slots:
  void slot_PlayerStateChanged( int state );

 private:
  Ui::PlayerDialog *ui;
  VideoPlayer *     m_pVediaPlayer;
  QTimer            m_timer;
  QString           m_fileName;
  //停止的状态
  bool isStop;
};
#endif // PLAYERDIALOG_H
