#ifndef __WOLFCC_ACCEPTOR_H__
#define __WOLFCC_ACCEPTOR_H__
#include "nets/inetaddr.h"
#include "nets/eventhandler.h"
#include "nets/sockaddr.h"

class Acceptor : public EventHandler
{
public:
	Acceptor();
	Acceptor(Handle handle);
	Acceptor(const SockAddr & stSockAddr);
	virtual ~Acceptor();
	
public:
	virtual int Open();
	virtual int Close();
	virtual void HandleClose();
	virtual void HandleInput();
	virtual void DestroyHandler(EventHandler* handler) = 0;

public:
	inline void SetAddr(const SockAddr & stAddr) { stAddr_ = stAddr; }
	inline const SockAddr & GetAddr() { return stAddr_; }

protected:
	virtual int OnAccept(Handle handle, const SockAddr & addr);
	virtual void OnConnected(EventHandler* eventHandler);
	virtual EventHandler* CreateHandler(const SockAddr & RemoteAddr) = 0;

protected:
	SockAddr stAddr_;
};

#endif 

