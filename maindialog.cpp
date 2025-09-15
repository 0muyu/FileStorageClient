#include "maindialog.h"

#include "ui_maindialog.h"

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
  ui->setupUi(this);
  //默认文件分页
  ui->sw_page->setCurrentIndex( 0 );
  //默认传输分页
  ui->tw_transmit->setCurrentIndex( 2 );
  //设置标题栏
  this->setWindowTitle( "我的网盘" );
  //设置最大最小化
  this->setWindowFlags( Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint );

  // 1.上传文件的菜单栏实现
  //首先定义菜单项 资源路径 :/images/folder.png
  QAction *action_addFolder =
      new QAction( QIcon( ":/images/folder.png" ), "新建文件夹" );

  QAction *action_uploadFile   = new QAction( "上传文件" );
  QAction *action_uploadFolder = new QAction( "上传文件夹 " );

  //添加菜单项
  m_pMenuAddfile.addAction( action_addFolder );
  m_pMenuAddfile.addSeparator();
  m_pMenuAddfile.addAction( action_uploadFile );
  m_pMenuAddfile.addAction( action_uploadFolder );

  //连接

  connect( action_addFolder, SIGNAL( triggered( bool ) ), this, SLOT( slot_addFolder( bool ) ) );
  connect( action_uploadFile, SIGNAL(triggered(bool)), this,SLOT(slot_uploadFile(bool) ) );
  connect( action_uploadFolder, SIGNAL( triggered( bool ) ), this, SLOT( slot_uploadFolder( bool ) ) );

  // 2.文件页面选择文件后右键菜单栏选项

  QAction *action_downloadFile = new QAction( "下载文件" );
  QAction *action_shareFile    = new QAction( "分享文件 " );
  QAction *action_deleteFile   = new QAction( "删除文件" );
  QAction *action_getShare     = new QAction( "获取分享" );  //直接 获取分享的文件到 文件列表上
  //添加菜单项
  m_pMenuFileInfo.addAction( action_addFolder );
  m_pMenuFileInfo.addSeparator();
  m_pMenuFileInfo.addAction( action_downloadFile );
  m_pMenuFileInfo.addAction( action_shareFile );
  m_pMenuFileInfo.addAction( action_deleteFile );
  m_pMenuFileInfo.addAction( "收藏" );

  m_pMenuFileInfo.addSeparator();
  m_pMenuFileInfo.addAction( action_getShare );

  //连接
  // 1.新建文件夹
  // 2.下载
  connect( action_downloadFile, SIGNAL( triggered( bool ) ), this, SLOT( slot_downloadFile( bool ) ) );
  // 3.分享
  connect( action_shareFile, SIGNAL( triggered( bool ) ), this, SLOT( slot_shareFile( bool ) ) );
  // 4.删除
  connect( action_deleteFile, SIGNAL( triggered( bool ) ), this, SLOT( slot_deleteFile( bool ) ) );

  // 5.获取分享
  connect( action_getShare, SIGNAL( triggered( bool ) ), this, SLOT( slot_getShare( bool ) ) );

  // 3.4 之前 1(添加文件).2(文件信息勾选菜单) 实现弹出菜单栏 是通过转到槽实现的，3、4的实现与 2一样都是在表格中点击；

  //这里实现 使用lambda 表达式， 借鉴2
  // []捕捉列表  ()函数参数列表  {}函数体
  connect( ui->table_upload, &QTableWidget::customContextMenuRequested, this, [ this ]( QPoint ) { this->m_pmenuUpload.exec( QCursor::pos() ); } );
  connect( ui->table_download, &QTableWidget::customContextMenuRequested, this, [ this ]( QPoint ) { this->m_pmenuDownload.exec( QCursor::pos() ); } );

  // 3.上传中 页面的菜单栏实现  4.下载中 页面的菜单栏实现
  QAction *actionUploadPause    = new QAction( "暂停" );
  QAction *actionUploadResume   = new QAction( "开始" );
  QAction *actionDownloadPause  = new QAction( "暂停" );
  QAction *actionDownloadResume = new QAction( "开始" );

  m_pmenuUpload.addAction( actionUploadPause );
  m_pmenuUpload.addAction( actionUploadResume );
  m_pmenuUpload.addAction( "全部开始" );
  m_pmenuUpload.addAction( "全部暂停" );

  m_pmenuDownload.addAction( actionDownloadPause );
  m_pmenuDownload.addAction( actionDownloadResume );
  m_pmenuDownload.addAction( "全部开始" );
  m_pmenuDownload.addAction( "全部暂停" );

  //连接

  connect( actionUploadPause, &QAction::triggered, this, &MainDialog::slot_uploadPause );
  connect( actionUploadResume, &QAction::triggered, this, &MainDialog::slot_uploadResume );
  connect( actionDownloadPause, &QAction::triggered, this, &MainDialog::slot_downloadPause );
  connect( actionDownloadResume, &QAction::triggered, this, &MainDialog::slot_downloadResume );
}

