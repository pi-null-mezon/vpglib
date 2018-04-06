#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTextStream>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QKeyEvent>
#include <QFile>

#include "qarrowwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void parseTaskfile(const QString &_filename);
    void setOutputFilename(const QString &_filename);

protected:
    void keyPressEvent(QKeyEvent *_event);

private:
    void __startSession();
    void __enrollKeyPress(const ArrowProps::ArrowDirection &_arrowdirection);

    Ui::MainWindow *ui;

    QFile outputfile;
    QTextStream ostream;
    QElapsedTimer elapsedtimer;
    QVector<ArrowProps> varrowprops;
    int position;
    bool sessionstarted;
};

#endif // MAINWINDOW_H
