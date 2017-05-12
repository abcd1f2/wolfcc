#ifndef __WOLFCC_SOCKET_H__
#define __WOLFCC_SOCKET_H__

#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "nets/handle.h"
#include "nets/sockaddr.h"
#include "nets/inetaddr.h"

class Socket
{
private:
	Handle handle;

public:
	Socket(Handle handle_=Handle())
	: handle(handle_)
	{ }

	Handle GetHandle() const { return handle; }

	void SetHandle(Handle handle) { this->handle = handle; }

	int Create(int domain, int type, int protocol);

	int Connect(const SockAddr& addr, uint32_t usec_timeout);

	int Connect(const sockaddr* addr, size_t addrlen, uint32_t usec_timeout);

	int Bind(const sockaddr* addr, size_t addrlen);

	int Bind(const SockAddr& addr);

	int Listen(uint32_t backlog);

	int Accept(Socket& client);

	int SetSendTimeout(uint32_t usec_timeout);

	int SetRecvTimeout(uint32_t usec_timeout);

	int SetDelay(bool delay);

	int SetBlocking(bool blocking);

	int GetBlocking(bool& blocking);

	int Send(const char* p, size_t len);

	int Sendn(const char* p, size_t len);

	int Recv(char* p, size_t len);

	void Close();

	int GetRemoteAddr(SockAddr & stAddr);

	int GetLocalAddr(SockAddr & stAddr);
};

#endif