MainDialog::~MainDialog() { delete ui; }

void MainDialog::closeEvent( QCloseEvent *event ) {
  if ( QMessageBox::question( this, "退出", "是否退出程序" )
       == QMessageBox::Yes ) {
    // 1.关闭
    event->accept();
    // 2.发送关闭信号给核心处理类
    Q_EMIT sig_close();
  } else {
    event->ignore();
  }
}

// 1.文件点击的槽函数
void MainDialog::on_pb_file_clicked() { ui->sw_page->setCurrentIndex( 0 ); }

// 2.传输点击的槽函数
void MainDialog::on_pb_transmit_clicked() { ui->sw_page->setCurrentIndex( 1 ); }
// 3.分享点击的槽函数
void MainDialog::on_pb_share_clicked() { ui->sw_page->setCurrentIndex( 2 ); }
// 4.点击添加文件按钮
void MainDialog::on_pb_addfile_clicked() {
  //弹出菜单栏
  m_pMenuAddfile.exec( QCursor::pos() );  //鼠标的坐标
}
// 4.1 新建文件夹
void MainDialog::slot_addFolder( bool flag ) {
  qDebug() << __func__;
  // 1.弹出输入窗口,输入文件名
  QString name = QInputDialog::getText( this, "新建文件夹", "输入文件名" );
  // 2.检测文件名合法性
  // 2.1空或全空格
  QString nameTmp = name;
  nameTmp.remove( " " );
  if ( nameTmp.isEmpty() || name.length() > 100 ) {
    QMessageBox::about( this, "提示", "名字非法" );
    return;
  }
  // 2.2 不可以敏感词汇

  // 2.3 非法 \ / : * ? <  >  | "
  if ( name.contains( "\\" ) || name.contains( "/" ) || name.contains( ":" ) || name.contains( "?" ) || name.contains( "<" ) || name.contains( ">" ) || name.contains( "|" )
       || name.contains( "\"" ) ) {
    QMessageBox::about( this, "提示", "文件名不能包含特殊符号" );
    return;
  }

  // 2.4 TODO 判断现在是否已存在
  // 3.给Kernel发送信号
  QString dir = ui->lb_path->text();
  Q_EMIT  sig_addFolder( name, dir );
}
// 4.2 上传文件
void MainDialog::slot_uploadFile( bool flag ) {
  qDebug() << "MainDialog::" << __func__;

  //弹出窗口  选择文件
  qDebug() << "\n\n\n\n";

  QString path = QFileDialog::getOpenFileName( this, "选择文件", "./" );

  qDebug() << "\n\n\n\n";

  if ( path.isEmpty() ) { return; }

  // TODO:目前上传的是否有一样的文件，如果有，取消

  //发送信号给核心处理类，将上传什么文件 到什么目录下
  QString dir = ui->lb_path->text();
  Q_EMIT  sig_uploadFile( path, dir );
}

