#ifndef __WOLFCC_CONNECTEVENTHANDLER_H__
#define __WOLFCC_CONNECTEVENTHANDLER_H__
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include "nets/eventhandler.h"
#include "utils/objectmanager.h"
#include "utils/queue.h"
#include "utils/datablock.h"
#include "pros/procpack.h"

class ConnectEventHandler : public EventHandler
{
    SockAddr    peerAddr;
    DataBlock   recvbuffer;
    DataBlock   sendbuffer;
    size_t      globalid;
    size_t      processorid;
    int         timeout;
    Procpack*   extractor;

    bool        closeaftersent;

    bool        received;
    timeval     opentime;
    timeval     inputtime;
    timeval     recvtime;

public:
    ConnectEventHandler();

    virtual ~ConnectEventHandler();

    void SetGlobalId(size_t globalid)
    {
        this->globalid = globalid;
    }

    size_t GetGlobalId() const
    {
        return this->globalid;
    }

    void SetProcessorId(size_t processorid)
    {
        this->processorid = processorid;
    }

    size_t GetProcessorId() const
    {
        return this->processorid;
    }

    void SetTimeout(int timeout)
    {
        this->timeout = timeout;
    }

    void SetExtractor(Procpack* extractor)
    {
        this->extractor = extractor;
    }

    void SetPeerAddr(const SockAddr& addr)
    {
        this->peerAddr = addr;
    }

    const SockAddr& GetPeerAddr() const
    {
        return this->peerAddr;
    }

    virtual int Open();

    virtual int Close();

    virtual void HandleClose();

    virtual void HandleInput();

    virtual void HandleOutput();

    virtual void HandleTimeout(void *);

    void Send(const char* data, size_t len);

    void CloseAfterSent()
    {
        closeaftersent = true;
    }
};

typedef ObjectManager<ConnectEventHandler> ConnectEventHandlerManager;

#endif
