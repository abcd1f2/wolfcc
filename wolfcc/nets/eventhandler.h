#ifndef __WOLFCC_EVENTHANDLER_H__
#define __WOLFCC_EVENTHANDLER_H__
#include "nets/event.h"
#include "nets/handle.h"
#include "utils/destroyable.h"

class Reactor;

class EventHandler : public Destroyable
{
	Handle		handle;
	Reactor*	reactor;
	void*		reactordata;
 
public:
	EventHandler() : reactor(0), reactordata(0) {}

	virtual ~EventHandler() {}

	void SetHandle(Handle handle)
		{ this->handle = handle; }

	Handle GetHandle() const
		{ return this->handle; }

	void SetReactor(Reactor* reactor)
		{ this->reactor = reactor; }

	Reactor* GetReactor() const
		{ return this->reactor; }


	void SetReactorData(void* data)
		{ this->reactordata = data; }

	void* GetReactorData() const
		{ return reactordata; }

	virtual int Open();
	virtual int Close();

	virtual void HandleInput();
	virtual void HandleOutput();
	virtual void HandleTimeout(void * pTimerID);
	virtual void HandleClose();
};

#endif

