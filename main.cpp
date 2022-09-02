#include "mainwindow.h"

#include <QApplication>
extern "C" {
    //设备相关api
    #include <libavdevice/avdevice.h>
}
#undef main
int main(int argc, char *argv[])
{

    //注册所有设备
    avdevice_register_all();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    _CrtDumpMemoryLeaks(); //B
    return a.exec();
}
