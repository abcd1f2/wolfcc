#include <assert.h>
#include "netmanager.h"
#include "nets/netevent.h"
#include "utils/wtime.h"
#include "utils/logging.h"
#include "poll.h"

NetManager::NetManager(NetEvent *net_event):
    net_event_p_(net_event)
{
    net_work_p_ = new NetWork();
    assert(net_work_p_);

    event_loop_p_ = new aeEventLoop();
    assert(event_loop_p_);
}

NetManager::~NetManager()
{

}

bool NetManager::StartListen()
{
    if (!net_work_p_->listenToPort(0, NULL, NULL)) {
        log(LOG_ERR, "listen to port error");
        return false;
    }

    if (!net_work_p_->listUnixSocket()) {
        log(LOG_ERR, "list unix socket error");
        return false;
    }

    if (!aeCreateEventLoop(EPOLL_EVENTS_NUM)) {
        log(LOG_ERR, "create event loop error");
        return false;
    }

    return true;
}

bool NetManager::aeCreateEventLoop(int setsize)
{
    assert(net_event_p_);
    net_event_p_->SetEventLoop(event_loop_p_);

    event_loop_p_->events = (aeFileEvent *)malloc(sizeof(aeFileEvent)*setsize);
    event_loop_p_->fired = (aeFiredEvent *)malloc(sizeof(aeFiredEvent)*setsize);
    if (event_loop_p_->events == NULL || event_loop_p_->fired == NULL) {
        goto err;
    }

    event_loop_p_->setsize = setsize;
    event_loop_p_->lastTime = get_stime();
    event_loop_p_->timeEventHead = NULL;
    event_loop_p_->timeEventNextId = 0;
    event_loop_p_->stop = 0;
    event_loop_p_->maxfd = -1;
    event_loop_p_->beforesleep = NULL;

    if (-1 == net_event_p_->Create()) {
        goto err;
    }
    
    for (int i = 0; i < setsize; i++)
        event_loop_p_->events[i].mask = AE_NONE;
    return true;

err:
    if (event_loop_p_) {
        free(event_loop_p_->events);
        free(event_loop_p_->fired);
        free(event_loop_p_);
    }
    return false;
}

int NetManager::aeGetSetSize() 
{
    return event_loop_p_->setsize;
}

int NetManager::aeResizeSetSize(int setsize)
{
    int i;

    if (setsize == event_loop_p_->setsize) return AE_OK;
    if (event_loop_p_->maxfd >= setsize) return AE_ERR;
    if (net_event_p_->Resize(setsize) == -1) return AE_ERR;

    event_loop_p_->events = (aeFileEvent *)realloc(event_loop_p_->events, sizeof(aeFileEvent)*setsize);
    event_loop_p_->fired = (aeFiredEvent *)realloc(event_loop_p_->fired, sizeof(aeFiredEvent)*setsize);
    event_loop_p_->setsize = setsize;

    /* Make sure that if we created new slots, they are initialized with
    * an AE_NONE mask. */
    for (i = event_loop_p_->maxfd + 1; i < setsize; i++)
        event_loop_p_->events[i].mask = AE_NONE;
    return AE_OK;
}

void NetManager::aeDeleteEventLoop() 
{
    net_event_p_->Free();
    free(event_loop_p_->events);
    free(event_loop_p_->fired);
    delete event_loop_p_;
}

void NetManager::aeStop()
{
    event_loop_p_->stop = 1;
}

int NetManager::aeCreateFileEvent(int fd, int mask, aeFileProc *proc, void *clientData)
{
    if (fd >= event_loop_p_->setsize) {
        errno = ERANGE;
        return AE_ERR;
    }

    aeFileEvent *fe = &event_loop_p_->events[fd];
    if (net_event_p_->AddEvent(fd, mask) == -1) {
        return AE_ERR;
    }

    fe->mask |= mask;

    // 设置回调函数
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    fe->clientData = clientData;
    if (fd > event_loop_p_->maxfd)
        event_loop_p_->maxfd = fd;
    return AE_OK;
}

