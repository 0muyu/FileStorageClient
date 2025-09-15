#include "playerdialog.h"

#include "ui_playerdialog.h"
//#define _DEF_PATH "D:/Desktop/20250607/kk 2025-06-07 09-07-09.mp4"

//#define _DEF_PATH "rtmp://172.29.0.235:1935/vod//120.mp4"
#define _DEF_LIVE_PATH "rtmp://172.29.0.235:1935/videotest"

PlayerDialog::PlayerDialog( QWidget *parent ) : QDialog( parent ), ui( new Ui::PlayerDialog ) {
  ui->setupUi( this );
  m_pVediaPlayer = new VideoPlayer;
  QObject::connect( m_pVediaPlayer, &VideoPlayer::sig_setImage, this, &PlayerDialog::slot_setImage );

  //设置状态为停止
  slot_PlayerStateChanged( PlayerState::Stop );

  connect( m_pVediaPlayer, &VideoPlayer::sig_PlayerStateChanged, this, &PlayerDialog::slot_PlayerStateChanged );

  connect( m_pVediaPlayer, &VideoPlayer::sig_TotalTime, this, &PlayerDialog::slot_TotalTime );

  QObject::connect( &m_timer, &QTimer::timeout, this, &PlayerDialog::slot_TimerTimeOut );
  m_timer.setInterval( 500 );  //超时时间 500 ms

  //安装事件过滤器，让该对象成为被观察对象//this去执行函数
  ui->slider_process->installEventFilter( this );

  //测试
  // m_pVediaPlayer->setFileName( _DEF_PATH );
}

PlayerDialog::~PlayerDialog()
{
  delete ui;
  delete m_pVediaPlayer;
}

// Qt线程
// QThread 定义子类  点击start()->run()

// 1.打开文件
void PlayerDialog::on_pb_start_clicked() {
  //开始播放->会在一段时间内循环获取图片
  // m_pVediaPlayer->start();
  // 1.打开浏览选择文件
  QString path = QFileDialog::getOpenFileName( this, "选择要播放的文件", "./", "视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);; 所有文件(*.*);;" );

  // 2.判断 获取路径是否为空
  if ( path.isEmpty() ) { return; }

  // 3.关闭，判断当前的状态 stop
  if ( m_pVediaPlayer->playerState() != PlayerState::Stop ) { m_pVediaPlayer->stop( true ); }

  // 4设置 m_play  filename
  m_pVediaPlayer->setFileName( path );

  avformat_network_init();  //支持打开网络文件

  // m_pVediaPlayer->setFileName( _DEF_PATH );

  // m_pVediaPlayer->stop( true );
  // m_pVediaPlayer->setFileName( _DEF_LIVE_PATH );
  m_pVediaPlayer->start();

  slot_PlayerStateChanged( PlayerState::Playing );
}
//设置显示图片
void PlayerDialog::slot_setImage( QImage image ) {
  // 1.0 使用lable的setImage

  // 2.0
  //设置缩放(pixmap:可以使用显卡进行渲染,显示速率和效果更好  image 内存中用于数据转换的)
  //  QPixmap pixmap;
  //  if ( !image.isNull() ) {
  //    pixmap = QPixmap::fromImage( image.scaled( ui->lb_show->size(), Qt::KeepAspectRatio ) );
  //  } else {
  //    pixmap = QPixmap::fromImage( image );
  //  }
  //  ui->lb_show->setPixmap( pixmap );

  // 3.0
  //使用opengl 进行渲染
  ui->wdg_show->slot_setImage( image );
}

void PlayerDialog::slot_TotalTime( qint64 uSec ) {
  qDebug() << __func__;
  qint64 Sec = uSec / 1000000;
  ui->slider_process->setRange( 0, Sec );  //精确到秒
  QString hStr = QString( "00%1" ).arg( Sec / 3600 );
  QString mStr = QString( "00%1" ).arg( Sec / 60 );
  QString sStr = QString( "00%1" ).arg( Sec % 60 );
  QString str  = QString( "%1:%2:%3" ).arg( hStr.right( 2 ) ).arg( mStr.right( 2 ) ).arg( sStr.right( 2 ) );
  ui->lb_endTime->setText( str );
}

