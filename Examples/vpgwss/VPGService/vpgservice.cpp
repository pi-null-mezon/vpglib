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
    auto argslist = app->arguments();
    quint16 port = 2308;
    for(int i = 0; i < argslist.size(); ++i) {
        if(argslist.at(i).contains("-p")) {
            port = argslist.at(i).section("-p",1,1).toUShort();
            break;
        }
    }

    daemon = new QVPGServer(port,app);

    if(daemon->isListening() == false) {
        QString _msg = QString("Failed to bind to port %1").arg(port);
        qInfo("%s! Abort...", _msg.toLocal8Bit().constData());
        logMessage(_msg, QtServiceBase::Error);
        app->quit();
    } else {
        qInfo("%s listen incoming connections on ws://localhost:%d", this->serviceName().toLocal8Bit().constData(), daemon->serverPort());
    }
}

void VPGService::pause()
{
    qInfo("%s paused", this->serviceName().toLocal8Bit().constData());
}

void VPGService::resume()
{
    qInfo("%s resumed", this->serviceName().toLocal8Bit().constData());
}

void VPGService::stop()
{
    qInfo("%s stoped", this->serviceName().toLocal8Bit().constData());
}
