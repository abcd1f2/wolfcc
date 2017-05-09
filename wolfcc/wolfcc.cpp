#include <stdio.h>
#include "servermanager.h"

int main(int argc, char **argv)
{
    ServerManager serverm;
    if (!serverm.Init()) {
        serverm.Stop();
    }

    serverm.Start();
    return 0;
}
