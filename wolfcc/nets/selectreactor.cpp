#include <errno.h>
#include <string.h>
#include <cassert>
#include <algorithm>
#include <new>
#include <list>
#include "nets/selectreactor.h"
#include "nets/eventhandler.h"
#include "utils/logging.h"

using std::list;

struct Event
{
	bool on;
	bool persist;
	int fd;
	timespec timeout;

	Event()
	{
		memset(this, 0, sizeof(*this));
		fd = -1;
	}
};

struct SelectEventData
{
	Event			readev;
	Event			writeev;
	list<Event*>	timers;

	SelectEventData()
	{
		memset(&readev, 0, sizeof(readev));
		memset(&writeev, 0, sizeof(writeev));
	}
};


SelectReactor::SelectReactor(Allocator* allocator_)
: allocator(allocator_)
{
	
}

SelectReactor::~SelectReactor()
{
	
}

int SelectReactor::InitEventHandler(EventHandler* handler)
{
	if ( handler->GetReactorData() == NULL ) {

		SelectEventData* ed = AllocEventData();
		if ( ed == NULL ) {
			return -1;
		}

		handler->SetReactorData(ed);
	}

	handler->SetReactor(this);	
	return 0;
}

void SelectReactor::UninitEventHander(EventHandler* handler, SelectEventData* ed)
{
	ReleaseEventData(ed);
	handlers.erase(handler);

	handler->SetReactorData(NULL);

	handler->HandleClose();		
}

bool SelectReactor::HasEvent(SelectEventData* ed)
{
	return (ed->readev.on | ed->writeev.on) || !ed->timers.empty();
}

int SelectReactor::RegisterHandler(int mask, EventHandler* handler, timeval* timeout)
{
	InitEventHandler(handler);

	SelectEventData* ed = static_cast<SelectEventData*>(handler->GetReactorData());
	assert(ed!=NULL);

	if (mask & ReadMask)
	{
		ed->readev.on = true;
		ed->readev.fd = handler->GetHandle();

		if (timeout) 
		{
			clock_gettime(CLOCK_MONOTONIC, &ed->readev.timeout);
            log(LOG_DEBUG, "Current time: (%d,%d)", ed->readev.timeout.tv_sec, ed->readev.timeout.tv_nsec);
			
			ed->readev.timeout.tv_sec += timeout->tv_sec;
			ed->readev.timeout.tv_nsec += timeout->tv_usec * 1000;

            log(LOG_DEBUG, "Updated current time: (%d,%d)", ed->readev.timeout.tv_sec, ed->readev.timeout.tv_nsec);
		}

		ed->readev.persist = false;
		if (mask & PersistMask) 
		{
			ed->readev.persist = true;
		}

		handlers.insert(handler);
	}

	if (mask & WriteMask)
	{
		ed->writeev.on = true;
		ed->writeev.fd = handler->GetHandle();

		if (timeout) {
			clock_gettime(CLOCK_MONOTONIC, &ed->writeev.timeout);
			ed->writeev.timeout.tv_sec += timeout->tv_sec;
			ed->writeev.timeout.tv_nsec += timeout->tv_usec * 1000;
		}

		ed->writeev.persist = false;
		if (mask & PersistMask) {
			ed->writeev.persist = true;
		}

		handlers.insert(handler);
	}

	return 0;	
}

int SelectReactor::RemoveHandler(int mask, EventHandler* handler)
{
	SelectEventData* ed = static_cast<SelectEventData*>(handler->GetReactorData());
	if (!ed)
	{
		handler->HandleClose();
		return 0;
	}

	if (mask & ReadMask)
	{
		ed->readev.on = false;
	}
	if (mask & WriteMask)
	{
		ed->writeev.on = false;
	}
	if ( !HasEvent(ed) ) {
		UninitEventHander(handler, ed);
	}

	return 0;
}

