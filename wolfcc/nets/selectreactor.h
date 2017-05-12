#ifndef __WOLFCC_SELECT_REACTOR_H__
#define __WOLFCC_SELECT_REACTOR_H__
#include <set>
#include "nets/reactor.h"

struct SelectEventData;
class Allocator;

class SelectReactor : public Reactor
{
	Allocator*		allocator;
	std::set<EventHandler*> handlers;
	bool stop;

public:
	SelectReactor(Allocator* allocator=NULL);
	virtual ~SelectReactor();

	virtual int RegisterHandler(int mask, EventHandler* handler, timeval* timeout=0);

	virtual int RemoveHandler(int mask, EventHandler* handler);

	virtual int ScheduleTimer(EventHandler* handler, timeval* timeout, void ** pTimerID=NULL);

	virtual int CancelTimer(EventHandler* handler, void * pTimerID=NULL /* cancel all Timers */);

	virtual int RunOnce();

	virtual void Run();

	virtual void Stop();

protected:

	int InitEventHandler(EventHandler* handler);

	void UninitEventHander(EventHandler* handler, SelectEventData* ed);
	
	bool HasEvent(SelectEventData* ed);

private:

	SelectEventData* AllocEventData();
	void ReleaseEventData(SelectEventData* data);
};

#endif
