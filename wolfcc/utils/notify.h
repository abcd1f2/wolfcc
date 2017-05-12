#ifndef __WOLFCC_NOTIFY_H__
#define __WOLFCC_NOTIFY_H__
#include "nets/handle.h"

class Notification
{
public:
    virtual ~Notification() {}

public:
    virtual void notify(void* sender) = 0;
};

class SocketNotification : public Notification
{
public:
    void SetHandle(Handle handle)
    {
        this->handle = handle;
    }

    Handle GetHandle() const
    {
        return this->handle;
    }

    virtual void notify(void* sender);

private:
    Handle handle;
};

#endif
