#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QString>
#include <QPixmap>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QPixmap img("/home/yqwu/cover1.jpg");
    w.show();
    w.add_label_button("test", "t");
    w.add_label_button("test", "q");
    w.reload_button();

    return a.exec();
}
