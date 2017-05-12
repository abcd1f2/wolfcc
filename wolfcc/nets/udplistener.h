#ifndef __WOLFCC_LISTENER_H__
#define __WOLFCC_LISTENER_H__
#include "nets/eventhandler.h"
#include "nets/sockaddr.h"

class UdpListener : public EventHandler
{
	SockAddr bindaddr;

public:
	UdpListener(const SockAddr& addr);

	virtual int Open();

	virtual int Close();

	virtual void HandleInput();

	virtual void HandleClose();

	virtual void HandlePackage(const char* p, size_t len, const SockAddr& addr) = 0;
};

#endif 
