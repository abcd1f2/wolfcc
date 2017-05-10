#ifndef __WOLFCC_NETMANAGER_H__
#define __WOLFCC_NETMANAGER_H__
#include <stdint.h>
#include <string>
#include "nets/netbase.h"
#include "nets/networkbase.h"

class NetEvent;

class NetManager
{
public:
    NetManager(NetEvent *net_event);
    ~NetManager();

public:
    bool StartListen();

    void StartMainLoop();

    bool aeCreateEventLoop(int setsize);
    void aeDeleteEventLoop();
    void aeStop();
    int aeCreateFileEvent(int fd, int mask, aeFileProc *proc, void *clientData);
    void aeDeleteFileEvent(int fd, int mask);
    int aeGetFileEvents(int fd);
    int64_t aeCreateTimeEvent(int64_t milliseconds, aeTimeProc *proc, void *clientData, aeEventFinalizerProc *finalizerProc);
    int aeDeleteTimeEvent(int64_t id);
    int aeProcessEvents(int flags);
    int aeWait(int fd, int mask, int64_t milliseconds);
    std::string aeGetApiName(void);
    void aeSetBeforeSleepProc(aeBeforeSleepProc *beforesleep);
    int aeGetSetSize();
    int aeResizeSetSize(int setsize);

private:
    void aeAddMillisecondsToNow(int64_t milliseconds, uint32_t &sec, uint32_t &ms);
    aeTimeEvent *aeSearchNearestTimer();
    int processTimeEvents();

private:
    aeEventLoop *event_loop_p_;
    NetEvent *net_event_p_;
    NetWork *net_work_p_;
};

#endif