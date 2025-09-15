

#include "ckernel.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>
#include <QString>
#include <QTextCodec>

#include "TcpClientMediator.h"
#include "TcpServerMediator.h"
#include "md5.h"

// 定义添加加密内容中的密钥
#define MD5_KEY "1234"

#define setFunarr(a) m_funArr[a - _DEF_PACK_BASE]

static std::string getMD5(QString val);
static std::string getFileMD5(QString path);
void Utf8ToGB2312(char *gbbuf, int nlen, QString &utf8);
CKernel::CKernel(QObject *parent)
    : QObject(parent), m_id(0), m_curDir("/"), m_quit(false) {
  // 加载配置文件
  loadIniFile();
  // 设置协议和函数数组
  initFunArr();

  // 生成系统默认路径
  setSystemPath();

#ifdef USE_SERVER
  m_pMediatorServer = new TcpServerMediator();
  QObject::connect(m_pMediatorServer, &INetMediator::SIG_ReadyData, this,
                   &CKernel::slot_ServerReadyData);
  m_pMediatorServer->OpenNet();
#endif

  m_pMediatorClient = new TcpClientMediator();
  QObject::connect(m_pMediatorClient, &INetMediator::SIG_ReadyData, this,
                   &CKernel::slot_dealClientData);

  m_Player = new PlayerDialog;  // 获取播放器

  m_pLoginDialog = new LoginDialog();

  QObject::connect(m_pLoginDialog, &LoginDialog::sig_loginMessage, this,
                   &CKernel::slot_loginMessage);
  QObject::connect(m_pLoginDialog, &LoginDialog::sig_registerMessage, this,
                   &CKernel::slot_registerMessage);

  m_pLoginDialog->show();

  // 客户端连接真实地址
  m_pMediatorClient->OpenNet(m_ip.toStdString().c_str(), m_port.toInt());

  m_pMainDialog = new MainDialog();

  // 连接mainDialog的关闭信号和Ckernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_close, this,
                   &CKernel::slot_close);
  // 连接MainDialog的上传文件信号和Ckernel的处理槽函数
  QObject::connect(m_pMainDialog, SIGNAL(sig_uploadFile(QString, QString)),
                   this, SLOT(slot_uploadFile(QString, QString)));

  // 连接 Ckernel的更新进度条信号和MainDialog的处理槽函数
  QObject::connect(this, SIGNAL(sig_updataUploadFileProcess(int, int)),
                   m_pMainDialog,
                   SLOT(slot_updataUploadFileProcess(
                       int, int)));  // ？？？？？不是发送放实现的地方吗？？

  // 连接MainDialog的下载文件信号和CKernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_downloadFile, this,
                   &CKernel::slot_downloadFile);

  // 连接MainDialog的下载文件夹 信号和CKernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_downloadFolder, this,
                   &CKernel::slot_downloadFolder);

  // 连接 Ckernel的更新进度条信号和MainDialog的处理槽函数
  QObject::connect(this, SIGNAL(sig_updataDownloadFileProcess(int, int)),
                   m_pMainDialog,
                   SLOT(slot_updataDownloadFileProcess(int, int)));

  // 连接MainDialog 的新建文件夹信号和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_addFolder, this,
                   &CKernel::slot_addFolder);

  // 连接MainDialog 的跳转路径信号和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_changeDir, this,
                   &CKernel::slot_changeDir);

  // 连接MainDialog 的上传文件夹和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_uploadFolder, this,
                   &CKernel::slot_uploadFolder);

  // 连接MainDialog 的分享文件和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_shareFile, this,
                   &CKernel::slot_shareFile);

  // 连接MainDialog 的分享文件和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_getShare, this,
                   &CKernel::slot_getShare);

  // 连接MainDialog 的删除文件连接MainDialog
  QObject::connect(m_pMainDialog, &MainDialog::sig_deleteFile, this,
                   &CKernel::slot_deleteFile);

  // 连接MainDialog 的设置上传暂停和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_setUploadPause, this,
                   &CKernel::slot_setUploadPause);

  // 连接MainDialog 的设置下载暂停和Kernel的处理槽函数
  QObject::connect(m_pMainDialog, &MainDialog::sig_setDownloadPause, this,
                   &CKernel::slot_setDownloadPause);

  // 12.连接MainDialog  的获取url的信号连接MainDialog
  QObject::connect(m_pMainDialog, &MainDialog::sig_getUrl, this,
                   &CKernel::slot_getUrl);

#ifdef USE_SERVER
  // 测试代码：
  char strBuf[1024] = "hello this TcpClient";
  int len = strlen("hello this TcpClient") + 1;
  m_pMediatorClient->SendData(0, strBuf,
                              len);  // 客户端一定发送给服务器，套接字穿啥都可以

#endif
}

CKernel::CKernel(const CKernel &kernel) {}

CKernel::~CKernel() {}

// 1.处理loginDialog发送的登录信息
void CKernel::slot_loginMessage(QString tel, QString pwd) {
  qDebug() << __func__;

  STRU_LOGIN_RQ rq;
  strcpy_s(rq.tel, sizeof(rq.tel), tel.toStdString().c_str());
  // strcpy_s( rq.password, sizeof( rq.password ), pwd.toStdString().c_str() );
  strcpy(rq.password, getMD5(pwd).c_str());
  this->sendData((char *)&rq, sizeof(rq));
}
// 2.处理loginDialog发送的注册信息
void CKernel::slot_registerMessage(QString tel, QString pwd, QString name) {
  qDebug() << __func__;

  STRU_REGISTER_RQ rq;
  strcpy_s(rq.tel, sizeof(rq.tel), tel.toStdString().c_str());
  // strcpy_s( rq.password, sizeof( rq.password ), pwd.toStdString().c_str() );
  strcpy(rq.password, getMD5(pwd).c_str());

  strcpy_s(rq.name, sizeof(rq.name), name.toStdString().c_str());  //?????

  this->sendData((char *)&rq, sizeof(rq));
}

void Utf8ToGB2312(char *gbbuf, int nlen, QString &utf8) {
  // 转码的对象
  QTextCodec *gb2312code = QTextCodec::codecForName("gb2312");
  // QByteArray char 类型数组的封装类 里面有很多关于转码 和 写IO的操作
  QByteArray ba = gb2312code->fromUnicode(utf8);  // Unicode -> 转码对象的字符集

  strcpy_s(gbbuf, nlen, ba.data());
}