// 4.3 上传文件夹
void MainDialog::slot_uploadFolder( bool flag ) {
  qDebug() << "MainDialog::" << __func__;

  // 1.点击弹出文件选择对话框 选择路径
  QString path = QFileDialog::getExistingDirectory( this, "选择文件夹", "./" );
  // 2.判断是否为空
  if ( path.isEmpty() ) {
    qDebug() << "path empty";
    return;
  }
  // 3.发送上传 文件夹的信号给Kernel
  Q_EMIT sig_uploadFolder( path, ui->lb_path->text() );
}
// 5.插入上传信息 到MainDialog 的"上传中"  控件里
void MainDialog::slot_insertUploadFile( FileInfo &info ) {
  qDebug() << __func__;
  // MainDialog 中的控件是表格
  //表格插入信息
  //列： 文件  大小 时间  速率  进度  按钮
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_upload->rowCount();
  ui->table_upload->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  MyTableWidgetItem *item0 = new MyTableWidgetItem;
  item0->slot_setInfo( info );
  ui->table_upload->setItem( rows, 0, item0 );

  // 2.2大小
  QTableWidgetItem *item1 = new QTableWidgetItem( FileInfo::getsize( info.size ) );
  ui->table_upload->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( info.time );
  ui->table_upload->setItem( rows, 2, item2 );

  // 2.4速率
  QTableWidgetItem *item3 = new QTableWidgetItem( "0kb/s" );
  ui->table_upload->setItem( rows, 3, item3 );

  // 2.5进度条
  QProgressBar *process = new QProgressBar;
  process->setMaximum( info.size );
  ui->table_upload->setCellWidget( rows, 4, process );

  // 2.6按钮
  QPushButton *button = new QPushButton;
  if ( info.isPause == 0 ) {
    button->setText( "暂停" );
  } else {
    button->setText( "开始" );
  }
  ui->table_upload->setCellWidget( rows, 5, button );
}
// 6.插入完成传输的文件信息 到MainDialog 的"已完成"  控件里
void MainDialog::slot_insertFinishFile( FileInfo &info ) {
  qDebug() << __func__;
  // MainDialog 中的控件是表格
  //表格插入信息
  //列： 文件  大小 时间  上传完成
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_finish->rowCount();
  ui->table_finish->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  MyTableWidgetItem *item0 = new MyTableWidgetItem;
  item0->slot_setInfo( info );
  ui->table_finish->setItem( rows, 0, item0 );

  // 2.2大小
  QTableWidgetItem *item1 = new QTableWidgetItem( FileInfo::getsize( info.size ) );
  ui->table_finish->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( info.time );
  ui->table_finish->setItem( rows, 2, item2 );

  // 2.4上传完成
  QTableWidgetItem *item3 = new QTableWidgetItem( "已完成 " );
  ui->table_finish->setItem( rows, 3, item3 );
}
// 7.处理CKernel发送来的更新文件上传进度的信号
void MainDialog::slot_updataUploadFileProcess( int timestamp, int pos ) {
  //遍历所有行大的 文件列（第0列）
  int rows = ui->table_upload->rowCount();  //这里的rowCount 是 动态更新的，如果要删除所有项就 while(reocount>0)

  for ( int i = 0; i < rows; i++ ) {
    // 1.取到 每个文件的时间戳 ，对比是否一致
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_upload->item( i, 0 );  //考虑使用c++11de  dynamic_cast 进行转换
    //一致
    if ( timestamp == item0->m_info.timeStamp ) {
      // 1.1更新上传进度
      QProgressBar *item4 = (QProgressBar *) ui->table_upload->cellWidget( i, 4 );

      item0->m_info.pos = pos;
      qDebug() << "pos: " << item0->m_info.pos;
      item4->setValue( pos );
      // 1.2检测是否结束 ，如果结束 删除该行，并添加到"已完成"控件
      qDebug() << "value: " << item4->value() << "max: " << item4->maximum();
      if ( item4->value() >= item4->maximum() ) {
        //删除该行，并添加到"已完成"控件
        slot_insertFinishFile( item0->m_info );  //先添加再删除
        slot_deleteUploadFileByRow( i );
        return;
      }
    }
  }
}
// 8.如果 进度条到满，删除在上传中对应的行
void MainDialog::slot_deleteUploadFileByRow( int row ) {
  qDebug() << __func__;

  ui->table_upload->removeRow( row );
}
// 9.逐行插入文件信息
void MainDialog::slot_insertFileInfo( FileInfo &info ) {
  qDebug() << __func__;
  // MainDialog 中的控件是表格
  //表格插入信息
  //列： 文件  大小 时间
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_file->rowCount();
  ui->table_file->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  MyTableWidgetItem *item0 = new MyTableWidgetItem;
  item0->slot_setInfo( info );
  ui->table_file->setItem( rows, 0, item0 );

  // 2.2大小
  QString strSize;
  if ( info.type == "file" ) {
    strSize = FileInfo::getsize( info.size );

  } else {
    strSize = " ";
  }
  QTableWidgetItem *item1 = new QTableWidgetItem( strSize );
  ui->table_file->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( info.time );
  ui->table_file->setItem( rows, 2, item2 );
}

