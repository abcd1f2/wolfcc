#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "nets/socket.h"
#include "nets/inetaddr.h"
#include "nets/sockaddr.h"
#include "nets/unixaddr.h"
#include "utils/logging.h"

int Socket::Create(int domain, int type, int protocol)
{
	handle = socket(domain, type, protocol);
	if (handle == Handle())
		return -1;
	return 0;
}

int Socket::Connect(const SockAddr& addr, uint32_t usec_timeout)
{
	return Connect(addr.GetSockAddrPtr(), addr.GetSockAddrSize(), usec_timeout);
}

int Socket::Connect(const sockaddr* addr, size_t addrlen, uint32_t usec_timeout)
{
	if (usec_timeout == 0)
	{
		return ::connect(handle, addr, addrlen);
	}
	else
	{
		bool blocking;
		if (GetBlocking(blocking) == -1)
			return -1;

		if (blocking)
		{
			if (SetBlocking(false) == -1)
				return -1;
		}

		int ret = ::connect(handle, addr, addrlen);
		if (ret == -1)
		{
			if (errno == EINPROGRESS)
			{
				struct pollfd stPollFd;
				stPollFd.fd = handle;
				stPollFd.events = POLLOUT;
				stPollFd.revents = 0;
				int timeout = usec_timeout/1000;

				int ret = poll(&stPollFd, 1, timeout);
				if ( ret < 0 ) 
				{
					return -1;
				}
				else if ( ret == 0 ) 
				     {
						errno = ETIMEDOUT;
						return -1;
				     }
				else if ( stPollFd.revents != POLLOUT ) 
					 { 
						return -1;
					 }					
					else 
					{
						int error = -1;
						int len = sizeof(error);
						getsockopt(handle, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
						if (error != 0) 
						{
							return -1;
						}
					}
			}
			else
			{
				return -1;
			}
		}

		if (blocking)
		{
			if (SetBlocking(true) == -1)
				return -1;
		}

		return 0;
	}
}

int Socket::Bind(const sockaddr* addr, size_t addrlen)
{
	if (handle == -1)
		return -1;

	int i = 1;
	if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(int)) == -1)
	{
		return -1;
	}

	if (::bind(handle, addr, addrlen) != 0)
	{
		return -1;
	}

	return 0;
}

int Socket::Bind(const SockAddr& addr)
{
	return Bind(addr.GetSockAddrPtr(), addr.GetSockAddrSize());
}

int Socket::Listen(unsigned int backlog)
{
	return ::listen(handle, backlog);
}

int Socket::Accept(Socket& client)
{
	sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	int s = ::accept(handle, reinterpret_cast<sockaddr*>(&clientaddr), &len);
	if (s == -1)
		return -1;

	client.handle = s;
	return 0;
}

int Socket::SetSendTimeout(uint32_t usec_timeout)
{
    timeval sendrecvtimeout;
    sendrecvtimeout.tv_sec = 0;
    sendrecvtimeout.tv_usec = usec_timeout;

	if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (void *)&sendrecvtimeout,sizeof(struct timeval)) != 0)
		return -1;

	return 0;
}

int Socket::SetRecvTimeout(uint32_t usec_timeout)
{
    timeval sendrecvtimeout;
    sendrecvtimeout.tv_sec = 0;
    sendrecvtimeout.tv_usec = usec_timeout;

	if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (void *)&sendrecvtimeout,sizeof(struct timeval)) != 0)
		return -1;

	return 0;
}

int Socket::SetDelay(bool delay)
{
	int flags = 0;
	if (!delay)
		flags = 1;

	if (setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof(flags)) != 0)
		return -1;

	return 0;
}

int Socket::GetBlocking(bool& blocking)
{
	int oldflags = fcntl(handle, F_GETFL, 0);
	if (oldflags == -1)
	{
		return -1;
	}
	blocking = ((oldflags & O_NONBLOCK) ? false : true);
	return 0;
}

int Socket::SetBlocking(bool blocking)
{
	int flags = fcntl(handle, F_GETFL, 0);
	if (flags == -1)
	{
		return -1;
	}

	if (blocking)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	if (fcntl(handle, F_SETFL, flags) != 0)
	{
		return -1;
	}

	return 0;
}

void Socket::Close()
{
	if ( handle != Handle() ) {
		::close(handle);
		handle = Handle();
	}
}

int Socket::GetRemoteAddr(SockAddr & stAddr)
{
	union {
		sockaddr generic;
		sockaddr_in  in_4;
		sockaddr_in6 in_6;
		sockaddr_un unix_domain;
	} Addr;

	memset(&Addr, 0, sizeof(Addr));

	socklen_t addrlen = sizeof(Addr);

	if ( getpeername(handle, &Addr.generic, &addrlen) != 0 ) {
		return -1;
	}

	stAddr.Assign((sockaddr*)&Addr);
	return 0;
}

int Socket::GetLocalAddr(SockAddr & stAddr)
{
	union {
		sockaddr generic;
		sockaddr_in  in_4;
		sockaddr_in6 in_6;
		sockaddr_un unix_domain;
	} Addr;

	memset(&Addr, 0, sizeof(Addr));

	socklen_t addrlen = sizeof(Addr);

	if ( getsockname(handle, &Addr.generic, &addrlen) != 0 ) {
		return -1;
	}

	stAddr.Assign((sockaddr*)&Addr);
	return 0;
}
