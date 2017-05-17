#ifndef __WOLFCC_NETMANAGER_H__
#define __WOLFCC_NETMANAGER_H__
#include <stdint.h>
#include <string>

#define _BSD_SOURCE

#if defined(__linux__)
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#endif

#if defined(_AIX)
#define _ALL_SOURCE
#endif

#if defined(__linux__) || defined(__OpenBSD__)
#define _XOPEN_SOURCE 700
/*
* On NetBSD, _XOPEN_SOURCE undefines _NETBSD_SOURCE and
* thus hides inet_aton etc.
*/
#elif !defined(__NetBSD__)
#define _XOPEN_SOURCE
#endif

#if defined(__sun)
#define _POSIX_C_SOURCE 199506L
#endif

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

class NetManager
{
public:
	int anetTcpConnect(char *err, char *addr, int port);
	int anetTcpNonBlockConnect(char *err, char *addr, int port);
	int anetTcpNonBlockBindConnect(char *err, char *addr, int port, char *source_addr);
	int anetTcpNonBlockBestEffortBindConnect(char *err, char *addr, int port, char *source_addr);
	int anetUnixConnect(char *err, char *path);
	int anetUnixNonBlockConnect(char *err, char *path);
	int anetRead(int fd, char *buf, int count);
	int anetResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len);
	int anetResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len);
	int anetTcpServer(char *err, int port, char *bindaddr, int backlog);
	int anetTcp6Server(char *err, int port, char *bindaddr, int backlog);
	int anetUnixServer(char *err, char *path, mode_t perm, int backlog);
	int anetTcpAccept(char *err, int serversock, char *ip, size_t ip_len, int *port);
	int anetUnixAccept(char *err, int serversock);
	int anetWrite(int fd, char *buf, int count);
	int anetNonBlock(char *err, int fd);
	int anetBlock(char *err, int fd);
	int anetEnableTcpNoDelay(char *err, int fd);
	int anetDisableTcpNoDelay(char *err, int fd);
	int anetTcpKeepAlive(char *err, int fd);
	int anetSendTimeout(char *err, int fd, long long ms);
	int anetPeerToString(int fd, char *ip, size_t ip_len, int *port);
	int anetKeepAlive(char *err, int fd, int interval);
	int anetSockName(int fd, char *ip, size_t ip_len, int *port);
	int anetFormatAddr(char *fmt, size_t fmt_len, char *ip, int port);
	int anetFormatPeer(int fd, char *fmt, size_t fmt_len);
	int anetFormatSock(int fd, char *fmt, size_t fmt_len);
};

#endif