// 10.勾选某一行（将最前面的小框打勾）
void MainDialog::on_table_file_cellClicked( int row, int column ) {
  qDebug() << __func__;
  //切换勾选和位勾选状态
  MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_file->item( row, 0 );

  if ( item0->checkState() == Qt::Checked ) {
    item0->setCheckState( Qt::Unchecked );
  } else {
    item0->setCheckState( Qt::Checked );
  }
}

// 11.文件界面右键弹出菜单栏
void MainDialog::on_table_file_customContextMenuRequested( const QPoint &pos ) {
  //弹出菜单栏
  m_pMenuFileInfo.exec( QCursor::pos() );  //鼠标的坐标
}
// 11.1新建文件夹
// 11.2 下载文件
void MainDialog::slot_downloadFile( bool flag ) {
  qDebug() << "MainDialog::" << __func__;
  //遍历列表
  int     rows = ui->table_file->rowCount();
  QString dir  = ui->lb_path->text();  //获取目录
  for ( int i = 0; i < rows; i++ ) {
    //查看选中的
    MyTableWidgetItem *itme0 = (MyTableWidgetItem *) ui->table_file->item( i, 0 );

    if ( itme0->checkState() == Qt::Checked ) {
      // 1.TODO：列表中有这个下载，不能开始，过滤

      // 2.根据类型发送信号类型
      if ( itme0->m_info.type == "file" ) {
        Q_EMIT sig_downloadFile( itme0->m_info.fileid, dir );

      } else {
        Q_EMIT sig_downloadFolder( itme0->m_info.fileid, dir );
      }
    }
  }
}
// 11.3 分享文件
void MainDialog::slot_shareFile( bool flag ) {
  qDebug() << "MainDialog::" << __func__;

  // 1.申请数组，将所有的需要分享文件都放在数组中
  QVector<int> shareArr;

  // 2.遍历所有项,将打勾项放入数组中
  int count = ui->/*table_download*/ table_file->rowCount();
  for ( int i = 0; i < count; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_file->item( i, 0 );
    //判断是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      //添加到数组中
      shareArr.push_back( item0->m_info.fileid );
    }
  }
  // 3.发送信号给Kernel进行处理

  Q_EMIT sig_shareFile( shareArr, ui->lb_path->text() );
}
// 11.4删除文件
void MainDialog::slot_deleteFile( bool flag ) {
  qDebug() << "MainDialog::" << __func__;

  // 1.申请数组，将所有的需要分享文件都放在数组中
  QVector<int> deleteArr;

  // 2.遍历所有项,将打勾项放入数组中
  int count = ui->table_file->rowCount();
  for ( int i = 0; i < count; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_file->item( i, 0 );
    //判断是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      //添加到数组中
      deleteArr.push_back( item0->m_info.fileid );
    }
  }
  // 3.发送信号给Kernel进行处理

  Q_EMIT sig_deleteFile( deleteArr, ui->lb_path->text() );
}
// 获取分享
void MainDialog::slot_getShare( bool flag ) {
  qDebug() << "MainDialog::" << __func__;
  // 1.弹窗 输入分享码
  QString txt = QInputDialog::getText( this, "获取分享", "输入分享码" );

  // 2.过滤
  int code = txt.toInt();
  if ( txt.length() != 9 || code < 100000000 || code >= 1000000000 ) {
    QMessageBox::about( this, "提示", "分享码非法" );
    return;
  }
  // 3.发送信号
  Q_EMIT sig_getShare( code, ui->lb_path->text() );
}
// 12.插入下载文件的信息  到MainDialog的"下载中"控件中
void MainDialog::slot_insertDownloadFile( FileInfo &info ) {
  qDebug() << __func__;
  // MainDialog 中的控件是表格
  //表格插入信息
  //列： 文件  大小 时间  速率  进度  按钮
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_download->rowCount();
  ui->table_download->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  MyTableWidgetItem *item0 = new MyTableWidgetItem;
  item0->slot_setInfo( info );
  ui->table_download->setItem( rows, 0, item0 );

  // 2.2大小
  QTableWidgetItem *item1 = new QTableWidgetItem( FileInfo::getsize( info.size ) );
  ui->table_download->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( info.time );
  ui->table_download->setItem( rows, 2, item2 );

  // 2.4速率
  QTableWidgetItem *item3 = new QTableWidgetItem( "0kb/s" );
  ui->table_download->setItem( rows, 3, item3 );

  // 2.5进度条
  QProgressBar *process = new QProgressBar;
  process->setMaximum( info.size );
  ui->table_download->setCellWidget( rows, 4, process );

  // 2.6按钮
  QPushButton *button = new QPushButton;
  if ( info.isPause == 0 ) {
    button->setText( "暂停" );
  } else {
    button->setText( "开始" );
  }
  ui->table_download->setCellWidget( rows, 5, button );
}
// 13.处理CKernel发送来的更新文件下载进度的信号
void MainDialog::slot_updataDownloadFileProcess( int timestamp, int pos ) {
  //遍历所有行大的 文件列（第0列）
  int rows = ui->table_download->rowCount();  //这里的rowCount 是 动态更新的，如果要删除所有项就 while(reocount>0)

  for ( int i = 0; i < rows; i++ ) {
    // 1.取到 每个文件的时间戳 ，对比是否一致
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_download->item( i, 0 );  //考虑使用c++11de  dynamic_cast 进行转换
    //一致
    if ( timestamp == item0->m_info.timeStamp ) {
      // 1.1更新上传进度
      QProgressBar *item4 = (QProgressBar *) ui->table_download->cellWidget( i, 4 );
      item0->m_info.pos   = pos;
      item4->setValue( pos );
      // 1.2检测是否结束 ，如果结束 删除该行，并添加到"已完成"控件
      qDebug() << "value: " << item4->value() << "max: " << item4->maximum();
      if ( item4->value() >= item4->maximum() ) {
        //删除该行，并添加到下载的"已完成"控件
        slot_insertDownloadFinishFile( item0->m_info );  //先添加再删除
        slot_deleteDownloadFileByRow( i );

        return;
      }
    }
  }
}
// 14.插入完成下载的文件信息 到MainDialog 的"已完成"  控件里
void MainDialog::slot_insertDownloadFinishFile( FileInfo &info ) {
  qDebug() << __func__;
  // MainDialog 中的控件是表格
  //表格插入信息
  //列： 文件  大小 时间  按钮
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_finish->rowCount();
  ui->table_finish->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  MyTableWidgetItem *item0 = new MyTableWidgetItem;
  item0->slot_setInfo( info );
  ui->table_finish->setItem( rows, 0, item0 );

  // 2.2大小
  QTableWidgetItem *item1 = new QTableWidgetItem( FileInfo::getsize( info.size ) );
  ui->table_finish->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( info.time );
  ui->table_finish->setItem( rows, 2, item2 );

  // 2.4 按钮
  QPushButton *button = new QPushButton;
  button->setIcon( QIcon( ":/images/folder.png" ) );
  //设置偏平
  button->setFlat( true );
  // tooltip提示
  button->setToolTip( info.absolutePath );

  QObject::connect( button, SIGNAL( clicked( bool ) ), this, SLOT( slot_openPath( bool ) ) );
  // ui->table_finish->setItem( rows, 3, button );
  ui->table_finish->setCellWidget( rows, 3, button );
}

