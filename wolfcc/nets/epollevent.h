#ifndef __WOLFCC_EPOLLEVENT_H__
#define __WOLFCC_EPOLLEVENT_H__
#include "nets/netevent.h"
#include <sys/epoll.h>

struct aeApiState 
{
    int epfd;
    epoll_event *events;
};

enum 
{
    EPOLL_CREATE_COUNT = 1024,
};

class EpollEvent : public NetEvent
{
public:
    EpollEvent();
    virtual ~EpollEvent();

public:
    virtual int Create();
    virtual int Resize(int setsize);
    virtual void Free(void);
    virtual int AddEvent(int fd, int mask);
    virtual void DelEvent(int fd, int delmask);
    virtual int Poll(struct timeval *tvp);
    virtual std::string GetName(void);
};

#endif
