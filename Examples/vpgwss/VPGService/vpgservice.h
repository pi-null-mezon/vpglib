/*
 * Designed by means of this tutorial https://www.youtube.com/watch?v=qDH7zWYaScA
 * The main difference (from tutorial) is that I have used QGuiApplication instead
 * of QCoreApplication because instead QCamera could not find any video devices
 * Alex.A.Taranov
*/

#ifndef VPGSERVICE_H
#define VPGSERVICE_H

#include <QGuiApplication>
#include "qtservice.h"
#include "qvpgserver.h"

class VPGService : public QtService<QGuiApplication>
{
public:
    /**
     * @brief The constructor
     */
    VPGService(int argc, char **argv);

    /**
     * @brief The deconstructor
     */
    ~VPGService();

    /**
     * @brief start the service
     */
    void start();

    /**
     * @brief pause the service
     */
    void pause();

    /**
     * @brief resume the service
     */
    void resume();

    /**
     * @brief stop the service
     */
    void stop();

private:
    QVPGServer *daemon;
};

#endif // VPGSERVICE_H
