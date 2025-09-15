#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
  Q_OBJECT

 public:
  explicit LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

 signals:
  // 1.登录信息
  void sig_loginMessage( QString tel, QString pwd );
  // 2.注册信息
  void sig_registerMessage( QString tel, QString pwd, QString name );

 private slots:

  // 1.登录清除
  void on_pb_clear_clicked();
  // 2.登录提交
  void on_pb_submit_clicked();
  // 3.注册清除
  void on_pb_clear_register_clicked();
  // 4.注册提交
  void on_pb_submit_register_clicked();

 private:
  Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
