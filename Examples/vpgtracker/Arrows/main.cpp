#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    QString _inputfilename, _outputfilename;
    bool _showmaximized = false;
    while(--argc > 0 && (*++argv)[0] == '-') // Old school cmd args parsing
            switch(*++argv[0]) {
                case 'h':
                    qInfo(QString("%1 v.%2\n"
                                  "\n"
                                  "Options:\n"
                                  " -h - this help\n"
                                  " -i - input filename with the task\n"
                                  " -o - output filename with user reactions log\n"
                                  " -m - enable show maximized option").arg(APP_NAME,APP_VERSION).toUtf8().constData());
                    return 0;

                case 'i':
                    _inputfilename = QString::fromLocal8Bit(++argv[0]);
                    break;

                case 'o':
                    _outputfilename = QString::fromLocal8Bit(++argv[0]);
                    break;

                case 'm':
                    _showmaximized = true;
                    break;

            }

    if(_inputfilename.isEmpty()) {
        qWarning("Input file name is empty! Abort...");
        return 1;
    }
    if(_outputfilename.isEmpty()) {
        qWarning("Output file name is empty! Abort...");
        return 2;
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.parseTaskfile(_inputfilename);
    w.setOutputFilename(_outputfilename);
    if(_showmaximized)
        w.showMaximized();
    else
        w.show();

    return a.exec();
}
