#ifndef __WOLFCC_HANDLERREGISTRY_H__
#define __WOLFCC_HANDLERREGISTRY_H__
#include <ctype.h>
#include <map>
#include "nets/eventhandler.h"

class HandlerRegistry
{
public:
    HandlerRegistry():idcounter_(0)
    {}

public:
    EventHandler* GetHandler(size_t id) const;
    size_t RegisterHandler(EventHandler* handler);
    void RemoveHandler(size_t id);
    size_t GetCount() const;

private:
    std::map<size_t, EventHandler*> registry;
    size_t idcounter_;
};

#endif
