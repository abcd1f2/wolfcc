#ifndef __WOLFCC_OBJECTALLOCATOR_H__
#define __WOLFCC_OBJECTALLOCATOR_H__
#include "utils/allocator.h"

template <class T>
class ObjectAllocator
{
public:
    virtual ~ObjectAllocator() {}
    virtual T * Create() = 0;
    virtual void Destroy(T * pObject) = 0;
};

template <class T>
class ObjectDefaultAllocator : public ObjectAllocator < T >
{
public:
    ObjectDefaultAllocator(Allocator * pAllocator) : pAllocator_(pAllocator) {}

    virtual T * Create()
    {
        void * p = pAllocator_->Allocate(sizeof(T));
        if (p == NULL) {
            return NULL;
        }
        return new (p)T;
    }

    virtual void Destroy(T * p)
    {
        p->~T();
        pAllocator_->Deallocate(p);
    }

private:
    Allocator * pAllocator_;
};

#endif
