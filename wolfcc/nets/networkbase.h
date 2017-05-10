#ifndef __WOLFCC_NETWORKBASE_H__
#define __WOLFCC_NETWORKBASE_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

class NetWork
{
public:
    int listenToPort(int port, int *fds, int *count);
    bool listUnixSocket();

    int anetTcpConnect(char *addr, int port);
    int anetTcpNonBlockConnect(char *addr, int port);
    int anetTcpNonBlockBindConnect(char *addr, int port, char *source_addr);
    int anetTcpNonBlockBestEffortBindConnect(char *addr, int port, char *source_addr);
    int anetUnixConnect(char *path);
    int anetUnixNonBlockConnect(char *path);
    int anetRead(int fd, char *buf, int count);
    int anetResolve(char *host, char *ipbuf, size_t ipbuf_len);
    int anetResolveIP(char *host, char *ipbuf, size_t ipbuf_len);
    int anetTcpServer(int port, char *bindaddr, int backlog);
    int anetTcp6Server(int port, char *bindaddr, int backlog);
    int anetUnixServer(char *path, mode_t perm, int backlog);
    int anetTcpAccept(int serversock, char *ip, size_t ip_len, int *port);
    int anetUnixAccept(int serversock);
    int anetWrite(int fd, char *buf, int count);
    int anetNonBlock(int fd);
    int anetBlock(int fd);
    int anetEnableTcpNoDelay(int fd);
    int anetDisableTcpNoDelay(int fd);
    int anetTcpKeepAlive(int fd);
    int anetSendTimeout(int fd, long long ms);
    int anetPeerToString(int fd, char *ip, size_t ip_len, int *port);
    int anetKeepAlive(int fd, int interval);
    int anetSockName(int fd, char *ip, size_t ip_len, int *port);
    int anetFormatAddr(char *fmt, size_t fmt_len, char *ip, int port);
    int anetFormatPeer(int fd, char *fmt, size_t fmt_len);
    int anetFormatSock(int fd, char *fmt, size_t fmt_len);

private:
    int anetSetTcpNoDelay(int fd, int val);
    int anetSetReuseAddr(int fd);
    int anetCreateSocket(int domain);
    int anetTcpGenericConnect(char *addr, int port, char *source_addr, int flags);
    int anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog);
    int anetV6Only(int s);
    int _anetTcpServer(int port, char *bindaddr, int af, int backlog);
    int anetGenericAccept(int s, struct sockaddr *sa, socklen_t *len);

private:
    int fd_;
    std::string addr_;
    int port_;
};

#endif
