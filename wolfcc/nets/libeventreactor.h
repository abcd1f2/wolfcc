#ifndef __WOLFCC_LIBEVENTREACTOR_H__
#define __WOLFCC_LIBEVENTREACTOR_H__
#include "nets/reactor.h"
#include "utils/allocator.h"

struct event_base;
struct EventData;

class LibEventReactor : public Reactor
{
public:
    LibEventReactor(Allocator* allocator = NULL);
    virtual ~LibEventReactor();

    virtual int RegisterHandler(int mask, EventHandler* handler, timeval* timeout = 0);

    virtual int RemoveHandler(int mask, EventHandler* handler);

    virtual int ScheduleTimer(EventHandler* handler, timeval* timeout, void ** pTimerID = NULL);

    virtual int CancelTimer(EventHandler* handler, void * pTimerID = NULL /* cancel all Timers */);

    virtual int RunOnce();

    virtual void Run();

    virtual void Stop();

protected:
    int InitEventHandler(EventHandler* handler);

    void UninitEventHander(EventHandler* handler, EventData* ed);

    bool HasEvent(EventData* ed);


private:
    EventData* AllocEventData();
    void ReleaseEventData(EventData* data);

private:
    Allocator   *allocator;
    event_base  *evb;
};

#endif
