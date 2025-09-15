#include <QApplication>

#include "ckernel.h"
#undef main
int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  // MainDialog w;
  // w.show();
  CKernel* kernel = CKernel::getInstance();
  return a.exec();
}