void NetManager::aeDeleteFileEvent(int fd, int mask)
{
    if (fd >= event_loop_p_->setsize) return;
    aeFileEvent *fe = &event_loop_p_->events[fd];
    if (fe->mask == AE_NONE) return;

    net_event_p_->DelEvent(fd, mask);
    fe->mask = fe->mask & (~mask);
    if (fd == event_loop_p_->maxfd && fe->mask == AE_NONE) {
        /* Update the max fd */
        int j;

        for (j = event_loop_p_->maxfd - 1; j >= 0; j--) {
            if (event_loop_p_->events[j].mask != AE_NONE) {
                break;
            }
        }
        event_loop_p_->maxfd = j;
    }
}

int NetManager::aeGetFileEvents(int fd)
{
    if (fd >= event_loop_p_->setsize) {
        return 0;
    }

    aeFileEvent *fe = &event_loop_p_->events[fd];

    return fe->mask;
}

void NetManager::aeAddMillisecondsToNow(int64_t milliseconds, uint32_t &sec, uint32_t &ms)
{
    uint32_t cur_sec, cur_ms, when_sec, when_ms;

    get_time_pair(cur_sec, cur_ms);
    when_sec = cur_sec + milliseconds / 1000;
    when_ms = cur_ms + milliseconds % 1000;
    if (when_ms >= 1000) {
        when_sec++;
        when_ms -= 1000;
    }
    sec = when_sec;
    ms = when_ms;
}

int64_t NetManager::aeCreateTimeEvent(int64_t milliseconds, aeTimeProc *proc, void *clientData, aeEventFinalizerProc *finalizerProc)
{
    long long id = event_loop_p_->timeEventNextId++;
    aeTimeEvent *te;
    te = (aeTimeEvent *)malloc(sizeof(*te));
    if (te == NULL)  {
        return AE_ERR;
    }

    te->id = id;
    aeAddMillisecondsToNow(milliseconds, &te->when_sec, &te->when_ms);
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    te->clientData = clientData;

    te->next = event_loop_p_->timeEventHead;
    event_loop_p_->timeEventHead = te;
    return id;
}

int NetManager::aeDeleteTimeEvent(int64_t id)
{
    aeTimeEvent *te = event_loop_p_->timeEventHead;
    while (te) {
        if (te->id == id) {
            te->id = AE_DELETED_EVENT_ID;
            return AE_OK;
        }
        te = te->next;
    }
    return AE_ERR;
}

aeTimeEvent *NetManager::aeSearchNearestTimer()
{
    aeTimeEvent *te = event_loop_p_->timeEventHead;
    aeTimeEvent *nearest = NULL;

    while (te) {
        if (!nearest || te->when_sec < nearest->when_sec ||
            (te->when_sec == nearest->when_sec &&
            te->when_ms < nearest->when_ms))
            nearest = te;
        te = te->next;
    }
    return nearest;
}

int NetManager::processTimeEvents()
{
    int processed = 0;
    aeTimeEvent *te, *prev;
    long long maxId;
    uint32_t now = get_stime();
    if (now < event_loop_p_->lastTime) {
        te = event_loop_p_->timeEventHead;
        while (te) {
            te->when_sec = 0;
            te = te->next;
        }
    }
    event_loop_p_->lastTime = now;

    prev = NULL;
    te = event_loop_p_->timeEventHead;
    maxId = event_loop_p_->timeEventNextId - 1;
    while (te) {
        uint32_t now_sec, now_ms;
        int64_t id;

        /* Remove events scheduled for deletion. */
        if (te->id == AE_DELETED_EVENT_ID) {
            aeTimeEvent *next = te->next;
            if (prev == NULL)
                event_loop_p_->timeEventHead = te->next;
            else
                prev->next = te->next;
            if (te->finalizerProc)
                te->finalizerProc(event_loop_p_, te->clientData);
            free(te);
            te = next;
            continue;
        }

        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        get_time_pair(now_sec, now_ms);
        if (now_sec > te->when_sec ||
            (now_sec == te->when_sec && now_ms >= te->when_ms))
        {
            int retval;

            id = te->id;
            retval = te->timeProc(event_loop_p_, id, te->clientData);
            processed++;
            if (retval != AE_NOMORE) {
                aeAddMillisecondsToNow(retval, &te->when_sec, &te->when_ms);
            }
            else {
                te->id = AE_DELETED_EVENT_ID;
            }
        }
        prev = te;
        te = te->next;
    }
    return processed;
}