void PlayerDialog::slot_TimerTimeOut() {
  if ( QObject::sender() == &m_timer ) {
    qint64 Sec = m_pVediaPlayer->getCurrentTime() / 1000000;
    ui->slider_process->setValue( Sec );
    QString hStr = QString( "00%1" ).arg( Sec / 3600 );
    QString mStr = QString( "00%1" ).arg( Sec / 60 % 60 );
    QString sStr = QString( "00%1" ).arg( Sec % 60 );
    QString str  = QString( "%1:%2:%3" ).arg( hStr.right( 2 ) ).arg( mStr.right( 2 ) ).arg( sStr.right( 2 ) );
    ui->lb_curTime->setText( str );
    if ( ui->slider_process->value() == ui->slider_process->maximum() && m_pVediaPlayer->playerState() == PlayerState::Stop ) {
      slot_PlayerStateChanged( PlayerState::Stop );
    } else if ( ui->slider_process->value() + 1 == ui->slider_process->maximum() && m_pVediaPlayer->playerState() == PlayerState::Stop ) {
      slot_PlayerStateChanged( PlayerState::Stop );
    }
  }
}

bool PlayerDialog::eventFilter( QObject *obj, QEvent *event ) {
  if ( obj == ui->slider_process ) {
    if ( event->type() == QEvent::MouseButtonPress ) {
      QMouseEvent *mouseEve = static_cast<QMouseEvent *>( event );
      int          min      = ui->slider_process->minimum();
      int          max      = ui->slider_process->maximum();
      int          value    = QStyle::sliderValueFromPosition( min, max, mouseEve->pos().x(), ui->slider_process->width() );

      m_timer.stop();
      ui->slider_process->setValue( value );
      m_pVediaPlayer->seek( (qint64) value * 1000000 );  // value 秒
      m_timer.start();
      //先暂停，再设置再开始，为了解决跳转时，进度条会闪跳一下再跳转

      return true;
    } else {
      return false;
    }
  }

  return QDialog::eventFilter( obj, event );
}

VideoPlayer *PlayerDialog::pVediaPlayer() const { return m_pVediaPlayer; }

//点击恢复
void PlayerDialog::on_pb_resume_clicked() {
  if ( /*m_pVediaPlayer->playerState() != Pause*/ isStop ) { return; }

  m_pVediaPlayer->play();

  //切换

  ui->pb_resume->hide();
  ui->pb_pause->show();
}

//点击暂停
void PlayerDialog::on_pb_pause_clicked() {
  if ( /*m_pVediaPlayer->playerState() != Playing */ isStop ) { return; }

  //切换
  m_pVediaPlayer->pause();

  ui->pb_pause->hide();
  ui->pb_resume->show();
}

//点击停止
void PlayerDialog::on_pb_stop_clicked() { m_pVediaPlayer->stop( true ); }

void PlayerDialog::slot_PlayerStateChanged( int state ) {
  switch ( state ) {
    case PlayerState::Stop:
      qDebug() << "VideoPlayer::Stop";
      m_timer.stop();
      ui->slider_process->setValue( 0 );

      ui->lb_endTime->setText( "00:00:00" );
      ui->lb_curTime->setText( "00:00:00" );

      ui->pb_pause->hide();
      ui->pb_resume->show();

      //发送一个黑色图片
      {
        QImage img;
        img.fill( Qt::black );
        slot_setImage( img );
      }
      this->update();
      isStop = true;
      break;
    case PlayerState::Playing:
      qDebug() << "VideoPlayer::Playing";
      ui->pb_resume->hide();
      ui->pb_pause->show();
      m_timer.start();
      this->update();
      isStop = false;
      break;
  }
}
