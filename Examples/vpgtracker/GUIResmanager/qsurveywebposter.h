#ifndef QSURVEYWEBPOSTER_H
#define QSURVEYWEBPOSTER_H

#include <QWidget>
#include <QThread>
#include <QDateTime>

class QMessageHolder : public QObject
{
    Q_OBJECT
public:
    QMessageHolder(QObject *_parent=nullptr) : QObject(_parent) {}

signals:
    void msgUpdated(const QString &_msg);

public slots:
    void updateMessage(const QString &_msg)
    {
        emit msgUpdated(_msg);
        msg = _msg;
    }
    QString message() const
    {
        return msg;
    }

private:
    QString msg;
};


void postSurvey(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt, QMessageHolder *_msgholder=0);

class QSurveyWebposter : public QThread
{
    Q_OBJECT
public:
    QSurveyWebposter(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt, QObject *_parent=nullptr);

signals:
    void surveyPosted(const QString &_msg);

protected:
    void run() override;

private:
    QString     url;
    QString     afilename; // arrows
    QString     mfilename; // measurements
    QString     label;     // participant label
    QDateTime   dt;        // date time
};

#endif // QSURVEYWEBPOSTER_H
