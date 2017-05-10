#include <stdio.h>
#include "servermanager.h"
#include "util/logging.h"

int main(int argc, char **argv)
{
    ServerManager serverm;
    if (!serverm.Init()) {
        serverm.Stop();
    }

    log(LOG_INFO, "start server");
    serverm.Start();
    serverm.Stop();
    return 0;
}
