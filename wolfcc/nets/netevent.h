#ifndef __WOLFCC_NETEVENT_H__
#define __WOLFCC_NETEVENT_H__
#include <string>

struct aeEventLoop;

class NetEvent
{
public:
    NetEvent(){}
    virtual ~NetEvent(){}

public:
    virtual void SetEventLoop(const aeEventLoop *event_loop_p){ event_loop_p_ = event_loop_p; }

    virtual int Create() = 0;
    virtual int Resize(int setsize) = 0;
    virtual void Free(void) = 0;
    virtual int AddEvent(int fd, int mask) = 0;
    virtual void DelEvent(int fd, int delmask) = 0;
    virtual int Poll(struct timeval *tvp) = 0;
    virtual std::string GetName(void) = 0;

    aeEventLoop *GetEventLoop() const { return event_loop_p_; }

protected:
    aeEventLoop *event_loop_p_;
};

#endif
