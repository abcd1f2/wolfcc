#include "utils/handlerregistry.h"
#include "nets/eventhandler.h"

EventHandler* HandlerRegistry::GetHandler(size_t id) const
{
    std::map<size_t, EventHandler*>::const_iterator it;
    it = registry.find(id);
    if (it == registry.end()) {
        return NULL;
    }

    return it->second;
}

size_t HandlerRegistry::RegisterHandler(EventHandler* handler)
{
    while (true) {
        size_t id = ++idcounter_;
        if (registry.find(id) == registry.end()) {
            registry.insert(std::make_pair(id, handler));
            return id;
        }
    }
}

void HandlerRegistry::RemoveHandler(size_t id)
{
    registry.erase(id);
}

size_t HandlerRegistry::GetCount() const
{
    return registry.size();
}
