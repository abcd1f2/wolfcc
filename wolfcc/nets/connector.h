#ifndef __WOLFCC_CONNECTOR_H__
#define __WOLFCC_CONNECTOR_H__
#include <sys/time.h>
#include "nets/eventhandler.h"
#include "nets/socket.h"
#include "nets/inetaddr.h"
#include "nets/sockaddr.h"

class Connector : public EventHandler
{
public:
	Connector();

	Connector(const SockAddr & RemoteAddr, const timeval * time=NULL, const SockAddr & LocalAddr=INetAddr::ANY_ADDR);

	virtual ~Connector();

	inline void SetRemoteAddr(const SockAddr & RemoteAddr) { stRemoteAddr_= RemoteAddr; }
	inline const SockAddr & GetRemoteAddr() const { return stRemoteAddr_; }

	inline void SetLocalAddr(const SockAddr & LocalAddr) { stLocalAddr_ = LocalAddr; }
	inline const SockAddr & GetLocalAddr() const { return stLocalAddr_; }

	void SetTimeout(const timeval * time);

	virtual int Open();

	virtual int Close();

	virtual void HandleClose();

	virtual void HandleInput();

	virtual void HandleOutput();

	virtual void HandleTimeout(void *);

protected:

	virtual int OnConnected(Handle handle) = 0;

	virtual void OnError();

private:
	SockAddr stRemoteAddr_;
	SockAddr stLocalAddr_;
	timeval * time_;
	Socket connfd_;
};

#endif

