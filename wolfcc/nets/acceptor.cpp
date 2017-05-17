#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "nets/acceptor.h"
#include "nets/reactor.h"
#include "nets/socket.h"
#include "nets/eventhandler.h"
#include "nets/inetaddr.h"
#include "nets/unixaddr.h"
#include "utils/logging.h"
#include "utils/scopeguard.h"

Acceptor::Acceptor()
{
}

Acceptor::Acceptor(Handle handle)
{
	SetHandle(handle);
}

Acceptor::Acceptor(const SockAddr & stSockAddr)
	: stAddr_(stSockAddr)
{
}

Acceptor::~Acceptor()
{
}

int Acceptor::Open()
{
	bool newSocket = false;

	if (GetHandle() == Handle()) {
		Socket listensock;

		if (stAddr_.GetFamily() == AF_LOCAL || stAddr_.GetFamily() == AF_FILE) {
			::remove(stAddr_.ToString().c_str());
		}

		if (listensock.Create(stAddr_.GetFamily(), SOCK_STREAM, 0) != 0) {
			return -1;
		}

		if (listensock.SetBlocking(false) != 0) {
			listensock.Close();
			return -1;
		}

		if (stAddr_.GetFamily() == AF_INET && listensock.SetDelay(false) != 0) {
			listensock.Close();
			return -1;
		}

		if (listensock.Bind(stAddr_.GetSockAddrPtr(), stAddr_.GetSockAddrSize()) != 0) {
			listensock.Close();
			return -1;
		}

		if (listensock.Listen(1024) != 0) {
			listensock.Close();
			return -1;
		}

		SetHandle(listensock.GetHandle());
		newSocket = true;
	}
	else {
		Socket listensock(GetHandle());

		if (listensock.SetBlocking(false) != 0) {
			return -1;
		}

		if (listensock.SetDelay(false) != 0) {
		//	return -1;	// maybe unix local socket
		}

		if ( listensock.GetLocalAddr(stAddr_) != 0 ) {
			return -1;
		}
	}

	if (GetReactor()->RegisterHandler(ReadMask | PersistMask, this) != 0) {
		if (newSocket) {
			::close(GetHandle());
			SetHandle(Handle());
		}
		return -1;
	}

	return 0;
}

int Acceptor::Close()
{
	return GetReactor()->RemoveHandler(ReadMask | WriteMask, this);
}

void Acceptor::HandleClose()
{
	::close(GetHandle());

    if (stAddr_.GetFamily() == AF_LOCAL || stAddr_.GetFamily() == AF_FILE) {
        ::remove(stAddr_.ToString().c_str());
    }

	SetHandle(Handle());
}

void Acceptor::HandleInput()
{
	union {
		sockaddr generic;
		sockaddr_in  in_4;
		sockaddr_in6 in_6;
		sockaddr_un unix_domain;
	} stAddr;

	socklen_t addrlen = sizeof(stAddr);

	Handle handle = ::accept(GetHandle(), (sockaddr*)&stAddr.generic, &addrlen);
	if (handle == Handle()) {
		return;
	}

	// guard
	ScopeGuard closeguard = MakeGuard(::close, handle);

	SockAddr stRemoteAddr;
	if ( stAddr.generic.sa_family == AF_LOCAL ) {
		stRemoteAddr.Assign(UnixAddr(""));
	}
	else {
		stRemoteAddr.Assign((sockaddr*)&stAddr);
	}

	log(LOG_DEBUG, "accept connet from %s", stRemoteAddr.ToString().c_str());

	if (OnAccept(handle, stRemoteAddr) != 0) {
		return;
	}

	EventHandler* handler;
	if ( (handler = CreateHandler(stRemoteAddr)) == NULL ) {
		return;
	}

	handler->SetHandle(handle);
	handler->SetReactor(GetReactor());
	if (handler->Open() != 0) {
		DestroyHandler(handler);
		return;
	}

	OnConnected(handler);
	closeguard.Dismiss();
}

int Acceptor::OnAccept(Handle, const SockAddr &)
{
	return 0;
}

void Acceptor::OnConnected(EventHandler *)
{
	
}