// 3.1处理MainDialog发送的上传文件信息
void CKernel::slot_uploadFile(QString path, QString dir) {
  qDebug() << "Ckernel:" << __func__;
  // 使用Qt的文件信息类
  QFileInfo qFileInfo(path);

  FileInfo Info;
  Info.absolutePath = path;
  Info.dir = dir;
  Info.md5 = QString::fromStdString(getFileMD5(path));

  Info.name = qFileInfo.fileName();
  Info.size = qFileInfo.size();
  Info.time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  Info.type = "file";

  char buf[1500] = "";
  Utf8ToGB2312(buf, 1500, path);
  Info.pFile = fopen(buf, "rb");
  if (!Info.pFile) {
    qDebug() << "file open failed\n";
    return;
  }

  int timestamp = QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
  // bug 修复:时间戳一样的问题
  while (m_mapTimestampToFileInfo.count(timestamp) > 0) {
    timestamp++;
  }

  Info.timeStamp = timestamp;
  qDebug() << "currentDateTime"
           << QDateTime::currentDateTime().toString("hhmmsszzz");
  qDebug() << "timestamp: " << timestamp;

  // 存储map 里 key 时间戳， value  文件信息
  m_mapTimestampToFileInfo[timestamp] = Info;

  // 发送上传文件请求
  STRU_UPLOAD_FILE_RQ rq;
  std::string strDir = dir.toStdString();
  strcpy(rq.dir, strDir.c_str());

  std::string strFileName = Info.name.toStdString();
  strcpy(rq.fileName, strFileName.c_str());

  strcpy(rq.fileType, "file");

  rq.md5;
  strcpy(rq.md5, Info.md5.toStdString().c_str());

  rq.size = Info.size;
  qDebug() << "file size: " << rq.size;

  strcpy(rq.time, Info.time.toStdString().c_str());

  rq.timestamp = timestamp;
  rq.userid = m_id;

  sendData((char *)&rq, sizeof(rq));
}
// 4.1处理MainDialog 发送下载文件的信号（什么目录下下载）
void CKernel::slot_downloadFile(int fileId, QString dir) {
  qDebug() << "Ckernel:" << __func__;

  STRU_DOWNLOAD_FILE_RQ rq;

  rq.userid = m_id;
  std::string strDir = dir.toStdString();
  strcpy_s(rq.dir, sizeof(rq.dir), strDir.c_str());
  rq.fileid = fileId;

  int timestamp = QDateTime::currentDateTime().toString("hhmmsszzz").toInt();

  // bug 修复:时间戳一样的问题
  while (m_mapTimestampToFileInfo.count(timestamp) > 0) {
    timestamp++;
  }

  rq.timestamp = timestamp;

  qDebug() << "userId: " << rq.userid;
  sendData((char *)&rq, sizeof(rq));
}
// 4.2处理MainDialog 发送下载文件夹的信号（什么目录下的文件夹下载）
void CKernel::slot_downloadFolder(int fileId, QString dir) {
  qDebug() << "Ckernel:" << __func__;

  STRU_DOWNLOAD_FOLDER_RQ rq;

  string strDir = dir.toStdString();
  strcpy(rq.dir, strDir.c_str());

  rq.fileid = fileId;

  int timestamp = QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
  while (m_mapTimestampToFileInfo.count(timestamp) > 0) {
    timestamp++;
  }
  rq.timestamp = timestamp;

  rq.userid = m_id;

  sendData((char *)&rq, sizeof(rq));
}
// 5.处理MainDialog 发送的新建文件夹的信号(什么名字什么目录下创建)
void CKernel::slot_addFolder(QString name, QString dir) {
  qDebug() << "Ckernel:" << __func__;

  // 发送请求包
  STRU_ADD_FOLDER_RQ rq;

  string strDir = dir.toStdString();
  strcpy(rq.dir, strDir.c_str());

  string strName = name.toStdString();
  strcpy(rq.fileName, strName.c_str());

  string strTime = QDateTime::currentDateTime()
                       .toString("yyyy-MM-dd hh:mm:ss")
                       .toStdString();
  strcpy(rq.time, strTime.c_str());

  rq.timestamp = QDateTime::currentDateTime().toString("hhmmsszzz").toInt();
  rq.userid = m_id;

  sendData((char *)&rq, sizeof(rq));
}

