#ifndef __WOLFCC_PUSHEVENTHANDLER_H__
#define __WOLFCC_PUSHEVENTHANDLER_H__
#include "utils/queue.h"
#include "nets/socketpairhandler.h"

class HandlerRegistry;
class Package;

class PushEventHandler : public SocketPairHandler
{
public:
    void SetHandlerRegistry(HandlerRegistry* registry){
        this->registry_p_ = registry;
    }

    void SetQueue(Queue<Package*>* queue) {
        this->queue_p_ = queue;
    }

    virtual void HandleEvent();

private:
    HandlerRegistry* registry_p_;
    Queue<Package*>* queue_p_;
};

#endif
