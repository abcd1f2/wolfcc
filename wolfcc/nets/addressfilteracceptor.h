#ifndef ADDRESS_FILTER_ACCEPTOR_H_
#define ADDRESS_FILTER_ACCEPTOR_H_
#include "nets/connecteventhandler.h"
#include "pros/procpack.h"
#include "nets/acceptor.h"
#include "utils/allocator.h"
#include "utils/objectpool.h"
#include <list>
#include <string>

class HandlerRegistry;

class AddressFilteredAcceptor : public Acceptor, public ConnectEventHandlerManager
{
public:
    AddressFilteredAcceptor(const SockAddr& bindaddr, 
        size_t processorid,
        Procpack* extractor, 
        HandlerRegistry* registry);

    void SetLimitAddresses(const std::list<std::string>* addresses)
    {
        limitaddresses = addresses;
    }

    virtual void call(Destroyable* handler);

    void SetTimeout(size_t timeout)
    {
        this->timeout = timeout;
    }

protected:

    virtual EventHandler* CreateHandler(const SockAddr& addr);

    virtual void DestroyHandler(EventHandler* handler);

    virtual int OnAccept(Handle handle, const SockAddr& addr);

    bool ValidAddress(const char* clientip);

private:
    ObjectPoolAllocator<ConnectEventHandler> ConnectEventHandlerAllocator;
    size_t            processorid;
    Procpack*         extractor;
    HandlerRegistry*  registry;
    size_t            timeout;

    const std::list<std::string>* limitaddresses;
};

#endif