// 6.处理MainDialog 发送跳转路径的信号(实际就是重新获取文件列表)
void CKernel::slot_changeDir(QString dir) {
  qDebug() << __func__;

  // 更新当前目录
  m_curDir = dir;

  // 清空原来的列表
  m_pMainDialog->slot_deleteAllFileInfo();
  // 重新获取列表
  slot_getCurDirFileListRq();
}
// 7.处理MainDialog 发送上传文件夹的信号（什么路径 的文件夹，到什么目录下）
void CKernel::slot_uploadFolder(QString path, QString dir) {
  qDebug() << "CKernel::" << __func__;

  // 1.获取当前路径下的  文件夹名和路径
  QFileInfo info(path);
  QDir dr(path);

  // 文件夹处理： 当前需上传文件夹addFolder c:/项目  内容：/0702/  /33/  1.txt
  // 上传到 /userId/
  qDebug() << "folder: " << info.fileName() << "dir: " << dir;
  slot_addFolder(info.fileName(), dir);  // 新建文件夹
  // 2.  获取当前文件夹中的内容（文件夹+文件）
  QFileInfoList lst = dr.entryInfoList();  // 获取路径下所有文件的文件信息列表
  // 遍历所有列表项
  // 构造文件夹下的路径
  QString newDir = dir + info.fileName() + "/";
  for (int i = 0; i < lst.size(); i++) {
    QFileInfo file = lst.at(i);
    // 2.1 如果是 .  或 .. 继续
    if (file.fileName() == "." || file.fileName() == "..") {
      continue;
    }

    // 2.2 如果是文件 上传文件  ->文件的绝对路径  传到什么目录下
    // /userId/新建的文件夹（与上传文件夹同名）
    if (file.isFile()) {
      qDebug() << "file: " << file.absoluteFilePath() << "dir: " << newDir;
      slot_uploadFile(file.absoluteFilePath(), newDir);
    }

    // 2.3 如果是文件夹，需要递归发送
    qDebug() << "folder path: " << file.absoluteFilePath() << "dir: " << dir;
    if (file.isDir()) {
      slot_uploadFolder(file.absoluteFilePath(), newDir);
    }
  }
}
// 8.处理MainDialog 发送分享文件的信号（什么目录下的什么文件列表）
void CKernel::slot_shareFile(QVector<int> shareArr, QString dir) {
  qDebug() << "CKernel::" << __func__;

  int packLen = sizeof(STRU_SHARE_FILE_RQ) + sizeof(int) * shareArr.size();

  STRU_SHARE_FILE_RQ *rq = (STRU_SHARE_FILE_RQ *)malloc(packLen);
  rq->init();

  string strDir = dir.toStdString();
  strcpy(rq->dir, strDir.c_str());

  rq->itemCount = shareArr.size();
  for (int i = 0; i < shareArr.size(); i++) {
    rq->fileidArray[i] = shareArr[i];
  }

  QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  strcpy(rq->shareTime, time.toStdString().c_str());
  rq->userid = m_id;

  sendData((char *)rq, packLen);
  free(rq);
}
// 9.处理MainDialog
// 发送获取分享文件的信号（获取什么分享码的文件，添加到什么目录下）
void CKernel::slot_getShare(int code, QString dir) {
  qDebug() << "CKernel::" << __func__;

  STRU_GET_SHARE_RQ rq;

  string strDir = dir.toStdString();
  strcpy(rq.dir, strDir.c_str());

  rq.shareLink = code;

  string time = QDateTime::currentDateTime()
                    .toString("yyyy-MM-dd hh:mm:ss")
                    .toStdString();
  strcpy(rq.time, time.c_str());

  rq.userid = m_id;

  sendData((char *)&rq, sizeof(rq));
}
// 10.处理MainDialog 发送删除文件的信号（什么目录下的什么文件列表）
void CKernel::slot_deleteFile(QVector<int> deleteArr, QString dir) {
  qDebug() << "CKernel::" << __func__;

  int packLen = sizeof(STRU_DELETE_FILE_RQ) + sizeof(int) * deleteArr.size();
  STRU_DELETE_FILE_RQ *rq = (STRU_DELETE_FILE_RQ *)malloc(packLen);
  rq->init();
  string strDir = dir.toStdString();
  strcpy(rq->dir, strDir.c_str());

  rq->fileCount = deleteArr.size();

  for (int i = 0; i < rq->fileCount; i++) {
    rq->fileidArray[i] = deleteArr[i];
  }
  rq->userid = m_id;

  sendData((char *)rq, packLen);
  free(rq);
}
// 11.给Kernel 发送设置上传暂停的信号  0开始  1暂停
void CKernel::slot_setUploadPause(int timestamp, int isPause) {
  qDebug() << __func__;
  // 1.isPause 1从正在下载变为暂停   isPause 0从暂停变为开始继续下载

  // 2.找到文件结构体
  if (m_mapTimestampToFileInfo.count(timestamp) > 0) {
    // 2.1查询map中有  程序未退出的情况 直接置位
    m_mapTimestampToFileInfo[timestamp].isPause = isPause;
  } else {
    // 2.2 map中没有 证明程序退出了  断点续传 需要走协议 TODO
    FileInfo info;

    if (!m_pMainDialog->slot_getUploadFileInfoByTimestamp(timestamp, info)) {
      qDebug() << "未找到上传文件信息";
      return;
    }

    // 2.2.1 将路径转化为ASCII
    char pathbuf[1000] = "";
    Utf8ToGB2312(pathbuf, 1000, info.absolutePath);

    // 2.2.2 打开文件 二进制只读
    info.pFile = fopen(pathbuf, "rb");
    if (!info.pFile) {
      qDebug() << "opne file failed: " << info.absolutePath;
      return;
    }

    // 2.2.3 设置开始标志
    info.isPause = 0;

    m_mapTimestampToFileInfo[timestamp] = info;

    // 3.发送请求
    STRU_CONTINUE_UPLOAD_RQ rq;

    string strDir = info.dir.toStdString();
    strcpy(rq.dir, strDir.c_str());
    rq.fileid = info.fileid;
    rq.timestamp = timestamp;
    rq.userid = m_id;

    sendData((char *)&rq, sizeof(rq));
  }
}
// 11.给Kernel 发送设置下载暂停的信号  0开始  1暂停
void CKernel::slot_setDownloadPause(int timestamp, int isPause) {
  qDebug() << __func__;

  // 1.isPause 1从正在下载变为暂停   isPause 0从暂停变为开始继续下载

  // 2.找到文件结构体

  if (m_mapTimestampToFileInfo.count(timestamp) > 0) {
    // 2.1查询map中有  程序未退出的情况 直接置位
    m_mapTimestampToFileInfo[timestamp].isPause = isPause;
  } else {
    // 2.2 map中没有 证明程序退出了  TODO断点续传 需要走协议
    // 创建信息结构体，装到map中；信息保存再控件中
    FileInfo info;

    if (!m_pMainDialog->slot_getDownloadFileInfoByTimestamp(timestamp, info)) {
      qDebug() << "未找到下载文件信息";
      return;
    }

    // 2.2.1 将路径转化未ASCII
    char pathbuf[1000] = "";
    Utf8ToGB2312(pathbuf, 1000, info.absolutePath);

    // 2.2.2 打开文件 二进制追加（不能是w,会覆盖原内容）
    info.pFile = fopen(pathbuf, "ab");
    if (!info.pFile) {
      qDebug() << "opne file failed: " << info.absolutePath;
      return;
    }

    // 2.2.3 设置开始标志
    info.isPause = 0;

    m_mapTimestampToFileInfo[timestamp] = info;

    // 3.发协议，告知服务器 文件下载到哪里了，然后服务器的
    // pos跳转到哪里，从那里继续读取数据，然后发送文件块；
    // 服务器接收有两种可能 1.文件信息还在(客户端异常退出很快好了，没有超过预定
    // 删除客户端所有信息的时间)  2.不在(超过了时间)

    STRU_CONTINUE_DOWNLOAD_RQ rq;

    string strDir = info.dir.toStdString();
    strcpy(rq.dir, strDir.c_str());
    rq.fileid = info.fileid;
    rq.pos = info.pos;
    rq.timestamp = timestamp;
    rq.userid = m_id;

    sendData((char *)&rq, sizeof(rq));
  }
}
// 12.处理MainDialog  发送获取url的信号(获取什么文件的url)
void CKernel::slot_getUrl(int fileId, QString dir) {
  qDebug() << __func__;

  _STRU_GET_URL_RQ rq;
  rq.userid = m_id;
  rq.fileid = fileId;
  strcpy(rq.dir, dir.toStdString().c_str());

  sendData((char *)&rq, sizeof(rq));
}

