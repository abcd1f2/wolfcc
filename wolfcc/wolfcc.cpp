#include <stdio.h>
#include "serverapp.h"
#include "utils/logging.h"
#include "utils/daemonize.h"

int main(int argc, char **argv)
{
#if 0
    Daemon::daemonize();
#endif

    ServerApp serverapp;
    if (!serverapp.Init()) {
        serverapp.Stop();
    }

    log(LOG_INFO, "start server");
    serverapp.Start();
    serverapp.Stop();
    return 0;
}
