#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "nets/udplistener.h"
#include "nets/socket.h"
#include "nets/reactor.h"
#include "utils/logging.h"

UdpListener::UdpListener(const SockAddr& addr)
:	bindaddr(addr)
{
}

int UdpListener::Open()
{
	Socket sock;
	if (sock.Create(bindaddr.GetFamily(), SOCK_DGRAM, 0) != 0)
		return -1;

	int buflen = 256 * 1024;
	socklen_t len = sizeof(buflen);
	if (setsockopt(sock.GetHandle(), SOL_SOCKET, SO_RCVBUF, &buflen, len) != 0)
	{
		log(LOG_ERR,"setsockopt fail: %s", strerror(errno));
		sock.Close();
		return -1;
	}

	if (sock.Bind(bindaddr.GetSockAddrPtr(), bindaddr.GetSockAddrSize()) != 0)
	{
		sock.Close();
		return -1;
	}

	SetHandle(sock.GetHandle());

	if (GetReactor()->RegisterHandler(ReadMask | PersistMask, this) != 0)
	{
		SetHandle(Handle());
		sock.Close();
		return -1;
	}
	return 0;
}

int UdpListener::Close()
{
	return GetReactor()->RemoveHandler(ReadMask, this);
}

void UdpListener::HandleInput()
{
	char buffer[1024 * 9];
	sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	int ret = recvfrom(GetHandle(), buffer, sizeof(buffer), 0, (sockaddr*)&addr, &addrlen);
	if (ret == 0 || (ret == -1 && (errno != EINTR && errno != EAGAIN))) {
		Close();
		return;
	}

	HandlePackage(buffer, ret, SockAddr(&addr));
}

void UdpListener::HandleClose()
{
	::close(GetHandle());
	EventHandler::HandleClose();
}
