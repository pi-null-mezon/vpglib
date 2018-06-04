#include "vpgservice.h"

VPGService::VPGService(int argc, char **argv) : QtService<QGuiApplication>(argc,argv,APP_NAME)
{
    setServiceDescription(QLatin1String("Runs VPG server on localhost with the websocket interface"));
    setServiceFlags(QtServiceBase::CanBeSuspended);
}

VPGService::~VPGService()
{
    delete daemon;
}

void VPGService::start()
{
    QGuiApplication *app = application();
    daemon = new QVPGServer(2308,app);

    if(daemon->isListening() == false) {
        QString _msg = QString("Failed to bind to port %1").arg(daemon->serverPort());
        qDebug("%s! Abort...", _msg.toLocal8Bit().constData());
        logMessage(_msg, QtServiceBase::Error);
        app->quit();
    }

    qDebug("%s started", this->serviceName().toLocal8Bit().constData());
}

void VPGService::pause()
{
    qDebug("%s paused", this->serviceName().toLocal8Bit().constData());
}

void VPGService::resume()
{
    qDebug("%s resumed", this->serviceName().toLocal8Bit().constData());
}

void VPGService::stop()
{
    qDebug("%s stoped", this->serviceName().toLocal8Bit().constData());
}
