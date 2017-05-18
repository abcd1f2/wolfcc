#include <cassert>
#include <string.h>
#include <new>
#include <list>
#include <event.h>
#include "nets/libeventreactor.h"
#include "nets/eventhandler.h"
#include "utils/logging.h"
#include "nets/event.h"

/* private data */
struct TimerData
{
    event	timer;
    void *	argv[2];

    TimerData()
    {
        memset(&timer, 0, sizeof(timer));
        memset(argv, 0, sizeof(argv));
    }
};

struct EventData
{
    event			readev;
    event			writeev;
    std::list<TimerData*>	timers;

    EventData()
    {
        memset(&readev, 0, sizeof(readev));
        memset(&writeev, 0, sizeof(writeev));
    }
};


// private callback
static void __EventCb(int/* fd*/, short events, void *arg)
{
    EventHandler* handler = static_cast<EventHandler*>(arg);
    assert(handler);

    if (events & EV_TIMEOUT) {
        handler->HandleTimeout(NULL);
    }
    else if (events & EV_READ) {
        handler->HandleInput();
    }
    else if (events & EV_WRITE) {
        handler->HandleOutput();
    }
}


static void __EventCb4Timer(int/* fd*/, short events, void *arg)
{
    if (!(events & EV_TIMEOUT)) {
        return;
    }

    void ** argv = static_cast<void **>(arg);
    assert(argv != NULL);

    EventHandler* handler = static_cast<EventHandler*>(argv[0]);
    assert(handler);

    TimerData* pTimerData = static_cast<TimerData*>(argv[1]);
    assert(pTimerData);

    // delete timer from handler
    EventData* ed = static_cast<EventData*>(handler->GetReactorData());
    assert(ed);


    // call handler
    handler->HandleTimeout(pTimerData);

    handler->GetReactor()->CancelTimer(handler, pTimerData);
}


LibEventReactor::LibEventReactor(Allocator* allocator_)
    : allocator(allocator_)
{
    evb = static_cast<event_base*>(event_init());
}

LibEventReactor::~LibEventReactor()
{
    // TODO: safe release
    //skip this to avoid the coredump
    //event_base_free(evb);
}

int LibEventReactor::InitEventHandler(EventHandler* handler)
{
    if (handler->GetReactorData() == NULL) {

        EventData* ed = AllocEventData();
        if (ed == NULL) {
            return -1;
        }

        handler->SetReactorData(ed);
    }

    handler->SetReactor(this);
    return 0;
}

void LibEventReactor::UninitEventHander(EventHandler* handler, EventData* ed)
{
    ReleaseEventData(ed);
    handler->SetReactorData(NULL);

    //handler->SetReactorData(0);
    handler->HandleClose();
}

bool LibEventReactor::HasEvent(EventData* ed)
{
    return (ed->readev.ev_events | ed->writeev.ev_events) || !ed->timers.empty();
}

int LibEventReactor::RegisterHandler(int mask, EventHandler* handler, timeval* timeout)
{
    InitEventHandler(handler);

    EventData* ed = static_cast<EventData*>(handler->GetReactorData());
    assert(ed != NULL);

    short events = 0;
    if (mask & ReadMask)
    {
        events |= EV_READ;
        if (timeout) {
            events |= EV_TIMEOUT;
        }

        if (mask & PersistMask) {
            events |= EV_PERSIST;
        }

        event_del(&ed->readev);
        event_set(&ed->readev, handler->GetHandle(), events, __EventCb, handler);
        event_base_set(evb, &ed->readev);
        if (event_add(&ed->readev, timeout) != 0) {
            return -1;
        }
    }

    events = 0;
    if (mask & WriteMask)
    {
        events |= EV_WRITE;
        if (timeout) {
            events |= EV_TIMEOUT;
        }

        if (mask & PersistMask) {
            events |= EV_PERSIST;
        }

        event_del(&ed->writeev);
        event_set(&ed->writeev, handler->GetHandle(), events, __EventCb, handler);
        event_base_set(evb, &ed->writeev);
        if (event_add(&ed->writeev, timeout) != 0) {
            return -1;
        }
    }

    return 0;
}

int LibEventReactor::RemoveHandler(int mask, EventHandler* handler)
{
    EventData* ed = static_cast<EventData*>(handler->GetReactorData());
    if (!ed)
    {
        handler->HandleClose();
        return 0;
    }

    if (mask & ReadMask)
    {
        event_del(&ed->readev);
        memset(&ed->readev, 0, sizeof(ed->readev));
    }
    if (mask & WriteMask)
    {
        event_del(&ed->writeev);
        memset(&ed->writeev, 0, sizeof(ed->writeev));
    }

    if (!HasEvent(ed)) {
        UninitEventHander(handler, ed);
    }

    return 0;
}

int LibEventReactor::ScheduleTimer(EventHandler* handler, timeval* timeout, void ** pTimerID)
{
    if (timeout == NULL) {
        return -1;
    }

    InitEventHandler(handler);

    EventData* ed = static_cast<EventData*>(handler->GetReactorData());
    assert(ed != NULL);

    TimerData * pTimerData = new TimerData;
    if (pTimerData == NULL) {
        return -1;
    }

    pTimerData->argv[0] = handler;
    pTimerData->argv[1] = pTimerData;

    evtimer_set(&pTimerData->timer, __EventCb4Timer, &pTimerData->argv);
    event_base_set(evb, &pTimerData->timer);
    if (evtimer_add(&pTimerData->timer, timeout) != 0) {
        delete pTimerData;
        return -1;
    }

    ed->timers.push_back(pTimerData);
    if (pTimerID) {
        *pTimerID = pTimerData;
    }

    return 0;
}

int LibEventReactor::CancelTimer(EventHandler* handler, void * pTimerID)
{
    EventData* ed = static_cast<EventData*>(handler->GetReactorData());
    if (!ed)
    {
        handler->HandleClose();
        return 0;
    }

    if (pTimerID == NULL) {
        for (std::list<TimerData *>::const_iterator pos = ed->timers.begin(); pos != ed->timers.end(); ++pos) {
            event_del(&(*pos)->timer);
            delete *pos;
        }
        ed->timers.clear();
    }
    else {
        TimerData* pTimerData = static_cast<TimerData *>(pTimerID);

        std::list<TimerData*>::iterator pos = ed->timers.begin();
        for (; pos != ed->timers.end(); ++pos) {

            if (*pos == pTimerData) {
                break;
            }
        }
        if (pos == ed->timers.end()) {
            return -1;
        }

        if (event_del(&pTimerData->timer) != 0) {
            return -1;
        }

        ed->timers.erase(pos);
        delete pTimerData;
    }

    if (!HasEvent(ed)) {
        UninitEventHander(handler, ed);
    }

    return 0;
}

int LibEventReactor::RunOnce()
{
    return event_base_loop(evb, EVLOOP_ONCE);
}

EventData* LibEventReactor::AllocEventData()
{
    void *p = NULL;
    if (allocator)
        p = allocator->Allocate(sizeof(EventData));
    else
        p = malloc(sizeof(EventData));

    return new(p)EventData;
}

void LibEventReactor::ReleaseEventData(EventData* data)
{
    data->~EventData();
    if (allocator)
        allocator->Deallocate(data);
    else
        free(data);
}

void LibEventReactor::Run()
{
    event_base_dispatch(evb);
}

void LibEventReactor::Stop()
{
    event_base_loopexit(evb, 0);
}