// 1.加载网络配置文件
void CKernel::loadIniFile() {
  qDebug() << "CKernel::" << __func__;
  // 默认值
  //   m_ip   = "10.51.40.39";  // 这是服务器ip 也就是wsl 的ifconfig的ip
  m_ip = "172.29.0.235";
  m_port = "8004";

  // 1.获取 exe同级目录
  QString path = QCoreApplication::applicationDirPath() + "./config.ini";
  // 2.检测 ini文件是否存在
  QFileInfo info(path);
  if (info.exists()) {
    qDebug() << "创建一个 .ini";
    // 2.1 ini 文件存在：加载
    //(1)打开组
    qDebug() << "path: " << path;
    QSettings setting(path, QSettings::IniFormat);  // 打开
    setting.beginGroup("net");

    //(2) 加载(读)
    // QVariant 类似 c++中的auto
    QVariant strIp = setting.value("ip", "");
    QVariant strPort = setting.value("port", "");
    if (!strIp.toString().isEmpty()) {
      m_ip = strIp.toString();
    }
    if (!strPort.toString().isEmpty()) {
      m_port = strPort.toString();
    }

    //(3)关闭组
    setting.endGroup();
  } else {
    // 2.2 ini 文件不存在 设置
    QSettings setting(path, QSettings::IniFormat);  // 没有会创建
    //(1)打开组
    setting.beginGroup("net");
    //(2) 设置  key value
    setting.setValue("ip", m_ip);
    setting.setValue("port", m_port);

    //(3)关闭组
    setting.endGroup();
  }

  // 3.打印获取到的ip 和端口
  qDebug() << "ip: " << m_ip << " port:" << m_port;
}

// 2.私有的设置协议与函数的映射数组
void CKernel::initFunArr() {
  qDebug() << __func__;

  // 1.初始化
  memset(m_funArr, 0, _DEF_PACK_COUNT);

  // 2.通过协议偏移量 找到对应的处理函数
  setFunarr(_DEF_PACK_LOGIN_RS) = &CKernel::slot_dealLoginRs;
  setFunarr(_DEF_PACK_REGISTER_RS) = &CKernel::slot_dealRegisterRs;
  setFunarr(_DEF_PACK_UPLOAD_FILE_RS) = &CKernel::slot_dealUploadFileRs;
  setFunarr(_DEF_PACK_FILE_CONTENT_RS) = &CKernel::slot_dealFileContentRs;
  setFunarr(_DEF_PACK_GET_FILE_INFO_RS) =
      &CKernel::slot_dealGetCurDirFileListRs;

  setFunarr(_DEF_PACK_FILE_HEADER_RQ) =
      &CKernel::slot_dealFileHeaderRq;  // 没错,就是rq
  setFunarr(_DEF_PACK_FILE_CONTENT_RQ) =
      &CKernel::slot_dealFileContentRq;  // 没错,就是rq
  setFunarr(_DEF_PACK_ADD_FOLDER_RS) = &CKernel::slot_dealAddFolderRs;
  setFunarr(_DEF_PACK_QUICK_UPLOAD_RS) = &CKernel::slot_dealQuickUploadRs;

  setFunarr(_DEF_PACK_SHARE_FILE_RS) = &CKernel::slot_dealShareFileRs;
  setFunarr(_DEF_PACK_MY_SHARE_RS) = &CKernel::slot_dealGetShareFileListRs;

  setFunarr(_DEF_PACK_GET_SHARE_RS) = &CKernel::slot_dealGetShareRs;

  setFunarr(_DEF_PACK_FOLDER_HEADER_RQ) = &CKernel::slot_dealFolderHeaderRq;

  setFunarr(_DEF_PACK_DELETE_FILE_RS) = &CKernel::slot_dealDeleteFileRs;
  setFunarr(_DEF_PACK_CONTINUE_UPLOAD_RS) = &CKernel::slot_dealContinueUploadRs;

  setFunarr(_DEF_PACK_GET_URL_RS) = &CKernel::slot_dealGetUrlRs;
}

// 3.私有的发送函数
void CKernel::sendData(char *buf, int len) {
  //  qDebug() << __func__;
  qDebug() << *(int *)buf;
  m_pMediatorClient->SendData(123, buf, len);
}
// 4.设置系统路径(系统路径组成：exe同级 ./NetDisk)
void CKernel::setSystemPath() {
  qDebug() << __func__;
  QString path = QCoreApplication::applicationDirPath() + "/NetDisk";

  QDir dir;

  // 没有文件夹，创建文件夹
  if (!dir.exists(path)) {
    dir.mkdir(path);  // 只能创建一层
  }

  // 保存默认路径
  m_sysPath = path;
}

// 1.生成MD5函数
static std::string getMD5(QString val) {
  qDebug() << __func__;
  QString str = QString("%1_%2").arg(val).arg(MD5_KEY);

  MD5 md(str.toStdString());

  qDebug() << str << "md5: " << md.toString().c_str();

  return md.toString();
}

// 2.获取文件 MD5
static std::string getFileMD5(QString path) {
  qDebug() << __func__;
  // 1.打卡问价 ，读取文件内容，读到md5类，生成md5;
  FILE *pFile = nullptr;

  // 2.fopen如果有中文，支持 ANSI编码，生成md5；
  // path里面是utf8(qt默认的)编码
  char buf[1500] = "";
  Utf8ToGB2312(buf, 1500, path);

  pFile = fopen(buf, "rb");
  if (!pFile) {
    qDebug() << "file md5 open fail";
    return string();
  }

  int len = 0;
  MD5 md;

  do {
    // 缓冲区，一次读多少，读多少次，文件指针，返回值读成功次数
    len = fread(buf, 1, 1500, pFile);
    md.update(buf, len);  // 不断拼接文本,不断更新md5;

    // 为避免主线程阻塞，影响时间循环，加入下面的处理 将信号取出并执行
    QCoreApplication::processEvents(QEventLoop::AllEvents, 90);
  } while (len > 0);

  // 关闭文件
  fclose(pFile);

  // 打印验证生成的文件md5
  qDebug() << "file md5:" << md.toString().c_str();

  return md.toString();
}

void CKernel::slot_close() {
  qDebug() << "Ckernel::" << __func__;

  m_quit = true;

  m_pMediatorClient->CloseNet();
  delete m_pMediatorClient;
  m_pMainDialog->hide();
  delete m_pMainDialog;
  delete m_pLoginDialog;
}

#ifdef USE_SERVER
// 1.服务器的处理 槽函数
void CKernel::slot_ServerReadyData(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << "Ckernel::" << __func__;

  QString str = QString("来自客户端:%1").arg(QString::fromStdString(buf));
  QMessageBox::about(
      m_pMainDialog, "提示",
      str);  // about 是阻塞函数
             // 且是一个模态窗口，也就是说该窗口弹出后其他窗口不能再点；

  // 测试代码：
  char strBuf[1024] = "hi,I'm TcpServer";
  int len = strlen("hi,I'm TcpServer") + 1;
  m_pMediatorServer->SendData(lSendIP, strBuf, len);

  delete[] buf;
}
#endif

