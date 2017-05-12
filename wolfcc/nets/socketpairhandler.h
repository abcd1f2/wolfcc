#ifndef __WOLFCC_SOCKETPAIRHANDLER_H__
#define __WOLFCC_SOCKETPAIRHANDLER_H__
#include "nets/eventhandler.h"

class SocketPairHandler : public EventHandler
{
public:
	Handle GetAnotherHandle() const
		{ return anotherhandle; }

	virtual int Open();

protected:
	virtual void HandleInput();

	virtual void HandleClose();

	virtual void HandleEvent() = 0;

private:
	void CloseHandles();

private:
	Handle anotherhandle;
};

#endif
