#include "vpgservice.h"

int main(int argc, char *argv[])
{
    VPGService daemon(argc, argv);
    return daemon.exec();
}
