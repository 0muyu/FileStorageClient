#include <QString>
#ifndef COMMON_H
#define COMMON_H

//文件信息
struct FileInfo {
  FileInfo() : fileid( 0 ), size( 0 ), pFile( nullptr ), pos( 0 ), timeStamp( 0 ), isPause( 0 ) {}

  int     fileid;
  QString name;
  QString dir;  //网盘目录
  QString time;
  int     size;  // int 32位  最大值2GB--假定网盘 文件都是2GB最大
  QString md5;
  QString type;
  QString absolutePath;  //文件本地绝对路径

  int pos;        //上传或下载到什么位置
  int timeStamp;  //时间戳
  int isPause;  //暂停  0 1

  //文件指针
  FILE* pFile;

  //字节单位换算

  static QString getsize( int size ) {
    QString res;
    int     count = 0;
    int     tmp   = size;

    // 1.计算进制位
    while ( tmp != 0 ) {
      tmp /= 1024;
      if ( tmp != 0 ) { count++; }
    }

    // 2.根据进制位进行换算
    switch ( count ) {
      case 0:                                                                                        // KB
        res = QString( "0.%1KB" ).arg( (int) ( size % 1024 / 1024.0 * 100 ), 2, 10, QChar( '0' ) );  // 0.0xKB
        break;

      case 1:  // KB
        res = QString( "%1.%2KB" ).arg( size / 1024 ).arg( (int) ( size % 1024 / 1024.0 * 100 ), 2, 10, QChar( '0' ) );
        break;

      case 2: res = QString( "%1.%2MB" ).arg( size / 1024 / 1024 ).arg( (int) ( size / 1024 % 1024 / 1024.0 * 100 ), 2, 10, QChar( '0' ) ); break;
      case 3: res = QString( "%1.%2GB" ).arg( size / 1024 / 1024 / 1024 ).arg( (int) ( size / 1024 / 1024 % 1024 / 1024.0 * 100 ), 2, 10, QChar( '0' ) ); break;
      default: break;
    }

    return res;
  }
};

#endif  // COMMON_H
