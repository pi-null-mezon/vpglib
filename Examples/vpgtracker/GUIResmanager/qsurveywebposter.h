#ifndef QSURVEYWEBPOSTER_H
#define QSURVEYWEBPOSTER_H

#include <QThread>
#include <QDateTime>

void postSurvey(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt);

class QSurveyWebposter : public QThread
{
    Q_OBJECT

public:
    QSurveyWebposter(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt, QObject *_parent=nullptr);

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
