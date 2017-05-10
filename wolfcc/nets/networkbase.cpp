#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "nets/networkbase.h"
#include "utils/logging.h"

int NetWork::anetSetBlock(int fd, int non_block)
{
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        log(LOG_ERR, "fcntl(F_GETFL): %s", strerror(errno));
        return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        log(LOG_ERR, "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetNonBlock(int fd)
{
    return anetSetBlock(err, fd, 1);
}

int NetWork::anetBlock(int fd)
{
    return anetSetBlock(err, fd, 0);
}

int NetWork::anetKeepAlive(int fd, int interval)
{
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
        log(LOG_ERR, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return ANET_ERR;
    }

    /* Default settings are more or less garbage, with the keepalive time
    * set to 7200 by default on Linux. Modify settings to make the feature
    * actually useful. */

    /* Send first probe after interval. */
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
        log(LOG_ERR, "setsockopt TCP_KEEPIDLE: %s\n", strerror(errno));
        return ANET_ERR;
    }

    /* Send next probes after the specified interval. Note that we set the
    * delay as interval / 3, as we send three probes before detecting
    * an error (see the next setsockopt call). */
    val = interval / 3;
    if (val == 0) val = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
        log(LOG_ERR, "setsockopt TCP_KEEPINTVL: %s\n", strerror(errno));
        return ANET_ERR;
    }

    /* Consider the socket in error state after three we send three ACK
    * probes without getting a reply. */
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
        log(LOG_ERR, "setsockopt TCP_KEEPCNT: %s\n", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetSetTcpNoDelay(int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
        log(LOG_ERR, "setsockopt TCP_NODELAY: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetEnableTcpNoDelay(int fd)
{
    return anetSetTcpNoDelay(err, fd, 1);
}

int NetWork::anetDisableTcpNoDelay(int fd)
{
    return anetSetTcpNoDelay(err, fd, 0);
}


int NetWork::anetSetSendBuffer(int fd, int buffsize)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1)
    {
        log(LOG_ERR, "setsockopt SO_SNDBUF: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetTcpKeepAlive(int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
        log(LOG_ERR, "setsockopt SO_KEEPALIVE: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
* number of milliseconds, or disable it if the 'ms' argument is zero. */
int NetWork::anetSendTimeout(int fd, long long ms)
{
    struct timeval tv;

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
        log(LOG_ERR, "setsockopt SO_SNDTIMEO: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

/* anetGenericResolve() is called by anetResolve() and anetResolveIP() to
* do the actual work. It resolves the hostname "host" and set the string
* representation of the IP address into the buffer pointed by "ipbuf".
*
* If flags is set to ANET_IP_ONLY the function only resolves hostnames
* that are actually already IPv4 or IPv6 addresses. This turns the function
* into a validating / normalizing function. */
int NetWork::anetGenericResolve(char *host, char *ipbuf, size_t ipbuf_len, int flags)
{
    struct addrinfo hints, *info;
    int rv;

    memset(&hints, 0, sizeof(hints));
    if (flags & ANET_IP_ONLY) hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0) {
        log(LOG_ERR, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    }
    else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }

    freeaddrinfo(info);
    return ANET_OK;
}

int NetWork::anetResolve(char *host, char *ipbuf, size_t ipbuf_len)
{
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_NONE);
}

int NetWork::anetResolveIP(char *host, char *ipbuf, size_t ipbuf_len)
{
    return anetGenericResolve(err, host, ipbuf, ipbuf_len, ANET_IP_ONLY);
}

int NetWork::anetSetReuseAddr(int fd)
{
    int yes = 1;
    /* Make sure connection-intensive things like the redis benckmark
    * will be able to close/open sockets a zillion of times */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        log(LOG_ERR, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetCreateSocket(int domain)
{
    int s;
    if ((s = socket(domain, SOCK_STREAM, 0)) == -1) {
        log(LOG_ERR, "creating socket: %s", strerror(errno));
        return ANET_ERR;
    }

    /* Make sure connection-intensive things like the redis benchmark
    * will be able to close/open sockets a zillion of times */
    if (anetSetReuseAddr(err, s) == ANET_ERR) {
        close(s);
        return ANET_ERR;
    }
    return s;
}

int NetWork::anetTcpGenericConnect(char *addr, int port, char *source_addr, int flags)
{
    int s = ANET_ERR, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0) {
        log(LOG_ERR, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        /* Try to create the socket and to connect it.
        * If we fail in the socket() call, or on connect(), we retry with
        * the next entry in servinfo. */
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;
        if (anetSetReuseAddr(err, s) == ANET_ERR) goto error;
        if (flags & ANET_CONNECT_NONBLOCK && anetNonBlock(err, s) != ANET_OK)
            goto error;
        if (source_addr) {
            int bound = 0;
            /* Using getaddrinfo saves us from self-determining IPv4 vs IPv6 */
            if ((rv = getaddrinfo(source_addr, NULL, &hints, &bservinfo)) != 0)
            {
                log(LOG_ERR, "%s", gai_strerror(rv));
                goto error;
            }
            for (b = bservinfo; b != NULL; b = b->ai_next) {
                if (bind(s, b->ai_addr, b->ai_addrlen) != -1) {
                    bound = 1;
                    break;
                }
            }
            freeaddrinfo(bservinfo);
            if (!bound) {
                log(LOG_ERR, "bind: %s", strerror(errno));
                goto error;
            }
        }
        if (connect(s, p->ai_addr, p->ai_addrlen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
            * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS && flags & ANET_CONNECT_NONBLOCK)
                goto end;
            close(s);
            s = ANET_ERR;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
        * have a connected socket. Let's return to the caller. */
        goto end;
    }
    if (p == NULL)
        log(LOG_ERR, "creating socket: %s", strerror(errno));

error:
    if (s != ANET_ERR) {
        close(s);
        s = ANET_ERR;
    }

end:
    freeaddrinfo(servinfo);

    /* Handle best effort binding: if a binding address was used, but it is
    * not possible to create a socket, try again without a binding address. */
    if (s == ANET_ERR && source_addr && (flags & ANET_CONNECT_BE_BINDING)) {
        return anetTcpGenericConnect(err, addr, port, NULL, flags);
    }
    else {
        return s;
    }
}

int NetWork::anetTcpConnect(char *addr, int port)
{
    return anetTcpGenericConnect(err, addr, port, NULL, ANET_CONNECT_NONE);
}

int NetWork::anetTcpNonBlockConnect(char *addr, int port)
{
    return anetTcpGenericConnect(err, addr, port, NULL, ANET_CONNECT_NONBLOCK);
}

int NetWork::anetTcpNonBlockBindConnect(char *addr, int port, char *source_addr)
{
    return anetTcpGenericConnect(err, addr, port, source_addr,
        ANET_CONNECT_NONBLOCK);
}

int NetWork::anetTcpNonBlockBestEffortBindConnect(char *addr, int port, char *source_addr)
{
    return anetTcpGenericConnect(err, addr, port, source_addr,
        ANET_CONNECT_NONBLOCK | ANET_CONNECT_BE_BINDING);
}

int NetWork::anetUnixGenericConnect(char *path, int flags)
{
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR)
        return ANET_ERR;

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (flags & ANET_CONNECT_NONBLOCK) {
        if (anetNonBlock(err, s) != ANET_OK)
            return ANET_ERR;
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS &&
            flags & ANET_CONNECT_NONBLOCK)
            return s;

        log(LOG_ERR, "connect: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return s;
}

int NetWork::anetUnixConnect(char *path)
{
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONE);
}

int NetWork::anetUnixNonBlockConnect(char *path)
{
    return anetUnixGenericConnect(err, path, ANET_CONNECT_NONBLOCK);
}

/* Like read(2) but make sure 'count' is read before to return
* (unless error or EOF condition is encountered) */
int NetWork::anetRead(int fd, char *buf, int count)
{
    ssize_t nread, totlen = 0;
    while (totlen != count) {
        nread = read(fd, buf, count - totlen);
        if (nread == 0) return totlen;
        if (nread == -1) return -1;
        totlen += nread;
        buf += nread;
    }
    return totlen;
}

/* Like write(2) but make sure 'count' is written before to return
* (unless error is encountered) */
int NetWork::anetWrite(int fd, char *buf, int count)
{
    ssize_t nwritten, totlen = 0;
    while (totlen != count) {
        nwritten = write(fd, buf, count - totlen);
        if (nwritten == 0) return totlen;
        if (nwritten == -1) return -1;
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

int NetWork::anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog)
{
    if (bind(s, sa, len) == -1) {
        log(LOG_ERR, "bind: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    if (listen(s, backlog) == -1) {
        log(LOG_ERR, "listen: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::anetV6Only(int s) {
    int yes = 1;
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == -1) {
        log(LOG_ERR, "setsockopt: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}

int NetWork::_anetTcpServer(int port, char *bindaddr, int af, int backlog)
{
    int s, rv;
    char _port[6];  /* strlen("65535") */
    struct addrinfo hints, *servinfo, *p;

    snprintf(_port, 6, "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* No effect if bindaddr != NULL */

    if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0) {
        log(LOG_ERR, "%s", gai_strerror(rv));
        return ANET_ERR;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (af == AF_INET6 && anetV6Only(err, s) == ANET_ERR) goto error;
        if (anetSetReuseAddr(err, s) == ANET_ERR) goto error;
        if (anetListen(err, s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) goto error;
        goto end;
    }
    if (p == NULL) {
        log(LOG_ERR, "unable to bind socket, errno: %d", errno);
        goto error;
    }

error:
    s = ANET_ERR;
end:
    freeaddrinfo(servinfo);
    return s;
}

int NetWork::anetTcpServer(int port, char *bindaddr, int backlog)
{
    return _anetTcpServer(err, port, bindaddr, AF_INET, backlog);
}

int NetWork::anetTcp6Server(int port, char *bindaddr, int backlog)
{
    return _anetTcpServer(err, port, bindaddr, AF_INET6, backlog);
}

int NetWork::anetUnixServer(char *path, mode_t perm, int backlog)
{
    int s;
    struct sockaddr_un sa;

    if ((s = anetCreateSocket(err, AF_LOCAL)) == ANET_ERR)
        return ANET_ERR;

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path, path, sizeof(sa.sun_path) - 1);
    if (anetListen(err, s, (struct sockaddr*)&sa, sizeof(sa), backlog) == ANET_ERR)
        return ANET_ERR;
    if (perm)
        chmod(sa.sun_path, perm);
    return s;
}

int NetWork::anetGenericAccept(int s, struct sockaddr *sa, socklen_t *len)
{
    int fd;
    while (1) {
        fd = accept(s, sa, len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                log(LOG_ERR, "accept: %s", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}

int NetWork::anetTcpAccept(int s, char *ip, size_t ip_len, int *port)
{
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err, s, (struct sockaddr*)&sa, &salen)) == -1)
        return ANET_ERR;

    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip) inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    }
    else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    return fd;
}

int NetWork::anetUnixAccept(int s)
{
    int fd;
    struct sockaddr_un sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err, s, (struct sockaddr*)&sa, &salen)) == -1)
        return ANET_ERR;

    return fd;
}

int NetWork::anetPeerToString(int fd, char *ip, size_t ip_len, int *port)
{
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if (getpeername(fd, (struct sockaddr*)&sa, &salen) == -1) goto error;
    if (ip_len == 0) goto error;

    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip) inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    }
    else if (sa.ss_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    else if (sa.ss_family == AF_UNIX) {
        if (ip) strncpy(ip, "/unixsocket", ip_len);
        if (port) *port = 0;
    }
    else {
        goto error;
    }
    return 0;

error:
    if (ip) {
        if (ip_len >= 2) {
            ip[0] = '?';
            ip[1] = '\0';
        }
        else if (ip_len == 1) {
            ip[0] = '\0';
        }
    }
    if (port) *port = 0;
    return -1;
}

int NetWork::anetFormatAddr(char *buf, size_t buf_len, char *ip, int port)
{
    return snprintf(buf, buf_len, strchr(ip, ':') ? "[%s]:%d" : "%s:%d", ip, port);
}

/* Like anetFormatAddr() but extract ip and port from the socket's peer. */
int NetWork::anetFormatPeer(int fd, char *buf, size_t buf_len)
{
    char ip[INET6_ADDRSTRLEN];
    int port;

    anetPeerToString(fd, ip, sizeof(ip), &port);
    return anetFormatAddr(buf, buf_len, ip, port);
}

int NetWork::anetSockName(int fd, char *ip, size_t ip_len, int *port)
{
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if (getsockname(fd, (struct sockaddr*)&sa, &salen) == -1) {
        if (port) *port = 0;
        ip[0] = '?';
        ip[1] = '\0';
        return -1;
    }
    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip) inet_ntop(AF_INET, (void*)&(s->sin_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin_port);
    }
    else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip) inet_ntop(AF_INET6, (void*)&(s->sin6_addr), ip, ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    return 0;
}

int NetWork::anetFormatSock(int fd, char *fmt, size_t fmt_len)
{
    char ip[INET6_ADDRSTRLEN];
    int port;

    anetSockName(fd, ip, sizeof(ip), &port);
    return anetFormatAddr(fmt, fmt_len, ip, port);
}

int NetWork::listenToPort(int port, int *fds, int *count)
{
    int j;

    /* Force binding of 0.0.0.0 if no bind address is specified, always
    * entering the loop if j == 0. */
    if (g_wolfserver.bindaddr_count == 0) g_wolfserver.bindaddr[0] = NULL;
    for (j = 0; j < g_wolfserver.bindaddr_count || j == 0; j++) {
        if (g_wolfserver.bindaddr[j].empty()) {
            int unsupported = 0;
            /* Bind * for both IPv6 and IPv4, we enter here only if
            * server.bindaddr_count == 0. */
            fds[*count] = anetTcp6Server(server.neterr, port, NULL, server.tcp_backlog);
            if (fds[*count] != ANET_ERR) {
                anetNonBlock(NULL, fds[*count]);
                (*count)++;
            }
            else if (errno == EAFNOSUPPORT) {
                unsupported++;
                serverLog(LL_WARNING, "Not listening to IPv6: unsupproted");
            }

            if (*count == 1 || unsupported) {
                /* Bind the IPv4 address as well. */
                fds[*count] = anetTcpServer(server.neterr, port, NULL,
                    server.tcp_backlog);
                if (fds[*count] != ANET_ERR) {
                    anetNonBlock(NULL, fds[*count]);
                    (*count)++;
                }
                else if (errno == EAFNOSUPPORT) {
                    unsupported++;
                    serverLog(LL_WARNING, "Not listening to IPv4: unsupproted");
                }
            }
            /* Exit the loop if we were able to bind * on IPv4 and IPv6,
            * otherwise fds[*count] will be ANET_ERR and we'll print an
            * error and return to the caller with an error. */
            if (*count + unsupported == 2) break;
        }
        else if (strchr(server.bindaddr[j], ':')) {
            /* Bind IPv6 address. */
            fds[*count] = anetTcp6Server(server.neterr, port, server.bindaddr[j],
                server.tcp_backlog);
        }
        else {
            /* Bind IPv4 address. */
            fds[*count] = anetTcpServer(server.neterr, port, server.bindaddr[j],
                server.tcp_backlog);
        }
        if (fds[*count] == ANET_ERR) {
            log(LOG_ERR,
                "Creating Server TCP listening socket %s:%d: %s",
                server.bindaddr[j] ? server.bindaddr[j] : "*",
                port, server.neterr);
            return C_ERR;
        }
        anetNonBlock(NULL, fds[*count]);
        (*count)++;
    }
    return C_OK;
}

bool NetWork::listUnixSocket()
{
    if (server.unixsocket != NULL) {
        unlink(server.unixsocket); /* don't care if this fails */
        server.sofd = anetUnixServer(server.unixsocket, server.unixsocketperm, server.tcp_backlog);
        if (server.sofd == ANET_ERR) {
            log(LOG_ERR, "Opening Unix socket");
            return false;
        }
        anetNonBlock(NULL, server.sofd);
    }

    return true;
}
