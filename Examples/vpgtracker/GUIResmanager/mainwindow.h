#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QProcess>
#include <QDateTime>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionAbout_triggered();
    void on_researchdirB_clicked();
    void on_participantidCB_activated(const QString &arg1);
    void on_startB_clicked();
    void readProcess();
    void readError(QProcess::ProcessError _error);
    void on_stopB_clicked();
    void startArrowsProcess();

protected:
    void closeEvent(QCloseEvent *_event);

private:
    void __loadSessionSettings();
    void __saveSessionSetting();
    void __updateParticipantsList();    

    QProcess proc;

    QString procname;
    QStringList procargs;
    QDateTime _startdt;

    QString websrvurl;
    QString vpgtrackeroutputfilename;
    QString arrowsoutputfilename;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
