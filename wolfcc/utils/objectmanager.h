#ifndef __WOLFCC_OGJECTMANAGER_H__
#define __WOLFCC_OGJECTMANAGER_H__
#include <assert.h>
#include "utils/objectallocator.h"
#include "utils/destroyable.h"

template <class T>
class ObjectManager : public DestroyCallback
{
public:
    ObjectManager(ObjectAllocator<T> * pObjectAllocator)
        : pObjectAllocator_(pObjectAllocator)
    {
        assert(pObjectAllocator_);
    }

    T * Create() {
        T * p = pObjectAllocator_->Create();
        if (p != NULL) {
            p->SetDestroyCallback(this);
        }
        return p;
    }

    virtual void call(Destroyable* pObject) {
        return pObjectAllocator_->Destroy(static_cast<T*>(pObject));
    }

private:
    ObjectAllocator<T> * pObjectAllocator_;
};

#endif