// 15.插如果 进度条到满，删除在下载中对应的行
void MainDialog::slot_deleteDownloadFileByRow( int row ) {
  qDebug() << __func__;

  ui->table_download->removeRow( row );
}
// 16.打开路径
void MainDialog::slot_openPath( bool flag ) {
  qDebug() << __func__;

  QPushButton *button = (QPushButton *) QObject::sender();  //获取发送信号的指针

  QString path = button->toolTip();

  //转化
  path.replace( '/', '\\' );
  qDebug() << path;

  //打开文件夹(弹出达到文件资源管理器的功能)

  //方法1：explorer
  //方法2：
  QProcess    process;
  QStringList lst;
  lst << QString( "/select," ) << path;  //相当于两次push_back
  process.startDetached( "explorer", lst );
  qDebug() << "process";
}

// 17.删除文件列表
void MainDialog::slot_deleteAllFileInfo() {
  qDebug() << __func__;

  // ui->table_file->clear ();//只清空，不删除行号，因此不选用
  int rows = ui->table_file->rowCount();
  for ( int i = rows - 1; i >= 0; i-- ) { ui->table_file->removeRow( i ); }
}

// 18.双击文件表，进入文件夹或文件,相当于">"
void MainDialog::on_table_file_cellDoubleClicked( int row, int column ) {
  qDebug() << __func__;

  // 1.首先获取双击那行的文件名字
  MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_file->item( row, 0 );

  // 2.判断是否为文件夹,是跳转(TODO:是文件，考虑打开文件)
  if ( item0->m_info.type != "file" ) {
    // 2.1路径拼接
    QString dir = ui->lb_path->text() + item0->m_info.name + "/";
    // 2.2 设置路径
    ui->lb_path->setText( dir );
    // 2.3发送信号，更新当前目录刷新文件列表
    Q_EMIT sig_changeDir( dir );
  } else {
    if ( isVideoFile( item0->m_info.name ) ) {  //是视频文件
      Q_EMIT sig_getUrl( item0->m_info.fileid, ui->lb_path->text() );
    }
  }
}
// 19.点击"<",回到上一级
void MainDialog::on_pb_prev_clicked() {
  qDebug() << __func__;

  // 1.获取目录
  QString path = ui->lb_path->text();

  // 2.判断是否在根目录
  if ( "/" == path ) { return; }

  // 3.跳转到上一级
  // 3.1 首先先找到最右边的"/" 在在其左边继续从右开始找"/"
  // eg: /0702/
  path = path.left( path.lastIndexOf( "/" ) );  // -> /0702

  path = path.left( path.lastIndexOf( "/" ) + 1 );  // ->/

  // 3.2 设置并跳转路径
  ui->lb_path->setText( path );
  Q_EMIT sig_changeDir( path );
}

