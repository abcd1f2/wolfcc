#ifndef __WOLFCC_OBJECTPOOL_H__
#define __WOLFCC_OBJECTPOOL_H__
#include <list>
#include <vector>
#include <sys/types.h>
#include "utils/allocator.h"
#include "utils/objectallocator.h"
#include "utils/mutex.h"

template <class T>
class ObjectPoolAllocator : public ObjectAllocator < T >
{
public:

    ObjectPoolAllocator(size_t iInitNum, 
        size_t iMaxNum = 0, 
        size_t iIncNum = 0, 
        Mutex * pMutex = NullMutex::Instance(), 
        Allocator *pAllocator = Allocator::Instance()):
        allocator_p_(pAllocator), 
        mutex_p_(pMutex), 
        iCurrNum_(0), 
        iMaxNum_(iMaxNum), 
        iIncNum_(iIncNum)
    {
        if (iMaxNum_ < iInitNum) {
            iMaxNum_ = iInitNum;
            iIncNum_ = 0;
        }

        IncreaseObjects(iInitNum);
    }

    virtual ~ObjectPoolAllocator()
    {
        for (typename::std::list<T *>::iterator pos = Objects_.begin(); pos != Objects_.end(); ++pos) {
            (*pos)->~T();
        }

        for (typename::std::vector<T *>::iterator pos = Chunks_.begin(); pos != Chunks_.end(); ++pos) {
			allocator_p_->Deallocate(*pos);
        }
    }

    inline void SetMaxNum(size_t iMaxNum)
    {
        if (iCurrNum_ >= iMaxNum) {
            iMaxNum_ = iCurrNum_;
        }
        else {
            iMaxNum_ = iMaxNum;
        }
    }

    inline void SetIncNum(size_t iIncNum)
    {
        iIncNum_ = iIncNum;
    }

    inline bool IsInit() const { return iCurrNum_ > 0; }

    virtual T * Create()
    {
        LockGuard lock(mutex_p_);
        if (lock.Lock(true) != 0) {
            return NULL;
        }

        if (Objects_.empty()) {
            if (IncreaseObjects(iIncNum_) != 0) {
                return NULL;
            }
        }

        T * pObject = *(Objects_.begin());
        Objects_.pop_front();

        return pObject;
    }

    virtual void Destroy(T * pObject)
    {
        LockGuard lock(mutex_p_);
        if (lock.Lock(true) != 0) {
            return;
        }

        if (pObject != NULL) {
            Objects_.push_front(pObject);
        }
    }

private:

    int IncreaseObjects(size_t iIncNum)
    {
        if (iCurrNum_ >= iMaxNum_) {
            return -1;
        }

        if (iCurrNum_ + iIncNum > iMaxNum_) {
            iIncNum = iMaxNum_ - iCurrNum_;
        }

        T * pChunk = (T *)allocator_p_->Allocate(sizeof(T)*iIncNum);
        if (pChunk == NULL) {
            return -1;
        }

        for (size_t i = 0; i < iIncNum; ++i) {
            T* p = new((void *)(pChunk + i)) T;
            Objects_.push_back(p);
        }

        Chunks_.push_back(pChunk);
        iCurrNum_ += iIncNum;
        return 0;
    }

private:
    Allocator *	allocator_p_;
    Mutex *		mutex_p_;

    size_t		iCurrNum_;
    size_t		iMaxNum_;
    size_t		iIncNum_;

    std::list<T *>	Objects_;
    std::vector<T *>	Chunks_;
};

#endif
