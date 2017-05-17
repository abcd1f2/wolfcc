#include <errno.h>
#include <arpa/inet.h>
#include "nets/sockaddr.h"
#include "nets/addressfilteracceptor.h"
#include "connecteventhandler.h"
#include "utils/handlerregistry.h"
#include "utils/logging.h"
#include "utils/destroyable.h"

#define ACF_MAX_CONNECTIONS 10240

AddressFilteredAcceptor::AddressFilteredAcceptor(
    const SockAddr& bindaddr, size_t processorid_,
    Procpack* extractor_, HandlerRegistry* registry_)
    : Acceptor(bindaddr),
    ConnectEventHandlerManager(&ConnectEventHandlerAllocator),
    ConnectEventHandlerAllocator(128, ACF_MAX_CONNECTIONS, 128),
    processorid(processorid_),
    extractor(extractor_),
    registry(registry_),
    timeout(3),
    limitaddresses_(NULL)
{

}

EventHandler* AddressFilteredAcceptor::CreateHandler(const SockAddr& addr)
{
    ConnectEventHandler* handler = Create();
    if (handler != NULL) {
        // set read and write event timeout
        handler->SetTimeout(timeout);
        handler->SetExtractor(extractor);
        handler->SetGlobalId(registry->RegisterHandler(handler));
        handler->SetProcessorId(processorid);
        handler->SetPeerAddr(addr);
    }

    return handler;
}

void AddressFilteredAcceptor::DestroyHandler(EventHandler* handler)
{
    call(handler);
}

void AddressFilteredAcceptor::call(Destroyable* handler)
{
    assert(handler);
    ConnectEventHandler* myhandler = static_cast<ConnectEventHandler*>(handler);

    registry->RemoveHandler(myhandler->GetGlobalId());
    ConnectEventHandlerManager::call(handler);
}


int AddressFilteredAcceptor::OnAccept(Handle handle, const SockAddr& addr)
{
    if (!ValidAddress(addr.ToString().c_str())) {
        log(LOG_WARN, "unallowed addres: %s", addr.ToString().c_str());
        return -1;
    }

    return Acceptor::OnAccept(handle, addr);
}

bool AddressFilteredAcceptor::ValidAddress(const char* clientip)
{
	if (!limitaddresses_) {
		return true;
	}

    std::list<std::string>::const_iterator iter;
    for (iter = limitaddresses_->begin(); iter != limitaddresses_->end(); ++iter) {
        if (strncmp(clientip, iter->c_str(), iter->length()) == 0)
            return true;
    }

    return false;
}