// 1.客户端的处理 槽函数
void CKernel::slot_dealClientData(unsigned int lSendIP, char *buf, int nlen) {
  //  qDebug() << "Ckernel::" << __func__;
  // 测试代码
  /* QString str = QString( "来自服务器:%1" ).arg( QString::fromStdString( buf )
   ); QMessageBox::about( m_pMainDialog, "提示", str ); */ // about 是阻塞函数

  // 且是一个模态窗口，也就是说该窗口弹出后其他窗口不能再点；
  // 1.取出协议
  int type = *(int *)buf;

  // 2.找打对应的处理函数
  if (type >= _DEF_PACK_BASE && type < _DEF_PACK_BASE + _DEF_PACK_COUNT) {
    int index = type - _DEF_PACK_BASE;

    PFUN pFun = m_funArr[index];
    if (pFun) {
      (this->*pFun)(lSendIP, buf, nlen);
    }
  }

  delete[] buf;
}

// 2. 处理登录回复
void CKernel::slot_dealLoginRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_LOGIN_RS *rs = (STRU_LOGIN_RS *)buf;

  // 2.根据结果显示给用户
  switch (rs->result) {
    case tel_not_exist:
      QMessageBox::about(m_pLoginDialog, "提示", "手机号未注册");
      break;

    case password_error:
      QMessageBox::about(m_pLoginDialog, "提示", "密码错误");
      break;

    case login_success:
      // 1.前台
      m_pLoginDialog->hide();
      m_pMainDialog->show();
      // 2.后台
      m_name = rs->name;
      m_id = rs->userid;
      m_pMainDialog->slot_setInfo(m_name);

      // TODO: 登录后主动获取获取根目录下的文件列表和分享文件的列表

      m_curDir = "/";
      slot_getCurDirFileListRq();
      slot_getShareFileListRq();

      // 初始化库
      initDatabase(m_id);

      break;
  }
}

// 3.处理注册回复
void CKernel::slot_dealRegisterRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_REGISTER_RS *rs = (STRU_REGISTER_RS *)buf;

  // 2.根据结果显示给用户
  switch (rs->result) {
    case tel_is_exist:
      QMessageBox::about(m_pLoginDialog, "提示", "电话号已注册");
      break;
    case name_is_exist:
      QMessageBox::about(m_pLoginDialog, "提示", "昵称已存在");
      break;

    case register_success:
      QMessageBox::about(m_pLoginDialog, "提示", "注册成功");
      break;
  }
}
// 4.处理上传文件回复
void CKernel::slot_dealUploadFileRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << "CKernel::" << __func__;

  // 1.拆包
  STRU_UPLOAD_FILE_RS *rs = (STRU_UPLOAD_FILE_RS *)buf;

  // 2.根据返回结果进行处理
  // 2.1 结果未假，直接结束处理
  if (!rs->result) {
    QMessageBox::about(m_pMainDialog, "提示", "上传文件失败");
    return;
  }
  // 2.2结果为真，获取文件 信息
  FileInfo &info = m_mapTimestampToFileInfo[rs->timestamp];  // 注意是使用引用
  // 2.2.1更新文件fieldid;
  info.fileid = rs->fileid;
  // 2.2.2 TODO: 插入上传信息 到MainDialog 的"上传中"  控件里
  // 缓存上传的任务
  // qDebug()<<"dealUploadFileRs "
  slot_writeUploadTask(info);
  m_pMainDialog->slot_insertUploadFile(
      info);  // 这里改成了public slots或友元CKernel

  // 3. 发送文件块请求
  STRU_FILE_CONTENT_RQ rq;
  rq.fileid = info.fileid;
  rq.timestamp = rs->timestamp;
  rq.userid = m_id;
  rq.len = fread(rq.content, 1, _DEF_BUFFER, info.pFile);
  qDebug() << "read_len: " << rq.len;

  sendData((char *)&rq, sizeof(rq));
}

// 5.处理文件内容传输回复
void CKernel::slot_dealFileContentRs(unsigned int lSendIP, char *buf,
                                     int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_FILE_CONTENT_RS *rs = (STRU_FILE_CONTENT_RS *)buf;

  // 2.查找文件信息结构体
  if (m_mapTimestampToFileInfo.find(rs->timestamp) ==
      m_mapTimestampToFileInfo.end()) {
    qDebug() << "file not found";
    return;
  }
  FileInfo &info = m_mapTimestampToFileInfo[rs->timestamp];

  // 2.循环判断是否暂停
  while (info.isPause) {
    QThread::msleep(100);

    // 为避免主线程阻塞，影响时间循环，加入下面的处理 将信号取出并执行
    QCoreApplication::processEvents(QEventLoop::AllEvents, 90);

    // 避免程序退出，但是还处于循环中无法正常结束程序

    if (m_quit) {
      return;
    }
  }

  // 3.根据回复结果进行处理结果

  if (!rs->result) {  // 文件内容上传失败 ，回退
    fseek(info.pFile, -1 * (rs->len), SEEK_CUR);
  } else {
    // 更新pos
    info.pos += rs->len;

    // TODO：更新上传进度
    // 方案1：写信号槽 考虑多线程;
    // 方案2：直接调用 一定是当前函数在主线程
    Q_EMIT sig_updataUploadFileProcess(info.timeStamp, info.pos);

    // 判断文件内容是否上传完毕,是，关闭文件，回收节点
    if (info.pos >= info.size) {
      // 完成任务，删除上传记录
      slot_deleteUploadTask(info);

      fclose(info.pFile);
      m_mapTimestampToFileInfo.erase(rs->timestamp);

      // 清空所有文件列表，重新获取列表
      m_pMainDialog->slot_deleteAllFileInfo();
      // 重新获取列表
      slot_getCurDirFileListRq();

      return;
    }
  }

  // 4.继续发送文件块
  STRU_FILE_CONTENT_RQ rq;
  rq.fileid = rs->fileid;
  rq.timestamp = rs->timestamp;
  rq.userid = m_id;
  rq.len = fread(rq.content, 1, _DEF_BUFFER, info.pFile);
  // qDebug()<<"fread len: "<<rs;
  sendData((char *)&rq, sizeof(rq));
}

