#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // 高分屏支持
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

    MainWindow w;
    w.resize(1024, 600);
    w.setWindowTitle("嵌入式电刺激控制台 (Widgets版)");
    w.show();

    return a.exec();
}
