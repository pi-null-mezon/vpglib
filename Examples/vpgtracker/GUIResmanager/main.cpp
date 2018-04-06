#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