// 6.处理获取文件列表的回复
void CKernel::slot_dealGetCurDirFileListRs(unsigned int lSendIP, char *buf,
                                           int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_GET_FILE_INFO_RS *rs = (STRU_GET_FILE_INFO_RS *)buf;

  // 先删除原来的列表
  m_pMainDialog->slot_deleteAllFileInfo();

  // 2.遍历文件信息数组
  int count = rs->count;
  qDebug() << "count: " << count;
  for (int i = 0; i < count; i++) {
    FileInfo Info;
    Info.fileid = rs->fileInfo[i].fileId;
    Info.type = QString::fromStdString(rs->fileInfo[i].fileType);
    // qDebug() << "type: " << rs->fileInfo[ i ].fileType;
    Info.name = QString::fromStdString(rs->fileInfo[i].name);
    Info.size = rs->fileInfo[i].size;

    Info.time = rs->fileInfo[i].time;
    // qDebug() << "time: " << Info.time;

    // 3.插入到控件中
    m_pMainDialog->slot_insertFileInfo(Info);
  }
}
// 7.处理文件头请求
void CKernel::slot_dealFileHeaderRq(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << "Ckernel::" << __func__;
  // 1.拆包
  STRU_FILE_HEADER_RQ *rq = (STRU_FILE_HEADER_RQ *)buf;

  // 2.创建文件信息结构体并保存在map中
  FileInfo info;
  // 指定了默认路径 sysPath(exe同级路径，不含最后的'/')+dir+name
  //  2.1 TODO: dir可能有很多层 需要循环创建目录
  QString tmpDir = QString::fromStdString(rq->dir);  // eg: /NetDisk/aaa 1.text;
  QStringList dirList = tmpDir.split("/");           // NetDisk aaa
  QString pathSum = m_sysPath;
  for (QString &node : dirList) {
    if (!node.isEmpty()) {
      pathSum += "/";
      pathSum += node;

      QDir dir;
      if (!dir.exists(pathSum)) {
        dir.mkdir(pathSum);
      }
    }
  }
  // 2.2 info 赋值
  info.dir = QString::fromStdString(rq->dir);
  info.name = QString::fromStdString(rq->fileName);
  info.absolutePath =
      QString("%1%2%3").arg(m_sysPath).arg(info.dir).arg(info.name);
  info.fileid = rq->fileid;
  info.md5 = QString::fromStdString(rq->md5);
  info.size = rq->size;
  info.time = QDateTime::currentDateTime().toString("yyyy-mm-dd hh:mm:ss");
  info.timeStamp = rq->timestamp;
  info.type = "file";

  char pathBuf[1024] = "";
  Utf8ToGB2312(pathBuf, 1024, info.absolutePath);
  info.pFile = fopen(pathBuf, "wb");
  if (!info.pFile) {
    qDebug() << "file open fail\n";
    return;
  }

  // 2.3 保存在map中
  m_mapTimestampToFileInfo[rq->timestamp] = info;

  // 3.保存下载文件信息 到MainDialog的"下载中"控件
  m_pMainDialog->slot_insertDownloadFile(info);

  // 更新数据库中 缓存下载的任务
  slot_writeDownloadTask(info);

  // 4.回复
  STRU_FILE_HEADER_RS rs;
  rs.fileid = rq->fileid;
  rs.result = 1;
  rs.timestamp = rq->timestamp;
  rs.userid = m_id;

  sendData((char *)&rs, sizeof(rs));
}
// 8.处理文件内容请求
void CKernel::slot_dealFileContentRq(unsigned int lSendIP, char *buf,
                                     int nlen) {
  qDebug() << "Ckernel::" << __func__;
  // 1.拆包
  STRU_FILE_CONTENT_RQ *rq = (STRU_FILE_CONTENT_RQ *)buf;

  // 2.先获取文件信息结构体
  if (m_mapTimestampToFileInfo.count(rq->timestamp) == 0) {
    qDebug() << "m_mapTimestampToFileInfo.count(rq->timestamp) error\n";
    return;
  }
  FileInfo &info = m_mapTimestampToFileInfo[rq->timestamp];
  // 2.循环判断是否暂停
  while (info.isPause) {
    QThread::msleep(100);

    // 为避免主线程阻塞，影响时间循环，加入下面的处理 将信号取出并执行
    QCoreApplication::processEvents(QEventLoop::AllEvents, 90);

    // 避免程序退出，但是还处于循环中无法正常结束程序

    if (m_quit) {
      return;
    }
  }

  // 3.文件内容回复
  STRU_FILE_CONTENT_RS rs;
  // 3.1先向文件中写入传过来的内容
  int len = fwrite(rq->content, 1, rq->len, info.pFile);
  // 3.1.1检测是否成功写入请求的长度
  if (len != rq->len) {
    // 不成功，pos返回写入前的位置
    rs.result = 0;
    fseek(info.pFile, -1 * len, SEEK_CUR);
  } else {
    // 成功pos+len
    rs.result = 1;
    info.pos += len;

    // 发送 获取下载文件进度的信号(更新进度)
    Q_EMIT sig_updataDownloadFileProcess(info.timeStamp, info.pos);
  }

  // 3.1.2检测是否已经读满文件
  if (info.pos >= info.size) {
    // 完成任务，删除下载记录
    slot_deleteDownloadTask(info);

    // 读满，关闭文件回收节点
    fclose(info.pFile);
    m_mapTimestampToFileInfo.erase(rq->timestamp);
  }

  rs.len = rq->len;
  rs.timestamp = rq->timestamp;
  rs.fileid = rq->fileid;
  rs.userid = m_id;

  sendData((char *)&rs, sizeof(rs));
}
// 9.处理新建文件夹回复
void CKernel::slot_dealAddFolderRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << "CKernel::" << __func__;
  // 1.拆包
  STRU_ADD_FOLDER_RS *rs = (STRU_ADD_FOLDER_RS *)buf;

  // 2.根据回复结果进行处理
  if (rs->result == 0) {
    qDebug() << "result==0";
    return;
  }

  // 先删除原来的列表
  m_pMainDialog->slot_deleteAllFileInfo();
  // 再更新文件列表（调用更新文件列表的方法发送请求重新获取）
  slot_getCurDirFileListRq();
}
// 10.处理秒传回复
void CKernel::slot_dealQuickUploadRs(unsigned int lSendIP, char *buf,
                                     int nlen) {
  qDebug() << "CKernel::" << __func__;
  // 1.拆包
  STRU_QUICK_UPLOAD_RS *rs = (STRU_QUICK_UPLOAD_RS *)buf;

  // 2.获取文件信息
  if (m_mapTimestampToFileInfo.count(rs->timestamp) == 0) {
    qDebug() << "m_mapTimestampToFileInfo.count (rs->timestamp)==0";
    return;
  }
  FileInfo info = m_mapTimestampToFileInfo[rs->timestamp];
  // 3.关闭文件
  if (info.pFile) {
    fclose(info.pFile);
  }
  // 4.写入上传已完成信息
  m_pMainDialog->slot_insertFinishFile(info);
  // m_pMainDialog->slot_insertDownloadFinishFile( info );
  // 5.判断是当前目录前提下  更新文件列表
  if (m_curDir == info.dir) {
    m_pMainDialog->slot_deleteAllFileInfo();
    slot_getCurDirFileListRq();
  }
  // 6.删除节点//？？？？？
  m_mapTimestampToFileInfo.erase(rs->timestamp);
}
// 11.处理分享文件的回复
void CKernel::slot_dealShareFileRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << __func__;
  // 1.拆包
  STRU_SHARE_FILE_RS *rs = (STRU_SHARE_FILE_RS *)buf;

  // 2.判断是否成功
  if (rs->result != 1) {
    qDebug() << "分享失败";
    return;
  }
  // 3.刷新我的分享界面 发送获取分享文件列表请求
  slot_getShareFileListRq();
}
// 12.处理 获取分享文件列表的回复
void CKernel::slot_dealGetShareFileListRs(unsigned int lSendIP, char *buf,
                                          int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_MY_SHARE_RS *rs = (STRU_MY_SHARE_RS *)buf;

  // 2.遍历处理返回的 分享成功的文件项,将其添加到控件上

  m_pMainDialog->slot_deleteAllShareFileInfo();
  int count = rs->itemCount;
  for (int i = 0; i < count; i++) {
    m_pMainDialog->slot_insertShareList(rs->items[i].name,
                                        rs->items[i].shareLink,
                                        rs->items[i].size, rs->items[i].time);
  }
}
// 13.处理  获取分享文件的回复
void CKernel::slot_dealGetShareRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_GET_SHARE_RS *rs = (STRU_GET_SHARE_RS *)buf;

  // 2.根据返回结果处理
  if (rs->result == 0) {
    QMessageBox::about(m_pMainDialog, "提示", "获取文件分享失败");
    return;
  } else {
    if (QString::fromStdString(rs->dir) == m_curDir) {
      slot_getCurDirFileListRq();
    }
  }
}
// 14.处理 文件夹头请求
void CKernel::slot_dealFolderHeaderRq(unsigned int lSendIP, char *buf,
                                      int nlen) {
  qDebug() << "Ckernel::" << __func__;
  // 1.拆包
  STRU_FOLDER_HEADER_RQ *rq = (STRU_FOLDER_HEADER_RQ *)buf;

  // 2.创建目录
  // dir可能有很多层 需要循环创建目录
  QString tmpDir = QString::fromStdString(
      rq->dir);  // eg:  /NetDisk/aaa/
                 // test/由于是创建文件夹，因此拆分完创建在什么路径下后，需要再创建一个文件
  QStringList dirList = tmpDir.split("/");  // 分割函数分割 NetDisk aaa
  QString pathSum = m_sysPath;
  for (QString &node : dirList) {
    if (!node.isEmpty()) {
      pathSum += "/";
      pathSum += node;

      QDir dir;
      if (!dir.exists(pathSum)) {
        dir.mkdir(pathSum);
      }
    }
  }

  // 3.在目录下 创建文件夹
  pathSum += "/";
  pathSum += QString::fromStdString(rq->fileName);
  QDir dir;
  if (!dir.exists(pathSum)) {
    dir.mkdir(pathSum);
  }
}
// 15.处理 删除文件的回复
void CKernel::slot_dealDeleteFileRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << "Ckernel::" << __func__;

  // 1.拆包
  STRU_DELETE_FILE_RS *rs = (STRU_DELETE_FILE_RS *)buf;
  // 2.根据回复结果进行处理
  if (rs->result == 1) {
    if (QString::fromStdString(rs->dir) == m_curDir) {
      // 先删除，再获取更新列表
      m_pMainDialog->slot_deleteAllFileInfo();
      slot_getCurDirFileListRq();
    }
  }
}
// 16.处理 上传续传的回复
void CKernel::slot_dealContinueUploadRs(unsigned int lSendIP, char *buf,
                                        int nlen) {
  qDebug() << __func__;

  // 1.拆包
  STRU_CONTINUE_UPLOAD_RS *rs = (STRU_CONTINUE_UPLOAD_RS *)buf;

  // 2.通过 通过timestamp 拿到文件信息
  if (m_mapTimestampToFileInfo.count(rs->timestamp) == 0) {
    qDebug() << "找不到文件信息";
    return;
  }

  FileInfo &info = m_mapTimestampToFileInfo[rs->timestamp];

  // 3.文件读取位置跳转  pos更新  界面显示 百分比更新
  info.pos = rs->pos;
  qDebug() << "info.pos: " << info.pos;
  fseek(info.pFile, info.pos, SEEK_SET);  // 从起始位置跳转pos那么多

  m_pMainDialog->slot_updataUploadFileProcess(info.timeStamp, info.pos);

  // 4.继续发送文件块请求

  STRU_FILE_CONTENT_RQ rq;

  rq.fileid = rs->fileid;

  rq.timestamp = rs->timestamp;
  rq.userid = m_id;
  rq.len = fread(rq.content, 1, _DEF_BUFFER, info.pFile);
  sendData((char *)&rq, sizeof(rq));
}
// 17.处理  获取url的回复
void CKernel::slot_dealGetUrlRs(unsigned int lSendIP, char *buf, int nlen) {
  qDebug() << __func__;

  _STRU_GET_URL_RS *rs = (_STRU_GET_URL_RS *)buf;

  if (!rs->result) {
    qDebug() << "获取url失败";
    return;
  }

  // 1.关闭当前的状态
  if (m_Player->pVediaPlayer()->playerState() != PlayerState::Stop) {
    m_Player->pVediaPlayer()->stop(true);
  }

  // 2.显示播放窗口
  m_Player->show();

  // 3.设置fileName
  m_Player->pVediaPlayer()->setFileName(rs->url);

  qDebug() << "url:" << rs->url;
  avformat_network_init();  // 支持打开网络文件

  m_Player->pVediaPlayer()->start();

  m_Player->slot_PlayerStateChanged(PlayerState::Playing);
}

