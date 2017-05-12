#include <pthread.h>
#include "utils/mutex.h"
#include "utils/mutexcond.h"

NullMutexCond::NullMutexCond(const NullMutex &)
{
}

NullMutexCond * NullMutexCond::Instance()
{
    static NullMutexCond oNullMutexCond;
    return &oNullMutexCond;
}

ThreadMutexCond::ThreadMutexCond(const ThreadMutex & cThreadMutex)
    : cThreadMutex_(cThreadMutex)
{
    pthread_cond_init(&cond_, NULL);
}

ThreadMutexCond::~ThreadMutexCond()
{
    pthread_cond_destroy(&cond_);
}

int ThreadMutexCond::Wait(const struct timespec * pTimeout) const
{
    if (pTimeout == NULL) {
        if (pthread_cond_wait(&cond_, &cThreadMutex_.mutex_) != 0) {
            return -1;
        }
    }
    else {
        if (pthread_cond_timedwait(&cond_, &cThreadMutex_.mutex_, pTimeout) != 0) {
            return -1;
        }
    }
    return 0;
}

int ThreadMutexCond::Signal() const
{
    return pthread_cond_signal(&cond_);
}

int ThreadMutexCond::Broadcast() const
{
    return pthread_cond_broadcast(&cond_);
}

