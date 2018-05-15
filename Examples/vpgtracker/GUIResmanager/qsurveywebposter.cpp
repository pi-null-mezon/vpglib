#include "qsurveywebposter.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>

QSurveyWebposter::QSurveyWebposter(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt, QObject *_parent) : QThread(_parent),
    url(_url),
    afilename(_afilename),
    mfilename(_mfilename),
    label(_label),
    dt(_dt)
{
}

void QSurveyWebposter::run()
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart labelPart;
    labelPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"label\""));
    labelPart.setBody(label.toUtf8());

    QHttpPart dtPart;
    dtPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"datetime\""));
    dtPart.setBody(dt.toString("ddMMyyyy_hhmmss").toUtf8());

    QHttpPart afilePart;
    afilePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/csv"));
    afilePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"afile\"; filename=\"afile.csv\""));
    QFile afile(afilename);
    if(afile.open(QIODevice::ReadOnly) == false) {
        qWarning("[QSurveyWebposter] Can not open %s!", afilename.toUtf8().constData());
        return;
    }
    afilePart.setBody(afile.readAll());

    QHttpPart mfilePart;
    mfilePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/csv"));
    mfilePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"mfile\"; filename=\"mfile.csv\""));
    QFile mfile(mfilename);
    if(mfile.open(QIODevice::ReadOnly) == false) {
        qWarning("[QSurveyWebposter] Can not open %s!", mfilename.toUtf8().constData());
        return;
    }
    mfilePart.setBody(mfile.readAll());

    multiPart->append(labelPart);
    multiPart->append(dtPart);
    multiPart->append(afilePart);
    multiPart->append(mfilePart);

    qInfo("[QSurveyWebposter] Request url: %s", url.toUtf8().constData());
    QNetworkRequest request(QUrl::fromUserInput(url));

    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.post(request, multiPart);
    multiPart->setParent(reply);
    connect(reply,SIGNAL(finished()),this,SLOT(quit()));
    exec();

    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(status_code.isValid()) {
        // Print or catch the status code
        qInfo("[QSurveyWebposter] Reply code: %s", status_code.toString().toUtf8().constData());
    }
    qInfo("[QSurveyWebposter] Error string: %s", reply->errorString().toUtf8().constData());
    qInfo("[QSurveyWebposter] Server reply: %s", reply->readAll().constData());

    reply->deleteLater();
}

void postSurvey(const QString &_url, const QString &_afilename, const QString &_mfilename, const QString &_label, const QDateTime &_dt)
{
    QSurveyWebposter *_wThread = new QSurveyWebposter(_url,_afilename,_mfilename,_label,_dt);
    QObject::connect(_wThread,SIGNAL(finished()), _wThread, SLOT(deleteLater()));
    _wThread->start();
}
