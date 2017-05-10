#include "nets/epollevent.h"
#include "nets/netbase.h"
#include <unistd.h>
#include "utils/logging.h"

EpollEvent::EpollEvent()
{

}

EpollEvent::~EpollEvent()
{

}

int EpollEvent::Create()
{
    aeApiState *state_p = new aeApiState();

    if (!state_p) return -1;
    state_p->events = (epoll_event *)malloc(sizeof(epoll_event) * event_loop_p_->setsize);
    if (!state_p->events) {
        delete state_p;
        return -1;
    }
    state_p->epfd = epoll_create(EPOLL_CREATE_COUNT); /* 1024 is just a hint for the kernel */
    if (state_p->epfd == -1) {
        free(state_p->events);
        delete state_p;
        return -1;
    }
    event_loop_p_->apidata = state_p;
    return 0;
}

int EpollEvent::Resize(int setsize) 
{
    aeApiState *state = reinterpret_cast<aeApiState*>(event_loop_p_->apidata);

    state->events = (epoll_event *)realloc(state->events, sizeof(struct epoll_event)*setsize);
    return 0;
}

void EpollEvent::Free()
{
    aeApiState *state = reinterpret_cast<aeApiState*>(event_loop_p_->apidata);

    close(state->epfd);
    free(state->events);
    free(state);
}

int EpollEvent::AddEvent(int fd, int mask)
{
    aeApiState *state = reinterpret_cast<aeApiState*>(event_loop_p_->apidata);
    struct epoll_event ee = { 0 };
    int op = event_loop_p_->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    ee.events = 0;
    mask |= event_loop_p_->events[fd].mask; /* Merge old events */
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.fd = fd;
    if (epoll_ctl(state->epfd, op, fd, &ee) == -1) {
        log(LOG_ERR, "epoll ctl error");
        return -1;
    }
    return 0;
}

void EpollEvent::DelEvent(int fd, int delmask) 
{
    aeApiState *state = reinterpret_cast<aeApiState*>(event_loop_p_->apidata);
    struct epoll_event ee = { 0 }; /* avoid valgrind warning */
    int mask = event_loop_p_->events[fd].mask & (~delmask);

    ee.events = 0;
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.fd = fd;
    if (mask != AE_NONE) {
        epoll_ctl(state->epfd, EPOLL_CTL_MOD, fd, &ee);
    }
    else {
        /* Note, Kernel < 2.6.9 requires a non null event pointer even for
        * EPOLL_CTL_DEL. */
        epoll_ctl(state->epfd, EPOLL_CTL_DEL, fd, &ee);
    }
}

int EpollEvent::Poll(struct timeval *tvp) 
{
    aeApiState *state = reinterpret_cast<aeApiState*>(event_loop_p_->apidata);
    int retval, numevents = 0;

    retval = epoll_wait(state->epfd, state->events, event_loop_p_->setsize, tvp ? (tvp->tv_sec * 1000 + tvp->tv_usec / 1000) : -1);
    if (retval > 0) {
        int j;

        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = state->events + j;

            if (e->events & EPOLLIN) mask |= AE_READABLE;
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
            if (e->events & EPOLLERR) mask |= AE_WRITABLE;
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE;
            event_loop_p_->fired[j].fd = e->data.fd;
            event_loop_p_->fired[j].mask = mask;
        }
    }
    return numevents;
}

std::string EpollEvent::GetName(void) 
{
    return "epoll";
}

