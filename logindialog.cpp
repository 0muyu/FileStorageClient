#include "logindialog.h"

#include <QMessageBox>
#include <QRegExp>

#include "ui_logindialog.h"
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
  ui->setupUi( this );

  setWindowTitle( "登录&注册" );

  //  ui->tw_page->setCurrentIndex( 1 );  //设置默认为登录界面
}

LoginDialog::~LoginDialog()
{
  delete ui;
}

// 1.登录清除
void LoginDialog::on_pb_clear_clicked() {
  ui->le_tel->setText( "" );
  ui->le_pass->setText( "" );
}
// 2.登录提交
void LoginDialog::on_pb_submit_clicked() {
  // 1.获取注册信息
  QString tel = ui->le_tel->text();
  QString pwd = ui->le_pass->text();

  QString telCpy = tel;
  QString pwdCpy = pwd;

  // 2.校验
  telCpy.remove( " " );
  pwdCpy.remove( " " );
  // 2.1 校验输入是否为空或全空格
  if ( telCpy.isEmpty() || pwdCpy.isEmpty() ) {
    QMessageBox::about( this, "提示", "输入不能为空或全空格！" );
    return;
  }

  // 2.2校验手机号 是否合法
  QRegExp exp = QRegExp( "^1[3456789][0-9]\{9\}$" );
  bool    res = exp.exactMatch( tel );
  if ( !res ) {
    QMessageBox::about( this, "提示", "手机号非法!" );
    return;
  }

  // 2.3校验密码是否过长以及两次输入的密码是否匹配
  if ( pwd.size() > 20 ) {
    QMessageBox::about( this, "提示", "密码过长，长度应<=20" );
    return;
  }

  // 3.将信号发送给核心类进行处理
  Q_EMIT sig_loginMessage( tel, pwd );
}

// 3.注册清除
void LoginDialog::on_pb_clear_register_clicked() {
  ui->le_name_register->setText( "" );
  ui->le_pass_register->setText( "" );
  ui->le_repass_register->setText( "" );
  ui->le_tel_register->setText( "" );
}
// 4.注册提交
void LoginDialog::on_pb_submit_register_clicked() {
  // 1.获取注册信息
  QString tel   = ui->le_tel_register->text();
  QString pwd   = ui->le_pass_register->text();
  QString repwd = ui->le_repass_register->text();
  QString name  = ui->le_name_register->text();

  QString telCpy  = tel;
  QString pwdCpy  = pwd;
  QString nameCpy = name;
  // 2.校验
  telCpy.remove( " " );
  pwdCpy.remove( " " );
  nameCpy.remove( " " );
  // 2.1 校验输入是否为空或全空格
  if ( telCpy.isEmpty() || pwdCpy.isEmpty() || nameCpy.isEmpty() ) {
    QMessageBox::about( this, "提示", "输入不能为空或全空格！" );
    return;
  }

  // 2.2校验手机号 是否合法
  QRegExp exp = QRegExp( "^1[3456789][0-9]\{9\}$" );
  bool    res = exp.exactMatch( tel );
  if ( !res ) {
    QMessageBox::about( this, "提示", "手机号非法!" );
    return;
  }

  // 2.3校验密码是否过长以及两次输入的密码是否匹配
  if ( pwd.size() > 20 ) {
    QMessageBox::about( this, "提示", "密码过长，长度应<=20" );
    return;
  }

  if ( pwd != repwd ) {
    QMessageBox::about( this, "提示", "两次输入密码不一致！" );
    return;
  }
  // 2.4校验昵称是否过长或含敏感词汇
  if ( name.size() > 10 ) {
    QMessageBox::about( this, "提示", "昵称过长,长度应<=10" );
    return;
  }

  // 3.将信号发送给核心类进行处理
  Q_EMIT sig_registerMessage( tel, pwd, name );
}