int SelectReactor::ScheduleTimer(EventHandler* handler, timeval* timeout, void ** pTimerID)
{
	if ( timeout == NULL ) {
		return -1;
	}

    log(LOG_ERR, "ScheduleTimer (%d,%d)", timeout->tv_sec, timeout->tv_usec);

	InitEventHandler(handler);

	SelectEventData* ed = static_cast<SelectEventData*>(handler->GetReactorData());
	assert(ed);

	Event * pTimer = new Event;
	if ( pTimer == NULL ) {
		return -1;
	}

	pTimer->on = true;
	pTimer->persist = false;

	clock_gettime(CLOCK_MONOTONIC, &pTimer->timeout);
	pTimer->timeout.tv_sec += timeout->tv_sec;
	pTimer->timeout.tv_nsec += timeout->tv_usec * 1000;

	ed->timers.push_back(pTimer);

	if ( pTimerID ) {
		*pTimerID = pTimer;
	}

	handlers.insert(handler);
	
	return 0;
}

int SelectReactor::CancelTimer(EventHandler* handler, void * pTimerID)
{
	SelectEventData* ed = static_cast<SelectEventData*>(handler->GetReactorData());
	if (!ed) {
		handler->HandleClose();
		return 0;
	}

	if ( pTimerID == NULL ) {
		for ( list<Event *>::const_iterator pos=ed->timers.begin(); pos!=ed->timers.end(); ++pos ) {
			delete *pos;
		}
		ed->timers.clear();
	}
	else {
		Event* pTimer = static_cast<Event *>(pTimerID);

		list<Event *>::iterator pos = ed->timers.begin();
		for ( ; pos!=ed->timers.end(); ++pos ) {

			if ( *pos == pTimer) {
				break;
			}
		}
		if ( pos == ed->timers.end() ) {
			return -1;
		}
		
		ed->timers.erase(pos);
		delete pTimer;
	}

	if ( !HasEvent(ed) ) {
		UninitEventHander(handler, ed);
	}

	return 0;
}

