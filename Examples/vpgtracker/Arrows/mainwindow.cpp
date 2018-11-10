#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sessionstarted(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::CustomizeWindowHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::parseTaskfile(const QString &_filename)
{
    varrowprops.clear();

    QFile _ifile(_filename);
    if(_ifile.open(QFile::ReadOnly) == false) {
        qCritical("Can not open %s file for reading!",_filename.toUtf8().constData());
        return;
    }

    QByteArray _line;
    do {
        _line = _ifile.readLine();
    } while(_line.at(0) == '*'); // skip comments
    ArrowProps _arrow;
    while(!_line.isEmpty()) {
        QString _strline(_line);
        _arrow.backgroundcolor = QColor(_strline.section(',',0,0).simplified());
        _arrow.arrowcolor = QColor(_strline.section(',',1,1).simplified());
        _arrow.arrowdirection = ArrowProps::str2direction(_strline.section(',',2,2).simplified());
        _arrow.arrowposition = ArrowProps::str2position(_strline.section(',',3,3).simplified());
        varrowprops.push_back(qMove(_arrow));
        _line = _ifile.readLine();
    }
}

void MainWindow::setOutputFilename(const QString &_filename)
{
    outputfile.setFileName(_filename);
    if(outputfile.open(QFile::WriteOnly) == false) {
        qCritical("Can not open output file to write data!");
        return;
    }
    ostream.setDevice(&outputfile);
    ostream << "This file has been created by " << APP_NAME  << " v." << APP_VERSION;
}

void MainWindow::keyPressEvent(QKeyEvent *_event)
{
    switch (_event->key()) {
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Space:
            if(sessionstarted == false)
                __startSession();
            break;
        case Qt::Key_Up:
            __enrollKeyPress(ArrowProps::Up);
            break;
        case Qt::Key_Down:
            __enrollKeyPress(ArrowProps::Down);
            break;
        case Qt::Key_Left:
            __enrollKeyPress(ArrowProps::Left);
            break;
        case Qt::Key_Right:
            __enrollKeyPress(ArrowProps::Right);
            break;
        case Qt::Key_F11:
            if(isMaximized())
                showNormal();
            else
                showMaximized();
            break;
    }

    QMainWindow::keyPressEvent(_event);
}

void MainWindow::__startSession()
{
    if((varrowprops.size() > 0) && outputfile.isOpen()) {
        elapsedtimer.start();
        position = 0;
        ui->centralWidget->updateArrow(varrowprops[position]);
        ostream << "\nSession has been started at " << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
        ostream << "\nSinceStart[ms],\tTaskKey,\tUserKey";
        sessionstarted = true;
    }
}

void MainWindow::__enrollKeyPress(const ArrowProps::ArrowDirection &_arrowdirection)
{
    if(sessionstarted == true) {

        ostream << "\n"  << elapsedtimer.elapsed()
                << ",\t" << ArrowProps::direction2str(varrowprops[position].arrowdirection)
                << ",\t" << ArrowProps::direction2str(_arrowdirection);

        position++;

        if(position < varrowprops.size()) {
            ui->centralWidget->updateArrow(varrowprops[position]);
        } else {
            close();
        }
    }
}
