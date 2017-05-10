#ifndef __WOLFCC_NETBASE_H__
#define __WOLFCC_NETBASE_H__
#include <stdint.h>

enum
{
    AE_NONE = 0,
    AE_READABLE = 1,
    AE_WRITABLE = 2
};

enum 
{
    ANET_ERR = -1,
    ANET_OK = 0,
    ANET_ERR_LEN = 256
};

enum 
{
    AE_ERR = -1,
    AE_OK = 0
};

enum 
{
    ANET_NONE = 0,
    ANET_IP_ONLY = 1,
};

enum 
{
    ANET_CONNECT_NONE = 0,
    ANET_CONNECT_NONBLOCK = 1,
    ANET_CONNECT_BE_BINDING = 2
};

enum 
{
    AE_NOMORE = -1,
    AE_DELETED_EVENT_ID = -1
};

enum 
{
    EPOLL_EVENTS_NUM = 16 * 1024
};

#define AE_FILE_EVENTS 1
#define AE_TIME_EVENTS 2
#define AE_ALL_EVENTS (AE_FILE_EVENTS|AE_TIME_EVENTS)
#define AE_DONT_WAIT 4

typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
typedef int aeTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData);
typedef void aeEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData);
typedef void aeBeforeSleepProc(struct aeEventLoop *eventLoop);

struct aeFileEvent
{
    int mask; /* one of AE_(READABLE|WRITABLE) */
    aeFileProc *rfileProc;
    aeFileProc *wfileProc;
    void *clientData;
};

struct aeTimeEvent
{
    int64_t id; /* time event identifier. */
    uint32_t when_sec; /* seconds */
    uint32_t when_ms; /* milliseconds */
    aeTimeProc *timeProc;
    aeEventFinalizerProc *finalizerProc;
    void *clientData;
    struct aeTimeEvent *next;
};

struct aeFiredEvent
{
    int fd;
    int mask;
};

struct aeEventLoop
{
    int maxfd;   /* highest file descriptor currently registered */
    int setsize; /* max number of file descriptors tracked */
    int64_t timeEventNextId;
    uint32_t lastTime;     /* Used to detect system clock skew */
    aeFileEvent *events; /* Registered events */
    aeFiredEvent *fired; /* Fired events */
    aeTimeEvent *timeEventHead;
    int stop;
    void *apidata; /* This is used for polling API specific data */
    aeBeforeSleepProc *beforesleep;
};

#endif