// 20.插入分享成功的文件信息 到MainDialog 的"分享"  控件里
void MainDialog::slot_insertShareList( QString name, int sharelink, int size, QString time ) {
  qDebug() << "MainDialog::" << __func__;

  //列： 文件  大小 时间  分享码
  // 1.新增一行 获取当前行+1 设置行数
  int rows = ui->table_share->rowCount();
  ui->table_share->setRowCount( rows + 1 );

  // 2.设置新增行的每一列控件(添加对象)
  // 2.1文件
  QTableWidgetItem *item0 = new QTableWidgetItem( name );
  ui->table_share->setItem( rows, 0, item0 );

  // 2.2大小
  QTableWidgetItem *item1 = new QTableWidgetItem( FileInfo::getsize( size ) );
  ui->table_share->setItem( rows, 1, item1 );

  // 2.3时间
  QTableWidgetItem *item2 = new QTableWidgetItem( time );
  ui->table_share->setItem( rows, 2, item2 );

  // 2.4分享码
  QTableWidgetItem *item3 = new QTableWidgetItem( QString::number( sharelink ) );
  ui->table_share->setItem( rows, 3, item3 );
}
// 21.删除分享文件列表
void MainDialog::slot_deleteAllShareFileInfo() {
  qDebug() << __func__;

  // ui->table_file->clear ();//只清空，不删除行号，因此不选用
  int rows = ui->table_share->rowCount();
  for ( int i = rows - 1; i >= 0; i-- ) { ui->table_share->removeRow( i ); }
}
// 22.上传中界面弹出菜单栏  菜单项的处理槽函数
void MainDialog::slot_uploadPause( bool flag ) {
  qDebug() << __func__;
  qDebug() << "上传暂停";

  // 1.遍历表单
  int rows = ui->table_upload->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_upload->item( i, 0 );
    //查看是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      QPushButton *button = (QPushButton *) ui->table_upload->cellWidget( i, 5 );
      //查看按钮状态 切换文字 发送信号
      if ( button->text() == "暂停" ) {
        button->setText( "开始" );
        //信号：设置文件信息结构体的暂停标志位
        Q_EMIT sig_setUploadPause( item0->m_info.timeStamp, 1 );
      }
    }
  }
}

