#ifndef __WOLFCC_REACTOR_H__
#define __WOLFCC_REACTOR_H__
#include "nets/event.h"
#include <sys/time.h>
#include <stddef.h>

class EventHandler;

class Reactor
{
public:
	Reactor(){}
	virtual ~Reactor(){}

	virtual int RegisterHandler(int mask, EventHandler* handler, timeval* timeout=0) = 0;

	virtual int RemoveHandler(int mask, EventHandler* handler) = 0;

	virtual int ScheduleTimer(EventHandler* handler, timeval* timeout, void ** pTimerID=NULL) = 0;

	virtual int CancelTimer(EventHandler* handler, void * pTimerID=NULL /* cancel all Timers */) = 0;

	virtual int RunOnce() = 0;

	virtual void Run();

	virtual void Stop() { running = false; }

    bool IsRunning() const { return running; }

private:
	bool running;	
};

#endif 

