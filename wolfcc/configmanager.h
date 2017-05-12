#ifndef __WOLFCC_CONFIGMANAGER_H__
#define __WOLFCC_CONFIGMANAGER_H__
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include "nets/sockaddr.h"

class ServerConfig
{
public:
    ServerConfig(const std::string& path);
    virtual ~ServerConfig();

public:
    virtual bool LoadConfig();

public:
    pid_t pid;          /* Main process pid. */

    //net
    SockAddr bindaddr_;


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

    std::list<std::string> limitaddress_; /*limit address*/

    //common
    int32_t log_level;
    std::string log_file;
    bool daemonize;
    std::string version;

    /* Limits */
    uint32_t maxclients;   /* Max number of simultaneous clients */
    uint64_t maxmemory;   /* Max number of memory bytes to use */
};

#endif