int NetManager::aeProcessEvents(int flags)
{
    int processed = 0, numevents;
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) {
        return 0;
    }
    
    if (event_loop_p_->maxfd != -1 || ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
        int j;
        aeTimeEvent *shortest = NULL;

        timeval tv, *tvp;
        if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT)) {
            shortest = aeSearchNearestTimer();
        }

        if (shortest) {
            uint32_t now_sec, now_ms;

            get_time_pair(now_sec, now_ms);
            tvp = &tv;

            /* How many milliseconds we need to wait for the next
            * time event to fire? */
            long long ms =
                (shortest->when_sec - now_sec) * 1000 +
                shortest->when_ms - now_ms;

            if (ms > 0) {
                tvp->tv_sec = ms / 1000;
                tvp->tv_usec = (ms % 1000) * 1000;
            }
            else {
                tvp->tv_sec = 0;
                tvp->tv_usec = 0;
            }
        }
        else {
            /* If we have to check for events but need to return
            * ASAP because of AE_DONT_WAIT we need to set the timeout
            * to zero
            */
            if (flags & AE_DONT_WAIT) {
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            }
            else {
                /* Otherwise we can block */
                tvp = NULL; /* wait forever */
            }
        }

        numevents = net_event_p_->Poll(tvp);
        for (j = 0; j < numevents; j++) {
            aeFileEvent *fe = &event_loop_p_->events[event_loop_p_->fired[j].fd];
            int mask = event_loop_p_->fired[j].mask;
            int fd = event_loop_p_->fired[j].fd;
            int rfired = 0;

            if (fe->mask & mask & AE_READABLE) {
                rfired = 1;
                fe->rfileProc(event_loop_p_, fd, fe->clientData, mask);
            }
            if (fe->mask & mask & AE_WRITABLE) {
                if (!rfired || fe->wfileProc != fe->rfileProc)
                    fe->wfileProc(event_loop_p_, fd, fe->clientData, mask);
            }
            processed++;
        }
    }

    if (flags & AE_TIME_EVENTS) {
        processed += processTimeEvents();
    }

    return processed; 
}

int NetManager::aeWait(int fd, int mask, int64_t milliseconds) 
{
    pollfd pfd;
    int retmask = 0, retval;

    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = fd;
    if (mask & AE_READABLE) pfd.events |= POLLIN;
    if (mask & AE_WRITABLE) pfd.events |= POLLOUT;

    if ((retval = poll(&pfd, 1, milliseconds)) == 1) {
        if (pfd.revents & POLLIN) retmask |= AE_READABLE;
        if (pfd.revents & POLLOUT) retmask |= AE_WRITABLE;
        if (pfd.revents & POLLERR) retmask |= AE_WRITABLE;
        if (pfd.revents & POLLHUP) retmask |= AE_WRITABLE;
        return retmask;
    }
    else {
        return retval;
    }
}

void NetManager::StartMainLoop()
{
    event_loop_p_->stop = 0;
    while (!event_loop_p_->stop) {
        if (event_loop_p_->beforesleep != NULL)
            event_loop_p_->beforesleep(event_loop_p_);
        aeProcessEvents(AE_ALL_EVENTS);
    }
}

std::string NetManager::aeGetApiName(void)
{
    return net_event_p_->GetName();
}

void NetManager::aeSetBeforeSleepProc(aeBeforeSleepProc *beforesleep) 
{
    event_loop_p_->beforesleep = beforesleep;
}

