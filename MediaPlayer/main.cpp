#include <QApplication>
#include <iostream>

#include "playerdialog.h"

#undef main

int main( int argc, char *argv[] ) {
  QApplication a(argc, argv);
  PlayerDialog w;

  w.show();
  return a.exec();
}
