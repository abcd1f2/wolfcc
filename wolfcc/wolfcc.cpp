#include <stdio.h>
#include "serverapp.h"
#include "utils/logging.h"
#include "utils/daemonize.h"
#include "serverapp.h"

static const char *g_config_name = "./wolfcc.conf"

int main(int argc, char **argv)
{
#if 0
    Daemon::daemonize();
#endif

	std::string config_name(g_config_name);
	if (argc >= 2) {
		config_name = argv[1];
	}
	log(LOG_INFO, "start server with config %s", config_name.c_str());

	ServerApp serverapp(config_name);
    if (!serverapp.Init()) {
        serverapp.Stop();
    }

    log(LOG_INFO, "start server");
    serverapp.Start();
    serverapp.Stop();
    return 0;
}