int SelectReactor::RunOnce()
{
	EventHandler* currentTimer = NULL;
	Event* timer = NULL;

	fd_set readSet;
	fd_set writeSet;
	FD_ZERO(&readSet);
	FD_ZERO(&writeSet);

	int maxfd = 0;
	timespec min = {0x7fffffff, 0};
	bool timeLimit = false;
	
	std::set<EventHandler*>::const_iterator it;
	for (it = handlers.begin(); it != handlers.end(); it ++) {
		EventHandler* h = *it;
		SelectEventData* ed = (SelectEventData*)h->GetReactorData();
		assert(ed);
		
		if (ed->readev.on) {
			if (ed->readev.fd > maxfd) {
				maxfd = ed->readev.fd;
			}

			FD_SET(ed->readev.fd, &readSet);

			if (ed->readev.timeout.tv_sec != 0 || ed->readev.timeout.tv_nsec != 0) {
				if (ed->readev.timeout.tv_sec < min.tv_sec ||
					(ed->readev.timeout.tv_sec == min.tv_sec && ed->readev.timeout.tv_nsec < min.tv_nsec)) {
					timeLimit = true;
					min = ed->readev.timeout;
					currentTimer = h;
					timer = NULL;
				}
			}
		}

		// Process read event
		if (ed->writeev.on) 
		{
			if (ed->readev.fd > maxfd) 
			{
				maxfd = ed->readev.fd;
			}

			FD_SET(ed->writeev.fd, &writeSet);

			if (ed->writeev.timeout.tv_sec != 0 || ed->writeev.timeout.tv_nsec != 0) 
			{
				if (ed->writeev.timeout.tv_sec < min.tv_sec ||
					(ed->writeev.timeout.tv_sec == min.tv_sec && ed->writeev.timeout.tv_nsec < min.tv_nsec)) 
				{
					
					timeLimit = true;
					min = ed->writeev.timeout;
					currentTimer = h;
					timer = NULL;
				}
			}
		}

		// Process timers
		std::list<Event*>::const_iterator it1;
		for (it1 = ed->timers.begin(); it1 != ed->timers.end(); it1 ++) 
		{
			Event* t = *it1;
			if (t->timeout.tv_sec < min.tv_sec ||
				(t->timeout.tv_sec == min.tv_nsec && t->timeout.tv_nsec < min.tv_nsec)) 
			{

				min = t->timeout;
				timeLimit = true;
				currentTimer = h;
				timer = t;
			}
		}
	}

	timeval timeout = {0,0};
	// do select
	int ret = 0;
	bool selectWithTimeout = timeLimit && currentTimer;
	if (selectWithTimeout) 
	{
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);

        log(LOG_DEBUG, "Now: (%d,%d)", now.tv_sec, now.tv_nsec);
        log(LOG_DEBUG, "First timer: (%d,%d)", min.tv_sec, min.tv_nsec);

		timeout.tv_sec = min.tv_sec - now.tv_sec;
		timeout.tv_usec = (min.tv_nsec - now.tv_nsec) / 1000;

		while (timeout.tv_usec < 0) 
		{
			timeout.tv_sec -= 1;
			timeout.tv_usec += 1000 * 1000;
		}
		while (timeout.tv_usec >= 1000 * 1000) 
		{
			timeout.tv_sec += 1;
			timeout.tv_usec -= 1000 * 1000;
		}
		if (timeout.tv_sec < 0 || timeout.tv_usec < 0) 
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
		}

		ret = select(maxfd + 1, &readSet, &writeSet, NULL, &timeout);//selectº¯Êýµ÷ÓÃ
	}
	else 
	{
		ret = select(maxfd + 1, &readSet, &writeSet, NULL, NULL);
	}

	if (ret == -1) 
	{
		if (errno == EINTR) 
		{
			return 0;
		}

        log(LOG_WARN, "Select fail: %s, timeout: %d,%d, maxfd: %d, selectWithTimeout: %d",
			strerror(errno), timeout.tv_sec, timeout.tv_usec, maxfd, selectWithTimeout);
		return -1;
	}

	// Process timeout
	if (ret == 0) 
	{
        log(LOG_DEBUG, "Select timeout");

		currentTimer->HandleTimeout(timer);

		if (timer) 
		{
			SelectEventData* ed = (SelectEventData*)currentTimer->GetReactorData();
			std::list<Event*>::iterator it = std::find(ed->timers.begin(), ed->timers.end(), timer);
			assert(it != ed->timers.end());

			if (it != ed->timers.end()) 
			{
				ed->timers.erase(it);
				delete timer;
			}
		}
		return 0;
	}

	// Process read events
	for (int i=0; i<maxfd+1; i++) 
	{
		if (FD_ISSET(i, &readSet))
		{
			std::set<EventHandler*>::const_iterator it;
			for (it = handlers.begin(); it != handlers.end(); it ++) 
			{
				EventHandler* h = *it;
				SelectEventData* ed = (SelectEventData*)h->GetReactorData();
				
				if (ed->readev.on && ed->readev.fd == i)
				{
					if (!ed->readev.persist) 
					{
						ed->readev.on = false;
					}
					h->HandleInput();
					return 0;
				}
			}
		}
	}

	// Process write events
	for (int i=0; i<maxfd+1; i++) 
	{
		if (FD_ISSET(i, &writeSet)) 
		{
			std::set<EventHandler*>::const_iterator it;
			for (it = handlers.begin(); it != handlers.end(); it ++) 
			{
				EventHandler* h = *it;
				SelectEventData* ed = (SelectEventData*)h->GetReactorData();
				
				if (ed->writeev.on && ed->writeev.fd == i) 
				{
					if (!ed->writeev.persist) 
					{
						ed->writeev.on = false;
					}
					h->HandleOutput();
					return 0;
				}
			}
		}
	}

	return 0;
}

SelectEventData* SelectReactor::AllocEventData()
{
	void *p = NULL;
	if (allocator)
		p = allocator->Allocate(sizeof(SelectEventData));
	else
		p = malloc(sizeof(SelectEventData));

	return new(p) SelectEventData;
}

void SelectReactor::ReleaseEventData(SelectEventData* data)
{
	data->~SelectEventData();
	if (allocator)
		allocator->Deallocate(data);
	else
		free(data);
}

void SelectReactor::Run()
{
	stop = false;
	while ( !stop ) {
		if (RunOnce() != 0) {
			break;
		}
	}
}

void SelectReactor::Stop()
{
	stop = true;
}
