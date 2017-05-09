#ifndef __WOLFCC_ATTRIB_H__
#define __WOLFCC_ATTRIB_H__

#include <sys/types.h>
#include <vector>
#include <string>

struct ServerAttri
{
    pid_t pid;          /* Main process pid. */

    //net
    int tcp_backlog;    /* TCP listen() backlog */
    std::vector<std::string> bindaddr; /* Addresses we should bind to */
    int bindaddr_count;         /* Number of addresses in server.bindaddr[] */
    std::vector<int> port;  /* TCP listening port */
    std::vector<int32_t> ipfd;  /* TCP socket file descriptors */
    int ipfd_count;             /* Used slots in ipfd[] */

    std::string unixsocket;     /* UNIX socket path */
    mode_t unixsocketperm;      /* UNIX socket permission */
    int sofd;                   /* Unix socket file descriptor */
    std::vector<int32_t> cfd;   /* Cluster bus listening socket */
    int cfd_count;              /* Used slots in cfd[] */
    std::string pidfile;        /* PID file path */
    int shutdown_asap;          /* SHUTDOWN needed ASAP */

    //common
    int32_t log_level;
    std::string log_file;
    bool daemonize;
    std::string version;
};

struct ClientAttri
{
    int32_t fd;
};

static ServerAttri g_wolfserver;

#endif