void MainDialog::slot_uploadResume( bool flag ) {
  qDebug() << __func__;
  qDebug() << "上传开始";  // 1.遍历表单
  int rows = ui->table_upload->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_upload->item( i, 0 );
    //查看是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      QPushButton *button = (QPushButton *) ui->table_upload->cellWidget( i, 5 );
      //查看按钮状态 切换文字 发送信号
      if ( button->text() == "开始" ) {
        button->setText( "暂停" );
        //信号：设置文件信息结构体的暂停标志位
        Q_EMIT sig_setUploadPause( item0->m_info.timeStamp, 0 );
      }
    }
  }
}

// 23.下载中界面弹出菜单栏  菜单项的处理槽函数
void MainDialog::slot_downloadPause( bool flag ) {
  qDebug() << __func__;
  qDebug() << "下载暂停";

  // 1.遍历表单
  int rows = ui->table_download->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_download->item( i, 0 );
    //查看是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      QPushButton *button = (QPushButton *) ui->table_download->cellWidget( i, 5 );
      //查看按钮状态 切换文字 发送信号
      if ( button->text() == "暂停" ) {
        button->setText( "开始" );
        //信号：设置文件信息结构体的暂停标志位
        Q_EMIT sig_setDownloadPause( item0->m_info.timeStamp, 1 );
      }
    }
  }
}

void MainDialog::slot_downloadResume( bool flag ) {
  qDebug() << __func__;
  qDebug() << "下载开始";

  // 1.遍历表单
  int rows = ui->table_download->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_download->item( i, 0 );
    //查看是否打勾
    if ( item0->checkState() == Qt::Checked ) {
      QPushButton *button = (QPushButton *) ui->table_download->cellWidget( i, 5 );
      //查看按钮状态 切换文字 发送信号
      if ( button->text() == "开始" ) {
        button->setText( "暂停" );
        //信号：设置文件信息结构体的暂停标志位
        Q_EMIT sig_setDownloadPause( item0->m_info.timeStamp, 0 );
      }
    }
  }
}

//下载页面  切换勾选未勾选状态
void MainDialog::on_table_upload_cellClicked( int row, int column ) {
  qDebug() << __func__;
  MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_upload->item( row, 0 );

  if ( item0->checkState() == Qt::Checked ) {
    item0->setCheckState( Qt::Unchecked );
  } else {
    item0->setCheckState( Qt::Checked );
  }
}

//下载页面  点击某一行
void MainDialog::on_table_download_cellClicked( int row, int column ) {
  qDebug() << __func__;
  MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_download->item( row, 0 );

  if ( item0->checkState() == Qt::Checked ) {
    item0->setCheckState( Qt::Unchecked );
  } else {
    item0->setCheckState( Qt::Checked );
  }
}
// 25.设置登录信息
void MainDialog::slot_setInfo( QString name ) { ui->pb_name->setText( name ); }
// 26.根据时间戳获取信息
// 26.1根据时间戳获取下载文件信息
bool MainDialog::slot_getDownloadFileInfoByTimestamp( int timestamp, FileInfo &info ) {
  qDebug() << __func__;

  int rows = ui->table_download->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_download->item( i, 0 );

    if ( item0->m_info.timeStamp == timestamp ) {
      info = item0->m_info;
      return true;
    }
  }

  return false;
}
// 26.2根据时间戳获取上传文件信息
bool MainDialog::slot_getUploadFileInfoByTimestamp( int timestamp, FileInfo &info ) {
  qDebug() << __func__;
  int rows = ui->table_upload->rowCount();
  for ( int i = 0; i < rows; i++ ) {
    MyTableWidgetItem *item0 = (MyTableWidgetItem *) ui->table_upload->item( i, 0 );

    if ( item0->m_info.timeStamp == timestamp ) {
      info = item0->m_info;
      return true;
    }
  }
  return false;
}

bool MainDialog::isVideoFile( const QString &filePath ) {
  QMimeDatabase db;
  QMimeType     mime = db.mimeTypeForFile( filePath );
  return mime.name().startsWith( "video/" );
}
