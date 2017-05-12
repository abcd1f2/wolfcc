#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <assert.h>
#include "utils/logging.h"
#include "nets/connector.h"
#include "nets/reactor.h"
#include "nets/socket.h"

using namespace std;

Connector::Connector()
	: stLocalAddr_(INetAddr::ANY_ADDR), time_(NULL)
{
}

Connector::Connector(const SockAddr & RemoteAddr, const timeval * time, const SockAddr & LocalAddr)
	: stRemoteAddr_(RemoteAddr), stLocalAddr_(LocalAddr), time_(NULL)
{
	SetTimeout(time);
}

Connector::~Connector()
{
	

	if ( time_ ) {
		delete time_;
	}
}

void Connector::SetTimeout(const timeval * time)
{
	if ( time != NULL ) {
		time_ = new timeval(*time);
	}
}

int Connector::Open()
{
	if (connfd_.Create(stRemoteAddr_.GetFamily(), SOCK_STREAM, 0) != 0) {
		log(LOG_ERR, "create socket error! %zd %s", errno, strerror(errno));
		return -1;
	}

	if ( connfd_.SetBlocking(false) != 0 ) {
        log(LOG_ERR, "set socket nonblock error! %zd %s", errno, strerror(errno));
        goto err;
	}

	if ( stRemoteAddr_.GetFamily() == AF_INET && stLocalAddr_ != INetAddr::ANY_ADDR ) {
		if ( connfd_.Bind(stLocalAddr_.GetSockAddrPtr(), stLocalAddr_.GetSockAddrSize()) != 0 ) {
            log(LOG_ERR, "bind socket to [%s] error! %zd %s", stLocalAddr_.ToString().c_str(), errno, strerror(errno));
            goto err;
		}
	}

	int ret = connfd_.Connect(stRemoteAddr_.GetSockAddrPtr(), stRemoteAddr_.GetSockAddrSize(), 0);
	if ( ret != 0 && errno != EINPROGRESS ) {
        log(LOG_ERR, "connect socket to [%s] error! %zd %s", stRemoteAddr_.ToString().c_str(), errno, strerror(errno));
        goto err;
	}

	if ( stRemoteAddr_.GetFamily() == AF_INET && connfd_.SetDelay(false) != 0) {
        log(LOG_ERR, "set non-delay fail: %s", strerror(errno));
        goto err;
	}    

	SetHandle(connfd_.GetHandle());

	if (GetReactor()->RegisterHandler(WriteMask, this, time_) != 0) {
		log(LOG_ERR, "register socket error");
        goto err;
	}

    return 0;

err:
    connfd_.Close();
    return -1;
}

int Connector::Close()
{
	return GetReactor()->RemoveHandler(WriteMask, this);
}

void Connector::HandleClose()
{
	if ( connfd_.GetHandle() != Socket().GetHandle() ) {
		connfd_.Close();
		connfd_ = Socket();
	}
	EventHandler::HandleClose();
}

void Connector::HandleInput()
{
}

void Connector::HandleOutput()
{
	int error;
	socklen_t len = sizeof(error);

	int ret = getsockopt(connfd_.GetHandle(), SOL_SOCKET, SO_ERROR, &error, &len);	
	if ( ret < 0 ) {
		OnError();
		return;
	}

	if ( error != 0 ) {
		errno = error;
		OnError();
		return;
	}

	if ( connfd_.SetBlocking(true) !=0 ) {
		OnError();
		return;
	}

	if ( stRemoteAddr_.GetFamily() == AF_LOCAL) {
		stLocalAddr_.Assign(UnixAddr(""));
	}
	else {
		if ( stLocalAddr_ == INetAddr::ANY_ADDR ) {
			if ( connfd_.GetLocalAddr(stLocalAddr_)!=0 ) {	
				OnError();
				return;
			}
		}
	}

	if ( OnConnected(connfd_.GetHandle()) == 0 ) {
		connfd_.SetHandle(Handle());
	}

	return;
}

void Connector::HandleTimeout(void *)
{
	errno = ETIMEDOUT;
	OnError();
	Close();
}

void Connector::OnError()
{
    Close();
}