// 请求槽函数
//  1.获取文件列表
void CKernel::slot_getCurDirFileListRq() {
  qDebug() << __func__;

  // 向服务器发送获取当前文件列表
  STRU_GET_FILE_INFO_RQ rq;

  rq.userId = m_id;
  std::string strDir = m_curDir.toStdString();
  strcpy(rq.dir, strDir.c_str());

  sendData((char *)&rq, sizeof(rq));
}
// 2.获取分享文件列表
void CKernel::slot_getShareFileListRq() {
  qDebug() << __func__;

  STRU_MY_SHARE_RQ rq;
  rq.userid = m_id;

  sendData((char *)&rq, sizeof(rq));
}

// 初始化数据库
void CKernel::initDatabase(int id) {
  qDebug() << __func__;
  m_sql = new CSqlite;

  // 1.首先找到 exe 同级目录  /database/id.db
  QString path = QCoreApplication::applicationDirPath() + "/database/";
  qDebug() << path;
  QDir dir;
  // 1.1检测路径是否存在，要不要创建
  if (!dir.exists(path)) {
    qDebug() << "madir";
    dir.mkdir(path);
  }

  path = path + QString("%1.db").arg(id);
  qDebug() << path;
  // 1.2查看有没有这个文件
  // 1.2.1有直接加载
  QFileInfo info(path);
  if (info.exists()) {
    // 1.连接数据库
    m_sql->ConnectSql(path);

    // 2.获取上传和下载的任务列表
    QList<FileInfo> uploadTaskList;
    QList<FileInfo> downloadTaskList;

    slot_getUploadTask(uploadTaskList);
    slot_getDownloadTask(downloadTaskList);

    // 3.处理上传和下载的任务
    // 3.1上传任务
    for (FileInfo &info : uploadTaskList) {
      QFileInfo file(info.absolutePath);
      qDebug() << "upload path: " << info.absolutePath;
      if (!file.exists()) {
        continue;
      }

      // 修改任务状态
      info.isPause = true;  // ？？？？？？？？？？？？

      // 上传的续传  在控件上看不到上传的进度
      m_pMainDialog->slot_insertUploadFile(info);

      // TODO 需要写一个协议获取当前位置

      // 同步控件的位置
      // qDebug() << "upload  info pos: " << info.pos;
      m_pMainDialog->slot_updataUploadFileProcess(info.timeStamp, info.pos);
    }

    // 3.2 加载下载任务
    for (FileInfo &info : downloadTaskList) {
      QFileInfo file(info.absolutePath);
      qDebug() << "download path: " << info.absolutePath;
      if (!file.exists()) {
        continue;
      }

      // 修改任务状态
      info.isPause = true;

      // 进行到多少可以知道
      info.pos = file.size();  //???????????

      m_pMainDialog->slot_insertDownloadFile(
          info);  // 到底插入Upload 还是 download

      // TODO 获取当前位置

      // 同步控件的位置
      qDebug() << "download  file size: " << file.size();
      m_pMainDialog->slot_updataDownloadFileProcess(info.timeStamp,
                                                    file.size());
    }

  } else {  // 1.2.2 没有创建表
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
      return;
    }

    file.close();  //?????????????

    // 连接
    m_sql->ConnectSql(path);

    // 创建表
    QString sqlBuf =
        "create table t_upload (timestamp int,f_id int,f_name "
        "varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 "
        "varchar(60),f_type varchar(60),f_absolutePath varchar(260));";
    m_sql->UpdateSql(sqlBuf);

    sqlBuf =
        "create table t_download (timestamp int,f_id int,f_name "
        "varchar(260),f_dir varchar(260),f_time varchar(60),f_size int,f_md5 "
        "varchar(60),f_type varchar(60),f_absolutePath varchar(260));";
    m_sql->UpdateSql(sqlBuf);
  }
}
// 1.缓存上传的任务
void CKernel::slot_writeUploadTask(FileInfo &info) {
  qDebug() << __func__;
  QString sqlBuf = QString(
                       "insert into t_upload values "
                       "(%1,%2,'%3','%4','%5',%6,'%7','%8','%9');")
                       .arg(info.timeStamp)
                       .arg(info.fileid)
                       .arg(info.name)
                       .arg(info.dir)
                       .arg(info.time)
                       .arg(info.size)
                       .arg(info.md5)
                       .arg(info.type)
                       .arg(info.absolutePath);

  m_sql->UpdateSql(sqlBuf);
}
// 2.缓存下载的任务
void CKernel::slot_writeDownloadTask(FileInfo &info) {
  qDebug() << __func__;

  QString sqlBuf = QString(
                       "insert into t_download values "
                       "(%1,%2,'%3','%4','%5',%6,'%7','%8','%9');")
                       .arg(info.timeStamp)
                       .arg(info.fileid)
                       .arg(info.name)
                       .arg(info.dir)
                       .arg(info.time)
                       .arg(info.size)
                       .arg(info.md5)
                       .arg(info.type)
                       .arg(info.absolutePath);
  //  这个地方 不能用警告里说的mutil arg 不然会出问题 就得写这么多个arg
  // QString a = QString( "%1/%2" ).arg( 1 , 2 );
  // 这个不知道为什么没报警告
  // a.remove( ' ' );

  m_sql->UpdateSql(sqlBuf);
}
// 3.完成任务，删除上传记录
void CKernel::slot_deleteUploadTask(FileInfo &info) {
  qDebug() << __func__;

  QString sqlBuf =
      QString("delete from t_upload where timestamp = %1;").arg(info.timeStamp);
  m_sql->UpdateSql(sqlBuf);
}
// 4.完成任务，删除上传记录
void CKernel::slot_deleteDownloadTask(FileInfo &info) {
  qDebug() << __func__;

  QString sqlBuf = QString("delete from t_download where timestamp = %1;")
                       .arg(info.timeStamp);
  // 你刚才是   我服啦
  m_sql->UpdateSql(sqlBuf);
}
// 5.加载上传任务
void CKernel::slot_getUploadTask(QList<FileInfo> &infoLst) {
  qDebug() << __func__;

  QString sqlBuf = "select * from t_upload;";

  QStringList lst;
  m_sql->SelectSql(sqlBuf, 9, lst);

  while (lst.size() != 0) {
    FileInfo info;
    info.timeStamp = QString(lst.front()).toInt();
    lst.pop_front();

    info.fileid = QString(lst.front()).toInt();
    lst.pop_front();

    info.name = lst.front();
    lst.pop_front();

    info.dir = lst.front();
    lst.pop_front();

    info.time = lst.front();
    lst.pop_front();

    info.size = QString(lst.front()).toInt();
    lst.pop_front();

    info.md5 = lst.front();
    lst.pop_front();

    info.type = lst.front();
    lst.pop_front();

    info.absolutePath = lst.front();
    lst.pop_front();

    infoLst.push_back(info);
  }
}
// 6.加载下载任务
void CKernel::slot_getDownloadTask(QList<FileInfo> &infoLst) {
  qDebug() << __func__;
  QString sqlBuf = "select * from t_download;";

  QStringList lst;
  m_sql->SelectSql(sqlBuf, 9, lst);

  while (lst.size() != 0) {
    FileInfo info;
    info.timeStamp = QString(lst.front()).toInt();
    lst.pop_front();

    info.fileid = QString(lst.front()).toInt();
    lst.pop_front();

    info.name = lst.front();
    lst.pop_front();

    info.dir = lst.front();
    lst.pop_front();

    info.time = lst.front();
    lst.pop_front();

    info.size = QString(lst.front()).toInt();
    lst.pop_front();

    info.md5 = lst.front();
    lst.pop_front();

    info.type = lst.front();
    lst.pop_front();

    info.absolutePath = lst.front();
    lst.pop_front();

    infoLst.push_back(info);
  }
}
