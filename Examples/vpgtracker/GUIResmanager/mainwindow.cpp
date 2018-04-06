#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDialog>
#include <QBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(QString("%1 v.%2").arg(APP_NAME,APP_VERSION));

    __loadSessionSettings();
    __updateParticipantsList();
    connect(&proc, SIGNAL(readyRead()), this, SLOT(readProcess()));
    connect(&proc, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(readError(QProcess::ProcessError)));
}

MainWindow::~MainWindow()
{
    if(proc.state() == QProcess::Running)
        proc.kill();

    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    QDialog *aboutdialog = new QDialog();
    aboutdialog->setWindowTitle(APP_NAME);

    int pS = aboutdialog->font().pointSize();
    aboutdialog->resize(35*pS, 15*pS);

    QVBoxLayout *templayout = new QVBoxLayout();

    QLabel *projectname = new QLabel(QString("%1 %2").arg(APP_NAME,APP_VERSION));
    projectname->setFrameStyle(QFrame::Box | QFrame::Raised);
    projectname->setAlignment(Qt::AlignCenter);
    QLabel *projectauthors = new QLabel(QString("Designed by: %1").arg(APP_DESIGNER));
    projectauthors->setWordWrap(true);
    projectauthors->setAlignment(Qt::AlignCenter);
    QLabel *hyperlink = new QLabel(QString("Support link: <a href='mailto:pi-null-mezon@yandex.ru?subject=%2'>%1</a>").arg(QString(APP_EMAIL),QString(APP_NAME)));
    hyperlink->setOpenExternalLinks(true);
    hyperlink->setAlignment(Qt::AlignCenter);

    templayout->addWidget(projectname);
    templayout->addWidget(projectauthors);
    templayout->addWidget(hyperlink);

    aboutdialog->setLayout(templayout);
    aboutdialog->exec();

    delete hyperlink;
    delete projectauthors;
    delete projectname;
    delete templayout;
    delete aboutdialog;
}

void MainWindow::on_researchdirB_clicked()
{
    QString _dirname = QFileDialog::getExistingDirectory(this, APP_NAME, "");
    if(!_dirname.isEmpty()) {
        ui->researchesdirLE->setText(_dirname);       
        __updateParticipantsList();
        ui->textEdit->clear();
    }
}

void MainWindow::closeEvent(QCloseEvent *_event)
{
    __saveSessionSetting();
    QMainWindow::closeEvent(_event);
}

void MainWindow::__loadSessionSettings()
{
    QSettings _settings(qApp->applicationDirPath().append("/%1.ini").arg(APP_NAME),QSettings::IniFormat);
    ui->researchesdirLE->setText(_settings.value("Storage/dirname",QString()).toString());
    procname = _settings.value("Process/name", "vpgtracker.exe").toString();
    procargs = _settings.value("Process/args", QStringList() << "--device=0" << "--facesize=256").toStringList();
}

void MainWindow::__saveSessionSetting()
{
    QSettings _settings(qApp->applicationDirPath().append("/%1.ini").arg(APP_NAME),QSettings::IniFormat);
    _settings.setValue("Storage/dirname",ui->researchesdirLE->text());
    _settings.setValue("Process/name", procname);
    _settings.setValue("Process/args", procargs);
}

void MainWindow::__updateParticipantsList()
{
    if(ui->researchesdirLE->text().isEmpty() == false) {
        QDir _dir(ui->researchesdirLE->text());
        QStringList _subdirname = _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        ui->participantidCB->clear();
        ui->participantidCB->addItems(_subdirname);
    }
}

void MainWindow::startArrowsProcess()
{
    QString _targetdirname = ui->researchesdirLE->text().append("/%1").arg(ui->participantidCB->currentText());
    QStringList _args;
    _args << QString("-i%1/Task.arrows").arg(qApp->applicationDirPath())
          << QString("-o%1/ar_%2.csv").arg(_targetdirname,_startdt.toString("ddMMyyyy_hhmmss"))
          << "-m";
    if(QProcess::startDetached(qApp->applicationDirPath().append("/Arrows.exe"),_args) == false) {
        ui->textEdit->append(tr("Не удалось запустить процесс Arrows.exe"));
    }
}

void MainWindow::on_participantidCB_activated(const QString &arg1)
{
    QString _targetdirname = ui->researchesdirLE->text().append("/%1").arg(arg1);
    QDir _dir(_targetdirname);
    if(_dir.exists() == false) {
        if(_dir.mkpath(_targetdirname))
            ui->textEdit->setText(tr("Создан новый испытуемый: %1").arg(arg1));
    }
}

void MainWindow::on_startB_clicked()
{
    if((ui->researchesdirLE->text().isEmpty() == false) && (ui->participantidCB->currentText().isEmpty() == false)) {
        QString _targetdirname = ui->researchesdirLE->text().append("/%1").arg(ui->participantidCB->currentText());
        QDir _dir(_targetdirname);
        if(_dir.exists() == false) {
            ui->textEdit->setText(tr("Не могу найти испытуемого с идентификатором: %1").arg(ui->participantidCB->currentText()));
            return;
        }

        if(proc.state() == QProcess::NotRunning) {
            // VPG process
            QStringList _args = procargs;
            _startdt = QDateTime::currentDateTime();
            _args << QString("--outputfilename=%1/%2.csv").arg(_targetdirname,_startdt.toString("ddMMyyyy_hhmmss"));
            proc.setProgram(qApp->applicationDirPath().append("/%1").arg(procname));
            proc.setArguments(_args);           

            ui->textEdit->clear();
            proc.start();
            QTimer::singleShot(5000,this,SLOT(startArrowsProcess()));
        } else {
            ui->textEdit->setText(tr("Процесс уже запущен!"));
        }

    } else {
        ui->textEdit->setText(tr("Не определён путь для сохранения измерений! Заполните поля расположенные выше!"));
    }
}

void MainWindow::readProcess()
{
    ui->textEdit->append(QString::fromLocal8Bit(proc.readAll()));
}

void MainWindow::readError(QProcess::ProcessError _error)
{
    Q_UNUSED(_error);
    //ui->textEdit->setText(tr("Произошла ошибка при запуске процесса! Код ошибки: %1").arg(QString::number(_error)));
}

void MainWindow::on_stopB_clicked()
{
    if(proc.state() == QProcess::Running)
        proc.kill();    
}
